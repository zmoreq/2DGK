#include <SFML/Graphics.hpp>
#include <string>
#include <iostream>



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
	std::string title = "Zadanie 3";
    sf::RenderWindow window(sf::VideoMode({ 1280, 720 }), title);

    sf::CircleShape circle(50.f);
    sf::RectangleShape rect(sf::Vector2f(50, 50));

    const sf::Texture rectTexture("../src/textures/texture1.png");
    const sf::Texture circleTexture("../src/textures/texture2.png");

    circle.setPosition(sf::Vector2f(10, 10));
    rect.setPosition(sf::Vector2f(640, 10));

    circle.setTexture(&circleTexture);
    circle.setFillColor(sf::Color(255, 255, 255, 128));
    rect.setTexture(&rectTexture);

    sf::Clock clock;
    float circleSpeed = 200.f;
    float rectSpeed = 200.f;
    sf::Vector2f player1Direction = sf::Vector2f(0.f, 0.f);
    sf::Vector2f player2Direction = sf::Vector2f(0.f, 0.f);

    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent()) // obsluga zdarzen
        {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        player1Direction = getDirectionFromKeyboard();

        float delta = clock.restart().asSeconds();

        rect.move(player1Direction * rectSpeed * delta); //player1

        // player2
        sf::Vector2f circleTargetPosition = sf::Vector2f(sf::Mouse::getPosition(window));
        sf::Vector2f circleCurrentPosition = circle.getPosition();

        sf::Vector2f circleNewPosition = lerpVector(circleCurrentPosition, circleTargetPosition, 2.f * delta);
        circle.setPosition(circleNewPosition);


        window.clear(sf::Color(111, 194, 118));
        window.draw(circle);
        window.draw(rect);
        window.display();
    }
}

