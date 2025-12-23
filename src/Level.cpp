#include "Level.h"
#include <iostream>
#include <fstream>

Level::Level(int windowWidth, int windowHeight)
{
	this->windowWidth = windowWidth;
	this->windowHeight = windowHeight;
	this->mapWidth = 1;
	this->mapHeight = 1;
	this->tileSize = 64;
}

Level::~Level()
{
}

std::vector<sf::RectangleShape>& Level::getWalls()
{
	return walls;
}

std::vector<sf::Vector2f>& Level::getPointLocations()
{
	return pointLocations;
}

bool Level::loadFromFile(std::string filePath)
{
	std::ifstream file(filePath);
	this->mapHeight = 0;
	this->mapWidth = 0;

	std::vector<std::string> lines;
	std::string line;

	bool readingMap = false;
	bool readingTextures = false;

	if (file.is_open())
	{

		while (std::getline(file, line)) {
			if (line == "[textures]")
			{
				readingTextures = true;
				continue;
			}

			if (line == "[map]")
			{
				readingMap = true;
				continue;
			}

			if (!line.empty() && readingTextures) {

				int pos = line.find("=");

				if (pos >= 0) {

					char key = line[0];
					std::string texturePath = line.substr(pos + 1);

					if (key == 'w') {
						if (!wallTexture.loadFromFile(texturePath)) {
							std::cout << "Failed to load wall texture from " << texturePath << std::endl;
						}
					}
					else if (key == 'f') {
						if (!floorTexture.loadFromFile(texturePath)) {
							std::cout << "Failed to load floor texture from " << texturePath << std::endl;
						}
					}
					else {
						readingTextures = false;
					}
				}
			}
			
			if (!line.empty() && readingMap) {
				lines.push_back(line);
			}
		}
		file.close();
		mapHeight = lines.size();
		mapWidth = lines[0].length();

		for (int i = 0; i < mapHeight; i++)
		{
			for (int j = 0; j < mapWidth; j++)
			{
				char c = lines[i][j];
				if (c == 'w')
				{
					sf::RectangleShape rect(sf::Vector2f(tileSize, tileSize));
					rect.setTexture(&wallTexture);
					rect.setPosition(sf::Vector2f(j * tileSize, i * tileSize));
					walls.push_back(rect);
				}
				else if (c == 'p')
				{
					sf::RectangleShape rect(sf::Vector2f(tileSize, tileSize));
					rect.setTexture(&floorTexture);
					rect.setFillColor(sf::Color(255, 255, 0));
					rect.setSize(sf::Vector2f(tileSize / 2.f, tileSize / 2.f));
					rect.setPosition(sf::Vector2f(j * tileSize + tileSize / 2.f, i * tileSize + tileSize / 2.f));
					rect.setOrigin(sf::Vector2f(tileSize / 4.f, tileSize / 4.f));
					points.push_back(rect);
					pointLocations.push_back(sf::Vector2f(j * tileSize + tileSize / 2.f, i * tileSize + tileSize / 2.f));
				}
			}
		}

		return true;
	}
	else
	{
		std::cout << "Unable to open file";
		return false;
	}
}

void Level::draw(sf::RenderWindow& window)
{
	for (auto& wall : walls)
	{
		window.draw(wall);
	}
	for (auto& point : points)
	{
		window.draw(point);
	}
}