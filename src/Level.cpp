#include "Level.h"
#include <iostream>
#include <fstream>

Level::Level(int windowWidth, int windowHeight)
{
	this->windowWidth = windowWidth;
	this->windowHeight = windowHeight;
	this->mapWidth = 1;
	this->mapHeight = 1;
	this->wallTexture.loadFromFile("../../../../textures/texture2.png");
}

Level::~Level()
{
}

bool Level::loadFromFile(std::string filePath)
{
	std::ifstream file(filePath);
	this->mapHeight = 0;
	this->mapWidth = 0;

	std::vector<std::string> lines;
	std::string line;

	if (file.is_open())
	{

		while (std::getline(file, line)) {
			if (!line.empty()) {
				lines.push_back(line);
			}
		}
		file.close();
		mapHeight = lines.size();
		mapWidth = lines[0].length();

		this->tileSize = std::min(this->windowWidth / this->mapWidth, this->windowHeight / this->mapHeight);
		for (int i = 0; i < mapHeight; i++)
		{
			for (int j = 0; j < mapWidth; j++)
			{
				char c = lines[i][j];
				if (c == '#')
				{
					sf::RectangleShape rect(sf::Vector2f(tileSize, tileSize));
					rect.setTexture(&wallTexture);
					rect.setPosition(sf::Vector2f(j * tileSize, i * tileSize));
					walls.push_back(rect);
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
}