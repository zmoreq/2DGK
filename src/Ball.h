#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <SFML/Graphics.hpp>


class Ball {

public:
	float radius;
	sf::Vector2f origin;
	sf::CircleShape shape;
	sf::Vector2f velocity;

	sf::Vector2f position;

	Ball();
	void update(float delta);
	sf::Vector2f getRandomVelocity(float minSpeed, float maxSpeed);
	~Ball();
};