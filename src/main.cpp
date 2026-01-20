#include <SFML/Graphics.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include "Level.h"
#include "Game.h"
#include "Ball.h"
#include <cmath>


sf::Vector2f getDirectionPlayer1() {
    sf::Vector2f direction(0.f, 0.f);
    
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) direction.x -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) direction.x += 1.f;
    
    return direction;
}

sf::Vector2f getDirectionPlayer2() {
    sf::Vector2f direction(0.f, 0.f);
    
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) direction.x -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) direction.x += 1.f;
    
    return direction;
}

static sf::Vector2f lerpVector(sf::Vector2f start, sf::Vector2f end, float delta) {
    return start + (end - start) * delta;
}

bool checkCircleRectCollision(const sf::CircleShape& circle, const sf::RectangleShape& wall) {
    sf::Vector2f circleCenter = circle.getPosition();
    float radius = circle.getRadius();

    sf::FloatRect wallBounds = wall.getGlobalBounds();

    float closestX = std::clamp(circleCenter.x, wallBounds.position.x, wallBounds.position.x + wallBounds.size.x);
    float closestY = std::clamp(circleCenter.y, wallBounds.position.y, wallBounds.position.y + wallBounds.size.y);

    float distanceX = circleCenter.x - closestX;
    float distanceY = circleCenter.y - closestY;

    return (distanceX * distanceX + distanceY * distanceY) < (radius * radius);
}

void calculateJumpParameters(float maxHeight, float horizontalDistance, float playerSpeed, float& outGravity, float& outJumpVelocity) {
    float timeToApex = horizontalDistance / playerSpeed;
    
    outGravity = (2.0f * maxHeight) / (timeToApex * timeToApex);
    
    outJumpVelocity = -outGravity * timeToApex; 
}


void updatePlayerPhysics(Level& level, sf::RectangleShape& player, sf::Vector2f& velocity, sf::Vector2f& acceleration, float delta, bool& isGrounded, float gravity, bool fastFall) {
    
    sf::Vector2f oldAcceleration = acceleration;
    player.move({ velocity.x * delta, 0.f });

    for (auto& wall : level.getWalls()) {
        if (player.getGlobalBounds().findIntersection(wall.getGlobalBounds())) {
            if (velocity.x > 0) {
                player.setPosition({ wall.getPosition().x - player.getSize().x / 2.f, player.getPosition().y });
            }
            else if (velocity.x < 0) {
                player.setPosition({ wall.getPosition().x + wall.getSize().x + player.getSize().x / 2.f, player.getPosition().y });
            }
            velocity.x = 0;
        }
    }

    player.move({ 0.f, velocity.y * delta + 0.5f * oldAcceleration.y * delta * delta });
    
    float currentGravity = gravity;
    if (fastFall && velocity.y > 0) {
        currentGravity *= 2.5f;
    }
    sf::Vector2f newAcceleration(0.f, currentGravity);
    
    isGrounded = false;

    for (auto& wall : level.getWalls()) {
        if (player.getGlobalBounds().findIntersection(wall.getGlobalBounds())) {
            if (velocity.y > 0) {
                player.setPosition({ player.getPosition().x, wall.getPosition().y - player.getSize().y / 2.f });
                velocity.y = 0;
                newAcceleration.y = 0;
                isGrounded = true;
            }
            else if (velocity.y < 0) {
                player.setPosition({ player.getPosition().x, wall.getPosition().y + wall.getSize().y + player.getSize().y / 2.f });
                velocity.y = 0;
                newAcceleration.y = 0;
            }
        }
	}

    for (auto& floor : level.getFloors()) {
        if (player.getGlobalBounds().findIntersection(floor.getGlobalBounds())) {
            float playerBottom = player.getPosition().y + player.getSize().y / 2.f;
            float floorTop = floor.getPosition().y;

            if (playerBottom <= floorTop + 5.f && velocity.y > 0) {
                player.setPosition({ player.getPosition().x, floor.getPosition().y - player.getSize().y / 2.f });
                velocity.y = 0;
                newAcceleration.y = 0;
                isGrounded = true;
            }
        }
    }
    
    velocity.y += 0.5f * (oldAcceleration.y + newAcceleration.y) * delta;
    
    acceleration = newAcceleration;
}

