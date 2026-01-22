#include <SFML/Graphics.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include "Level.h"
#include "Game.h"
#include "Ball.h"
#include <cmath>


sf::Vector2f getDirectionPlayer() {
    sf::Vector2f direction(0.f, 0.f);
    
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) || 
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) direction.x -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) || 
        sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) direction.x += 1.f;
    
    return direction;
}

static sf::Vector2f lerpVector(sf::Vector2f start, sf::Vector2f end, float delta) {
    return start + (end - start) * delta;
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

void updatePointsVisuals(Level& level, sf::Vector2f activePoint) {
    for (auto& p : level.points) {
        if (p.getPosition() == activePoint) {
            p.setFillColor(sf::Color(255, 255, 0, 255));
        }
        else {
            p.setFillColor(sf::Color(255, 255, 0, 0));
        }
    }
}

void runGame() {
    std::string title = "2DGK - Parallax";
    int windowWidth = 1280;
    int windowHeight = 720;
    sf::Vector2f cameraPos = sf::Vector2f(windowWidth / 2.f, windowHeight / 2.f);
    sf::RenderWindow window(sf::VideoMode({ 1280, 720 }), title);

    sf::RectangleShape player(sf::Vector2f(100, 100));
    player.setOrigin({ 50.f, 50.f });

    const sf::Texture playerTexture("../../../../textures/player.png");
    player.setPosition(sf::Vector2f(770, 0));
    player.setTexture(&playerTexture);
    player.setFillColor(sf::Color(255, 0, 0, 255));

    sf::Clock clock;
    float playerSpeed = 200.f;
    
    float maxJumpHeight = 150.f;
    float horizontalDistanceToApex = 100.f;
    float gravity = 800.f;
    float jumpForce = -450.f;
    
    calculateJumpParameters(maxJumpHeight, horizontalDistanceToApex, playerSpeed, gravity, jumpForce);
    
    sf::Vector2f playerVelocity(0.f, 0.f);
    sf::Vector2f playerAcceleration(0.f, gravity);
    bool playerGrounded = false;
    
    float playerCoyoteTime = 0.f;
    const float coyoteTimeDuration = 0.1f;
    
    bool playerDoubleJumpAvailable = false;
    bool playerJumpPressed = false;

    sf::Font font;
    if (!font.openFromFile("C:/Windows/Fonts/arial.ttf")) {
        std::cout << "Failed to load font" << std::endl;
    }
    
    sf::Text paramText(font);
    paramText.setCharacterSize(20);
    paramText.setFillColor(sf::Color::White);
    paramText.setOutlineColor(sf::Color::Black);
    paramText.setOutlineThickness(2);

    std::cout << "=== STEROWANIE ===" << std::endl;
    std::cout << "[Left/Right lub A/D] - Ruch" << std::endl;
    std::cout << "[Up lub W] - Skok (trzymaj = wyzszy skok, podwojny skok)" << std::endl;
    std::cout << "[Down lub S] - Szybkie spadanie" << std::endl;
    std::cout << "[1/2] - Zwieksz/Zmniejsz max wysokosc" << std::endl;
    std::cout << "[3/4] - Zwieksz/Zmniejsz dystans poziomy" << std::endl;
    std::cout << "[5/6] - Parallax FAR speed" << std::endl;
    std::cout << "[7/8] - Parallax MID speed" << std::endl;
    std::cout << "[9/0] - Parallax NEAR speed" << std::endl;
    std::cout << "=======================" << std::endl;

    Level level(window.getSize().x, window.getSize().y);
    level.loadFromFile("../../../../src/level1.txt");

	sf::Vector2f point;
    std::vector<sf::Vector2f>& locations = level.getPointLocations();
    if (!locations.empty()) {
        point = locations[0];
    }

    sf::View view({ player.getPosition().x, player.getPosition().y }, { (float)windowWidth, (float)windowHeight });
    window.setView(view);

	updatePointsVisuals(level, point);

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
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
                else if (keyPressed->scancode == sf::Keyboard::Scancode::Num5) {
                    level.parallaxFarSpeed = std::clamp(level.parallaxFarSpeed + 0.1f, 0.f, 1.f);
                    std::cout << "Parallax Far Speed: " << level.parallaxFarSpeed << std::endl;
                }
                else if (keyPressed->scancode == sf::Keyboard::Scancode::Num6) {
                    level.parallaxFarSpeed = std::clamp(level.parallaxFarSpeed - 0.1f, 0.f, 1.f);
                    std::cout << "Parallax Far Speed: " << level.parallaxFarSpeed << std::endl;
                }
                else if (keyPressed->scancode == sf::Keyboard::Scancode::Num7) {
                    level.parallaxMidSpeed = std::clamp(level.parallaxMidSpeed + 0.1f, 0.f, 1.f);
                    std::cout << "Parallax Mid Speed: " << level.parallaxMidSpeed << std::endl;
                }
                else if (keyPressed->scancode == sf::Keyboard::Scancode::Num8) {
                    level.parallaxMidSpeed = std::clamp(level.parallaxMidSpeed - 0.1f, 0.f, 1.f);
                    std::cout << "Parallax Mid Speed: " << level.parallaxMidSpeed << std::endl;
                }
                else if (keyPressed->scancode == sf::Keyboard::Scancode::Num9) {
                    level.parallaxNearSpeed = std::clamp(level.parallaxNearSpeed + 0.1f, 0.f, 1.f);
                    std::cout << "Parallax Near Speed: " << level.parallaxNearSpeed << std::endl;
                }
                else if (keyPressed->scancode == sf::Keyboard::Scancode::Num0) {
                    level.parallaxNearSpeed = std::clamp(level.parallaxNearSpeed - 0.1f, 0.f, 1.f);
                    std::cout << "Parallax Near Speed: " << level.parallaxNearSpeed << std::endl;
                }
                
                if (paramsChanged) {
                    calculateJumpParameters(maxJumpHeight, horizontalDistanceToApex, playerSpeed, gravity, jumpForce);
                    playerAcceleration.y = gravity;
                    
                    std::cout << "\n=== Parametry skoku ===" << std::endl;
                    std::cout << "Max wysokosc: " << maxJumpHeight << " px" << std::endl;
                    std::cout << "Dystans poziomy: " << horizontalDistanceToApex << " px" << std::endl;
                    std::cout << "v_0: " << jumpForce << " px/s" << std::endl;
                    std::cout << "g: " << gravity << " px/s^2" << std::endl;
                    std::cout << "=======================" << std::endl;
                }
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
            window.close();

        sf::Vector2f playerDirection = getDirectionPlayer();
        float delta = clock.restart().asSeconds();
        
        if (playerGrounded) {
            playerCoyoteTime = coyoteTimeDuration;
        } else {
            playerCoyoteTime -= delta;
        }

        bool playerJumpPressedNow = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) || 
                                     sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W);
        if (playerJumpPressedNow && !playerJumpPressed) {
            if (playerGrounded || playerCoyoteTime > 0.f) {
                playerVelocity.y = jumpForce;
                playerDoubleJumpAvailable = true;
                playerCoyoteTime = 0.f;
            }
            else if (playerDoubleJumpAvailable) {
                playerVelocity.y = jumpForce;
                playerDoubleJumpAvailable = false;
            }
        }
        playerJumpPressed = playerJumpPressedNow;

        playerVelocity.x = playerDirection.x * playerSpeed;
        
        bool playerFastFall = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down) || 
                              sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S);

        updatePlayerPhysics(level, player, playerVelocity, playerAcceleration, delta, playerGrounded, gravity, playerFastFall);

        if (!playerJumpPressed && playerVelocity.y < 0) {
            playerVelocity.y *= 0.5f;
        }
        
        if (playerGrounded) {
            playerDoubleJumpAvailable = false;
        }

        float minX = windowWidth / 2.f;
        float maxX = (level.mapWidth * level.tileSize) - windowWidth / 2.f;
        float minY = windowHeight / 2.f;
        float maxY = (level.mapHeight * level.tileSize) - windowHeight / 2.f;

        if (maxX < minX) minX = maxX = (level.mapWidth * level.tileSize) / 2.f;
        if (maxY < minY) minY = maxY = (level.mapHeight * level.tileSize) / 2.f;

        cameraPos = lerpVector(cameraPos, player.getPosition(), 5.f * delta);
        cameraPos.x = std::clamp(cameraPos.x, minX, maxX);
        cameraPos.y = std::clamp(cameraPos.y, minY, maxY);

        view.setCenter(cameraPos);
        window.setView(view);

		window.clear(sf::Color(0, 0, 0));

		level.drawParallax(window, view);
        level.draw(window);
        window.draw(player);

        paramText.setString(
            "Parametry skoku:\n" +
            std::string("Max wysokosc: ") + std::to_string(static_cast<int>(maxJumpHeight)) + " px\n" +
            std::string("Dystans: ") + std::to_string(static_cast<int>(horizontalDistanceToApex)) + " px\n" +
            std::string("v_0: ") + std::to_string(static_cast<int>(jumpForce)) + " px/s\n" +
            std::string("g: ") + std::to_string(static_cast<int>(gravity)) + " px/s^2\n\n" +
            "[1/2] Wysokosc  [3/4] Dystans\n" +
            "[5/6] FAR  [7/8] MID  [9/0] NEAR\n" +
            "Parallax: " + std::to_string(level.parallaxFarSpeed).substr(0,3) + " / " + 
                   std::to_string(level.parallaxMidSpeed).substr(0,3) + " / " + 
                   std::to_string(level.parallaxNearSpeed).substr(0,3)
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
        while (const std::optional event = window.pollEvent())
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
                            b2.velocity = reflect(-normal, b2.velocity);
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

