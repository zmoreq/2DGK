#include <SFML/Graphics.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include "Level.h"
#include "Game.h"


sf::Vector2f getDirectionFromKeyboard() {

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) return sf::Vector2f(-1.f, 0.f);
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) return sf::Vector2f(1.f, 0.f);
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) return sf::Vector2f(0.f, -1.f);
    else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) return sf::Vector2f(0.f, 1.f);

    return sf::Vector2f(0.f, 0.f);
}

static sf::Vector2f lerpVector(sf::Vector2f start, sf::Vector2f end, float delta) {
    return start + (end - start) * delta;
}

int main()
{
	std::string title = "2DGK";
	int windowWidth = 1280;
	int windowHeight = 720;
	sf::Vector2f cameraPos = sf::Vector2f(windowWidth / 2.f, windowHeight / 2.f);
    sf::RenderWindow window(sf::VideoMode({ 1280, 720 }), title);

    sf::RectangleShape player(sf::Vector2f(50, 50));

    const sf::Texture rectTexture("../../../../textures/texture1.png");
    

    player.setPosition(sf::Vector2f(720, 360));

    player.setTexture(&rectTexture);
	player.setFillColor(sf::Color(255, 255, 255, 125));

    sf::Clock clock;
    float playerSpeed = 200.f;
    sf::Vector2f player1Direction = sf::Vector2f(0.f, 0.f);

    // test
    Level level(window.getSize().x, window.getSize().y);
	level.loadFromFile("../../../../src/level1.txt");

    //view
    sf::View view({ player.getPosition().x, player.getPosition().y}, {(float)windowWidth, (float)windowHeight});
    window.setView(view);

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent()) // obsluga zdarzen
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        player1Direction = getDirectionFromKeyboard();

        float delta = clock.restart().asSeconds();

        player.move(player1Direction * playerSpeed * delta); //player1

		//camera follow player
        float minX = windowWidth / 2;
        float maxX = (level.mapWidth * level.tileSize) - windowWidth / 2;

        float minY = windowHeight / 2;
        float maxY = (level.mapHeight * level.tileSize) - windowHeight / 2;

		cameraPos = lerpVector(cameraPos, player.getPosition(), 5.f * delta);

        cameraPos.x = std::clamp(cameraPos.x, minX, maxX);

        cameraPos.y = std::clamp(cameraPos.y, minY, maxY);
		view.setCenter(cameraPos);

		window.setView(view);

        window.clear(sf::Color(111, 194, 118));

        window.draw(player);
		level.draw(window);

        window.display();
    }
}