void updatePlayerPhysics(Level& level, sf::CircleShape& player, sf::Vector2f& velocity, sf::Vector2f& acceleration, float delta, bool& isGrounded, float gravity, bool fastFall) {
    
    sf::Vector2f oldAcceleration = acceleration;
    player.move(velocity * delta + 0.5f * oldAcceleration * delta * delta);

    float currentGravity = gravity;
    if (fastFall && velocity.y > 0) {
        currentGravity *= 2.5f;
    }
    sf::Vector2f newAcceleration(0.f, currentGravity);
    
    float radius = player.getRadius();
    sf::Vector2f playerCenter = player.getPosition();

    isGrounded = false;

    for (const auto& wall : level.getWalls()) {
        sf::FloatRect wallBounds = wall.getGlobalBounds();

        float closestX = std::clamp(playerCenter.x, wallBounds.position.x, wallBounds.position.x + wallBounds.size.x);
        float closestY = std::clamp(playerCenter.y, wallBounds.position.y, wallBounds.position.y + wallBounds.size.y);

        float dx = playerCenter.x - closestX;
        float dy = playerCenter.y - closestY;

        float distanceSquared = dx * dx + dy * dy;

        if (distanceSquared < radius * radius) {

            float distance = std::sqrt(distanceSquared);

            if (distance == 0.0f) {
                distance = 0.01f;
                dx = 0.01f;
            }

            float overlap = radius - distance;

            float normX = dx / distance;
            float normY = dy / distance;

            player.move({ normX * overlap, normY * overlap });

            if (normY < -0.5f) {
                velocity.y = 0;
                newAcceleration.y = 0;
                isGrounded = true;
            } else {
                sf::Vector2f normal(normX, normY);
                float dotProduct = velocity.x * normX + velocity.y * normY;
                velocity.x -= dotProduct * normX;
                velocity.y -= dotProduct * normY;
            }

            playerCenter = player.getPosition();
        }
    }

    for (const auto& floor : level.getFloors()) {
        sf::FloatRect floorBounds = floor.getGlobalBounds();

        float playerBottom = playerCenter.y + radius;
        float floorTop = floorBounds.position.y;

        if (playerBottom <= floorTop + 5.f && velocity.y > 0) {
            float closestX = std::clamp(playerCenter.x, floorBounds.position.x, floorBounds.position.x + floorBounds.size.x);
            float closestY = std::clamp(playerCenter.y, floorBounds.position.y, floorBounds.position.y + floorBounds.size.y);

            float dx = playerCenter.x - closestX;
            float dy = playerCenter.y - closestY;

            float distanceSquared = dx * dx + dy * dy;

            if (distanceSquared < radius * radius) {
                float distance = std::sqrt(distanceSquared);

                if (distance == 0.0f) {
                    distance = 0.01f;
                    dx = 0.01f;
                }

                float overlap = radius - distance;

                float normX = dx / distance;
                float normY = dy / distance;

                player.move({ normX * overlap, normY * overlap });
                
                if (normY < -0.5f) {
                    velocity.y = 0;
                    newAcceleration.y = 0;
                    isGrounded = true;
                }

                playerCenter = player.getPosition();
            }
        }
    }
    
    velocity += 0.5f * (oldAcceleration + newAcceleration) * delta;
    
    acceleration = newAcceleration;
}

void updatePointsVisuals(Level& level, sf::Vector2f activePoint) {
    for (auto& p : level.points) {
        if (p.getPosition() == activePoint) {
            p.setFillColor(sf::Color(255, 255, 0, 255)); // Widoczny
        }
        else {
            p.setFillColor(sf::Color(255, 255, 0, 0));   // Niewidoczny
        }
    }
}

void changePointLocation(sf::Vector2f& point, sf::Vector2f location1, sf::Vector2f location2, Level& level) {
    std::vector<sf::Vector2f>& locations = level.getPointLocations();

    for (int i = 0; i < locations.size(); ++i) {
        if (location1 != locations[i] && location2 != locations[i]) {
            point = locations[i];

			updatePointsVisuals(level, point);
            return;
        }
    }
}

