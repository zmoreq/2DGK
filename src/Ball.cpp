#include "Ball.h"
#include <random>

Ball::Ball() {
	this->radius = 25.f;
	this->origin = sf::Vector2f(radius, radius);
	this->shape = sf::CircleShape(radius);
	this->velocity = getRandomVelocity(150.f, 400.f);

	this->position = shape.getPosition();
	shape.setOrigin(origin);
	shape.setPosition(sf::Vector2f(400.f, 300.f));
	shape.setFillColor(sf::Color::Red);
}

void Ball::update(float delta) {
	position += velocity * delta;
	shape.setPosition(position);
}

sf::Vector2f Ball::getRandomVelocity(float minSpeed, float maxSpeed) {
	static std::random_device rd;
	static std::mt19937 gen(rd());

	std::uniform_real_distribution<float> angleDist(0.f, 2.f * 3.14159f);
	std::uniform_real_distribution<float> speedDist(minSpeed, maxSpeed);

	float angle = angleDist(gen);
	float speed = speedDist(gen);

	return sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed);
}

Ball::~Ball() {
}