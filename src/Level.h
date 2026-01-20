#pragma once

#include <string>
#include <vector>
#include <SFML/Graphics.hpp>
#include "Game.h"

class Level
{
public:
	int windowWidth;
	int windowHeight;

	int mapWidth;
	int mapHeight;

	int tileSize;

	std::string filePath;
	sf::Texture wallTexture;
	sf::Texture floorTexture;
	std::vector<sf::RectangleShape> walls;
	std::vector<sf::RectangleShape> floors;
	std::vector<sf::RectangleShape> points;
	std::vector<sf::Vector2f> pointLocations;

	Level(int windowWidth, int windowHeight);
	~Level();

	bool loadFromFile(std::string filePath);
	void draw(sf::RenderWindow& window);
	std::vector<sf::RectangleShape>& getWalls();
	std::vector<sf::RectangleShape>& getFloors();
	std::vector<sf::Vector2f>& getPointLocations();

private:

};