void updateArrowDirection(sf::CircleShape& arrow, sf::Vector2f playerPos, sf::Vector2f targetPos) {
    float dx = targetPos.x - playerPos.x;
    float dy = targetPos.y - playerPos.y;

    float angleInRadians = std::atan2(dy, dx);

    float radius = 60.f;

    float arrowX = playerPos.x + radius * std::cos(angleInRadians);
    float arrowY = playerPos.y + radius * std::sin(angleInRadians);

    arrow.setPosition({ arrowX, arrowY });

    sf::Angle angle = sf::radians(angleInRadians);
    arrow.setRotation(angle + sf::degrees(90.f));
}

void handlePointLocations(sf::RectangleShape& player1, sf::CircleShape& player2, Level& level, sf::Vector2f& point) {
	std::vector<sf::Vector2f> locations = level.getPointLocations();

    if (locations.size() < 2) return;

    for (int i = 0; i < locations.size(); ++i) {
        if (locations[i] == point) {
            continue;
        }

        int targetIndex1 = 0;
		int targetIndex2 = 0;

        if (player1.getGlobalBounds().contains(point) || player2.getGlobalBounds().contains(point)) {
            do {
                targetIndex1 = std::rand() % locations.size();
            } while (targetIndex1 == i);

            do {
                targetIndex2 = std::rand() % locations.size();
			} while (targetIndex2 == i);

			sf::Vector2f player1Spawn = locations[targetIndex1];
			sf::Vector2f player2Spawn = locations[targetIndex2];

            player1.setPosition(player1Spawn);
            player2.setPosition(player2Spawn);

            changePointLocation(point, player1Spawn, player2Spawn, level);
        }
    }
}

