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

	int tileSize = std::min(windowWidth / mapWidth, windowHeight / mapHeight);


	std::string filePath;
	sf::Texture wallTexture;
	std::vector<sf::RectangleShape> walls;

	Level(int windowWidth, int windowHeight);
	~Level();

	bool loadFromFile(std::string filePath);
	void draw(sf::RenderWindow& window);

private:

};