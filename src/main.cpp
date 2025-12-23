#include <SFML/Graphics.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include "Level.h"
#include "Game.h"
#include "Ball.h"
#include <cmath>


sf::Vector2f getDirectionPlayer1() {

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) return sf::Vector2f(-1.f, 0.f);
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) return sf::Vector2f(1.f, 0.f);
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) return sf::Vector2f(0.f, -1.f);
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) return sf::Vector2f(0.f, 1.f);

    return sf::Vector2f(0.f, 0.f);
}

sf::Vector2f getDirectionPlayer2() {

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) return sf::Vector2f(-1.f, 0.f);
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) return sf::Vector2f(1.f, 0.f);
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) return sf::Vector2f(0.f, -1.f);
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) return sf::Vector2f(0.f, 1.f);

    return sf::Vector2f(0.f, 0.f);
}

static sf::Vector2f lerpVector(sf::Vector2f start, sf::Vector2f end, float delta) {
    return start + (end - start) * delta;
}

void updatePlayerPhysics(Level& level, sf::RectangleShape& player, sf::Vector2f velocity) {
    player.move({ velocity.x, 0.f });

    for (auto& wall : level.getWalls()) {
        if (player.getGlobalBounds().findIntersection(wall.getGlobalBounds())) {
            std::cout << "Collision detected!" << std::endl;

            if (velocity.x > 0) {
                player.setPosition({ wall.getPosition().x - player.getSize().x / 2.f, player.getPosition().y });
            }
            else if (velocity.x < 0) {
                player.setPosition({ wall.getPosition().x + wall.getSize().x + player.getSize().x / 2.f, player.getPosition().y });
            }
        }
    }

	player.move({ 0.f, velocity.y });

    for (auto& wall : level.getWalls()) {
        if (player.getGlobalBounds().findIntersection(wall.getGlobalBounds())) {
            std::cout << "Collision detected!" << std::endl;
            if (velocity.y > 0) {
                player.setPosition({ player.getPosition().x, wall.getPosition().y - player.getSize().y / 2.f });
            }
            else if (velocity.y < 0) {
                player.setPosition({ player.getPosition().x, wall.getPosition().y + wall.getSize().y + player.getSize().y / 2.f });
            }
        }
	}
}

void changePointLocation(sf::Vector2f& point, sf::Vector2f location1, sf::Vector2f location2, Level level) {
	std::vector<sf::Vector2f> locations = level.getPointLocations();

    for (int i = 0; i < locations.size(); ++i) {
        if (location1 != locations[i] && location2 != locations[i]) {
			point = locations[i];
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

void handlePointLocations(sf::RectangleShape& player1, sf::RectangleShape& player2, Level level, sf::Vector2f& point) {
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
    sf::RectangleShape player2(sf::Vector2f(50, 50));

    player1.setOrigin({ 25.f, 25.f });
    player2.setOrigin({ 25.f, 25.f });

    const sf::Texture rectTexture("../../../../textures/texture1.png");

	sf::Vector2f point = sf::Vector2f(100.f, 100.f);

    player1.setPosition(sf::Vector2f(770, 360));
    player2.setPosition(sf::Vector2f(560, 360));

    player1.setTexture(&rectTexture);
    player2.setTexture(&rectTexture);

    player1.setFillColor(sf::Color(255, 0, 0, 255));
    player2.setFillColor(sf::Color(0, 0, 255, 255));

    sf::Clock clock;
    float playerSpeed = 200.f;
    sf::Vector2f player1Direction = sf::Vector2f(0.f, 0.f);
    sf::Vector2f player2Direction = sf::Vector2f(0.f, 0.f);

    // test
    Level level(window.getSize().x, window.getSize().y);
    level.loadFromFile("../../../../src/level1.txt");

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

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent()) // obsluga zdarzen
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
            window.close();

        player1Direction = getDirectionPlayer1();
        player2Direction = getDirectionPlayer2();

        float delta = clock.restart().asSeconds();

		sf::Vector2f player1Velocity = player1Direction * playerSpeed * delta;
        updatePlayerPhysics(level, player1, player1Velocity);
		
		sf::Vector2f player2Velocity = player2Direction * playerSpeed * delta;
		updatePlayerPhysics(level, player2, player2Velocity);

        handlePointLocations(player1, player2, level, point);

        updateArrowDirection(arrow1, player1.getPosition(), point);
        updateArrowDirection(arrow2, player2.getPosition(), point);

        //camera follow player
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

        view.setCenter(cameraPos);
        window.setView(view);

        window.clear(sf::Color(111, 194, 118));

        level.draw(window);

        window.draw(player1);
        window.draw(player2);
		
        
        window.draw(arrow1);
        window.draw(arrow2);

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