void runGame() {
    std::string title = "2DGK";
    int windowWidth = 1280;
    int windowHeight = 720;
    sf::Vector2f cameraPos = sf::Vector2f(windowWidth / 2.f, windowHeight / 2.f);
    sf::RenderWindow window(sf::VideoMode({ 1280, 720 }), title);

    sf::RectangleShape player1(sf::Vector2f(50, 50));
    sf::CircleShape player2(25.f);

    player1.setOrigin({ 25.f, 25.f });
    player2.setOrigin({ 25.f, 25.f });

    const sf::Texture rectTexture("../../../../textures/texture1.png");

    player1.setPosition(sf::Vector2f(770, 360));
    player2.setPosition(sf::Vector2f(560, 360));

    player1.setTexture(&rectTexture);
    player2.setTexture(&rectTexture);

    player1.setFillColor(sf::Color(255, 0, 0, 255));
    player2.setFillColor(sf::Color(0, 0, 255, 255));

    sf::Clock clock;
    float playerSpeed = 200.f;
    
    float maxJumpHeight = 150.f;
    float horizontalDistanceToApex = 100.f;
    float gravity = 800.f;
    float jumpForce = -450.f;
    
    calculateJumpParameters(maxJumpHeight, horizontalDistanceToApex, playerSpeed, gravity, jumpForce);
    
    sf::Vector2f player1Velocity(0.f, 0.f);
    sf::Vector2f player2Velocity(0.f, 0.f);
    sf::Vector2f player1Acceleration(0.f, gravity);
    sf::Vector2f player2Acceleration(0.f, gravity);
    bool player1Grounded = false;
    bool player2Grounded = false;
    
    float player1CoyoteTime = 0.f;
    float player2CoyoteTime = 0.f;
    const float coyoteTimeDuration = 0.1f;
    
    bool player1DoubleJumpAvailable = false;
    bool player2DoubleJumpAvailable = false;
    
    bool player1JumpPressed = false;
    bool player2JumpPressed = false;

    sf::Font font;
    if (!font.openFromFile("C:/Windows/Fonts/arial.ttf")) {
        std::cout << "Failed to load font" << std::endl;
    }
    
    sf::Text paramText(font);
    paramText.setCharacterSize(20);
    paramText.setFillColor(sf::Color::White);
    paramText.setOutlineColor(sf::Color::Black);
    paramText.setOutlineThickness(2);

    std::cout << "=== Parametry skoku ===" << std::endl;
    std::cout << "Max wysokosc: " << maxJumpHeight << " px" << std::endl;
    std::cout << "Dystans poziomy do szczytu: " << horizontalDistanceToApex << " px" << std::endl;
    std::cout << "v_0 (początkowa predkosc skoku): " << jumpForce << " px/s" << std::endl;
    std::cout << "g (grawitacja): " << gravity << " px/s^2" << std::endl;
    std::cout << "\nSterowanie:" << std::endl;
    std::cout << "[1/2] - Zwieksz/Zmniejsz max wysokosc" << std::endl;
    std::cout << "[3/4] - Zwieksz/Zmniejsz dystans poziomy" << std::endl;
    std::cout << "[Up/W] - Skok (trzymaj dluzej = wyzszy skok, podwojny skok)" << std::endl;
    std::cout << "[Down/S] - Szybkie spadanie" << std::endl;
    std::cout << "=======================" << std::endl;

    // test
    Level level(window.getSize().x, window.getSize().y);
    level.loadFromFile("../../../../src/level1.txt");

	sf::Vector2f point;
    std::vector<sf::Vector2f>& locations = level.getPointLocations();
    if (!locations.empty()) {
        point = locations[0];
    }

    //view
    sf::Vector2f averagePos = (player1.getPosition() + player2.getPosition()) / 2.f;
    sf::View view({ averagePos.x, averagePos.y }, { (float)windowWidth, (float)windowHeight });
    float distance;
    window.setView(view);

    //arrows
    sf::CircleShape arrow1(15.f, 3);
	sf::CircleShape arrow2(15.f, 3);

    arrow1.setFillColor(sf::Color::Yellow);
	arrow2.setFillColor(sf::Color::Yellow);

    arrow1.setOrigin({ 15.f, 15.f });
	arrow2.setOrigin({ 15.f, 15.f });

    arrow1.setScale({ 0.7f, 1.2f });
    arrow2.setScale({ 0.7f, 1.2f });

	updatePointsVisuals(level, point);

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent()) // obsluga zdarzen
        {
            if (event->is<sf::Event::Closed>())
                window.close();
            
            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                bool paramsChanged = false;
                
                if (keyPressed->scancode == sf::Keyboard::Scancode::Num1) {
                    maxJumpHeight += 10.f;
                    paramsChanged = true;
                }
                else if (keyPressed->scancode == sf::Keyboard::Scancode::Num2) {
                    maxJumpHeight = std::max(10.f, maxJumpHeight - 10.f);
                    paramsChanged = true;
                }
                
                else if (keyPressed->scancode == sf::Keyboard::Scancode::Num3) {
                    horizontalDistanceToApex += 10.f;
                    paramsChanged = true;
                }
                else if (keyPressed->scancode == sf::Keyboard::Scancode::Num4) {
                    horizontalDistanceToApex = std::max(10.f, horizontalDistanceToApex - 10.f);
                    paramsChanged = true;
                }
                
                if (paramsChanged) {

                    calculateJumpParameters(maxJumpHeight, horizontalDistanceToApex, playerSpeed, gravity, jumpForce);
                    player1Acceleration.y = gravity;
                    player2Acceleration.y = gravity;
                    
                    std::cout << "\n=== Parametry skoku ===" << std::endl;
                    std::cout << "Max wysokosc: " << maxJumpHeight << " px" << std::endl;
                    std::cout << "Dystans poziomy do szczytu: " << horizontalDistanceToApex << " px" << std::endl;
                    std::cout << "v_0 (poczatkowa predkosc skoku): " << jumpForce << " px/s" << std::endl;
                    std::cout << "g (grawitacja): " << gravity << " px/s^2" << std::endl;
                    std::cout << "=======================" << std::endl;
                }
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
            window.close();

        sf::Vector2f player1Direction = getDirectionPlayer1();
        sf::Vector2f player2Direction = getDirectionPlayer2();

        float delta = clock.restart().asSeconds();
        
        if (player1Grounded) {
            player1CoyoteTime = coyoteTimeDuration;
        } else {
            player1CoyoteTime -= delta;
        }
        
        if (player2Grounded) {
            player2CoyoteTime = coyoteTimeDuration;
        } else {
            player2CoyoteTime -= delta;
        }

        bool player1JumpPressedNow = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up);
        if (player1JumpPressedNow && !player1JumpPressed) {

            if (player1Grounded || player1CoyoteTime > 0.f) {
                player1Velocity.y = jumpForce;
                player1DoubleJumpAvailable = true;
                player1CoyoteTime = 0.f;
            }
            else if (player1DoubleJumpAvailable) {
                player1Velocity.y = jumpForce;
                player1DoubleJumpAvailable = false;
            }
        }
        player1JumpPressed = player1JumpPressedNow;
        
        bool player2JumpPressedNow = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W);
        if (player2JumpPressedNow && !player2JumpPressed) {

            if (player2Grounded || player2CoyoteTime > 0.f) {
                player2Velocity.y = jumpForce;
                player2DoubleJumpAvailable = true;
                player2CoyoteTime = 0.f;
            }
            else if (player2DoubleJumpAvailable) {
                player2Velocity.y = jumpForce;
                player2DoubleJumpAvailable = false;
            }
        }
        player2JumpPressed = player2JumpPressedNow;

        player1Velocity.x = player1Direction.x * playerSpeed;
        player2Velocity.x = player2Direction.x * playerSpeed;
        
        bool player1FastFall = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down);
        bool player2FastFall = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S);

        updatePlayerPhysics(level, player1, player1Velocity, player1Acceleration, delta, player1Grounded, gravity, player1FastFall);
		updatePlayerPhysics(level, player2, player2Velocity, player2Acceleration, delta, player2Grounded, gravity, player2FastFall);

        if (!player1JumpPressed && player1Velocity.y < 0) {
            player1Velocity.y *= 0.5f;
        }
        if (!player2JumpPressed && player2Velocity.y < 0) {
            player2Velocity.y *= 0.5f;
        }
        
        if (player1Grounded) {
            player1DoubleJumpAvailable = false;
        }
        if (player2Grounded) {
            player2DoubleJumpAvailable = false;
        }

        // camera follow player
        float minX = windowWidth / 2;
        float maxX = (level.mapWidth * level.tileSize) - windowWidth / 2;

        float minY = windowHeight / 2;
        float maxY = (level.mapHeight * level.tileSize) - windowHeight / 2;

        sf::Vector2f diff = player1.getPosition() - player2.getPosition();
        float horizontalDistance = std::abs(diff.x) + 200;
        float verticalDistance = std::abs(diff.y) + 200;

        float zoomFactor = std::max(horizontalDistance / view.getSize().x, verticalDistance / view.getSize().y);

        float targetWidth = view.getSize().x * (1.f * zoomFactor);
        float targetHeight = view.getSize().y * (1.f * zoomFactor);

        targetWidth = std::clamp(targetWidth, 1280.f, 4000.f);
        targetHeight = std::clamp(targetHeight, 720.f, 8000.f);
        view.setSize(lerpVector(view.getSize(), sf::Vector2f(targetWidth, targetHeight), 5.f * delta));

        averagePos = (player1.getPosition() + player2.getPosition()) / 2.f;
        cameraPos = lerpVector(cameraPos, averagePos, 5.f * delta);

        cameraPos.x = std::clamp(cameraPos.x, minX, maxX);
        cameraPos.y = std::clamp(cameraPos.y, minY, maxY);

        sf::FloatRect viewRect(view.getCenter() - view.getSize() / 2.f, view.getSize());

        view.setCenter(cameraPos);
        window.setView(view);

        window.clear(sf::Color(111, 194, 118));

        level.draw(window);

        window.draw(player1);
        window.draw(player2);
		
        if (!viewRect.contains(point))
        {
            window.draw(arrow1);
            window.draw(arrow2);
		}

        paramText.setString(
            "Parametry skoku:\n" +
            std::string("Max wysokosc: ") + std::to_string(static_cast<int>(maxJumpHeight)) + " px\n" +
            std::string("Dystans poziomy: ") + std::to_string(static_cast<int>(horizontalDistanceToApex)) + " px\n" +
            std::string("v_0: ") + std::to_string(static_cast<int>(jumpForce)) + " px/s\n" +
            std::string("g: ") + std::to_string(static_cast<int>(gravity)) + " px/s^2\n\n" +
            "[1/2] Wysokosc  [3/4] Dystans\n" +
            "Podwojny skok, Zmienna wys., Szybkie spadanie"
        );
        paramText.setPosition({ view.getCenter().x - view.getSize().x / 2.f + 10.f,
                             view.getCenter().y - view.getSize().y / 2.f + 10.f });
        window.draw(paramText);

        window.display();
    }
}

