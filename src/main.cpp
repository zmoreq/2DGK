#include <SFML/Graphics.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include "Level.h"
#include "Game.h"
#include "Ball.h"


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


    player1.setPosition(sf::Vector2f(720, 360));
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

        player1.move(player1Direction * playerSpeed * delta); //player1
        player2.move(player2Direction * playerSpeed * delta); //player2

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

        window.draw(player1);
        window.draw(player2);
        level.draw(window);

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
	//runGame();
	runBallsSimulation();
	return 0;
}

