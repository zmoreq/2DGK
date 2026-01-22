#pragma once

#include <string>
#include <vector>
#include <SFML/Graphics.hpp>
#include "Game.h"

class Level
{
public:
	Level(int windowWidth, int windowHeight);
	~Level();

	std::vector<sf::RectangleShape>& getWalls();
	std::vector<sf::RectangleShape>& getFloors();
	std::vector<sf::Vector2f>& getPointLocations();

	bool loadFromFile(std::string filePath);
	void draw(sf::RenderWindow& window);
	void drawParallax(sf::RenderWindow& window, sf::View& view);

	int mapWidth;
	int mapHeight;
	int tileSize;
	int parallaxTileSize;

	std::vector<sf::RectangleShape> points;
	
	// 3 warstwy paralaksy (UPROSZCZONE - bez wrappingu)
	std::vector<sf::RectangleShape> parallaxFar;
	std::vector<sf::RectangleShape> parallaxMid;
	std::vector<sf::RectangleShape> parallaxNear;
	
	// Prędkości paralaksy (zmieniane w runtime)
	float parallaxFarSpeed = 0.3f;   // 0.0-1.0 (0=nie rusza się, 1=jak gameplay)
	float parallaxMidSpeed = 0.6f;
	float parallaxNearSpeed = 0.9f;
	
	// WYSOKOŚĆ warstw (offset Y w pikselach) - DODANE!
	float parallaxFarOffsetY = -50.f;    // Dodatnie = w GÓRĘ, ujemne = w DÓŁ
	float parallaxMidOffsetY = 110.f;
	float parallaxNearOffsetY = 200.f;

private:
	int windowWidth;
	int windowHeight;

	std::vector<sf::RectangleShape> walls;
	std::vector<sf::RectangleShape> floors;
	std::vector<sf::Vector2f> pointLocations;

	// Tekstury gameplay
	sf::Texture wallTexture;
	sf::Texture floorTexture;
	
	// 6 tekstur paralaksy
	sf::Texture parallaxTile1;
	sf::Texture parallaxTile2;
	sf::Texture parallaxTile3;
	sf::Texture parallaxTile4;
	sf::Texture parallaxTile5;
	sf::Texture parallaxTile6;
};