void checkWallCollisions(Ball& ball, int windowWidth, int windowHeight) {
    float wallBounciness = 1.f;

    if (ball.position.x - ball.radius < 0) {
        ball.position.x = ball.radius;
        ball.velocity.x *= -wallBounciness;
    }
    else if (ball.position.x + ball.radius > windowWidth) {
        ball.position.x = windowWidth - ball.radius;
        ball.velocity.x *= -wallBounciness;
    }

    if (ball.position.y - ball.radius < 0) {
        ball.position.y = ball.radius;
        ball.velocity.y *= -wallBounciness;
    }
    else if (ball.position.y + ball.radius > windowHeight) {
        ball.position.y = windowHeight - ball.radius;
        ball.velocity.y *= -wallBounciness;
    }
}

sf::Vector2f reflect(const sf::Vector2f& N, const sf::Vector2f& V) {
    float dot = N.x * V.x + N.y * V.y;
    return V - 2.0f * dot * N;
}

void runBallsSimulation() {
    std::string title = "Symulacja pilek";
    int windowWidth = 1280;
    int windowHeight = 720;
    sf::RenderWindow window(sf::VideoMode({ 1280, 720 }), title);
    sf::Clock clock;
    std::vector<Ball> balls;
    int ballsNumber = 10;

    bool enableSeparation = true;
    bool enableBounce = true;
    
    for (int i = 0; i < ballsNumber; i++)
    {
        balls.push_back(Ball());
    }

    while (window.isOpen())
    {
        float delta = clock.restart().asSeconds();
        while (const std::optional event = window.pollEvent()) // obsluga zdarzen
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if (keyPressed->scancode == sf::Keyboard::Scancode::Num1) {
                    enableSeparation = !enableSeparation;
                }
                else if (keyPressed->scancode == sf::Keyboard::Scancode::Num2) {
                    enableBounce = !enableBounce;
                }
                std::string statusStr = "Sterowanie:\n";
                statusStr += "[S] Separacja: " + std::string(enableSeparation ? "ON" : "OFF") + "\n";
                statusStr += "[B] Odbicia:   " + std::string(enableBounce ? "ON" : "OFF");

                std::cout << statusStr << std::endl;
            }
        }


        window.clear(sf::Color::Black);

        for (auto& ball : balls) {
            checkWallCollisions(ball, windowWidth, windowHeight);
            ball.update(delta);
        }
        
        for (size_t i = 0; i < balls.size(); ++i) {
            for (size_t j = i + 1; j < balls.size(); ++j) {

                Ball& b1 = balls[i];
                Ball& b2 = balls[j];

                sf::Vector2f delta = b2.position - b1.position;
                float distanceSquared = delta.x * delta.x + delta.y * delta.y;
                float sumRadii = b1.radius + b2.radius;

                if (distanceSquared < sumRadii * sumRadii) {

                    float distance = std::sqrt(distanceSquared);

                    if (distance == 0.0f) { distance = 0.0001f; delta = sf::Vector2f(1.0f, 0.0f); }

                    sf::Vector2f normal = delta / distance;

                    if (enableSeparation) {
                        float overlap = sumRadii - distance;

                        sf::Vector2f separationVector = normal * (overlap * 0.5f);

                        b1.position -= separationVector;
                        b2.position += separationVector;
                    }
                    
                    if (enableBounce) {
                        sf::Vector2f relativeVelocity = b2.velocity - b1.velocity;

                        float velocityAlongNormal = relativeVelocity.x * normal.x + relativeVelocity.y * normal.y;

                        if (velocityAlongNormal < 0) {

                            b1.velocity = reflect(normal, b1.velocity);
                            b2.velocity = reflect(-normal, b2.velocity);;
                        }
                    }
                    
                }
            }
        }

        for (auto& ball : balls) {
            ball.shape.setPosition(ball.position);
            window.draw(ball.shape);
        }
        window.display();
    }
}



int main()
{
	runGame();
	//runBallsSimulation();
	return 0;
}

