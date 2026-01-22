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
    this->parallaxTileSize = 256;
}

Level::~Level()
{
}

std::vector<sf::RectangleShape>& Level::getWalls()
{
    return walls;
}

std::vector<sf::RectangleShape>& Level::getFloors()
{
    return floors;
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
    std::vector<std::string> parallaxFarLines;
    std::vector<std::string> parallaxMidLines;
    std::vector<std::string> parallaxNearLines;
    std::string line;

    bool readingMap = false;
    bool readingTextures = false;
    bool readingParallaxFar = false;
    bool readingParallaxMid = false;
    bool readingParallaxNear = false;

    if (file.is_open())
    {
        while (std::getline(file, line)) {
            if (line == "[textures]") {
                readingTextures = true;
                readingMap = false;
                readingParallaxFar = false;
                readingParallaxMid = false;
                readingParallaxNear = false;
                continue;
            }
            if (line == "[parallax_far]") {
                readingParallaxFar = true;
                readingTextures = false;
                readingMap = false;
                readingParallaxMid = false;
                readingParallaxNear = false;
                continue;
            }
            if (line == "[parallax_mid]") {
                readingParallaxMid = true;
                readingTextures = false;
                readingMap = false;
                readingParallaxFar = false;
                readingParallaxNear = false;
                continue;
            }
            if (line == "[parallax_near]") {
                readingParallaxNear = true;
                readingTextures = false;
                readingMap = false;
                readingParallaxFar = false;
                readingParallaxMid = false;
                continue;
            }
            if (line == "[map]") {
                readingMap = true;
                readingTextures = false;
                readingParallaxFar = false;
                readingParallaxMid = false;
                readingParallaxNear = false;
                continue;
            }

            if (!line.empty() && readingTextures) {
                int pos = line.find("=");
                if (pos >= 0) {
                    std::string key = line.substr(0, pos);
                    std::string texturePath = line.substr(pos + 1);

                    if (key == "w") {
                        if (!wallTexture.loadFromFile(texturePath)) {
                            std::cout << "Failed to load wall texture" << std::endl;
                        }
                    }
                    else if (key == "f") {
                        if (!floorTexture.loadFromFile(texturePath)) {
                            std::cout << "Failed to load floor texture" << std::endl;
                        }
                    }
                    else if (key == "p1") {
                        if (!parallaxTile1.loadFromFile(texturePath)) {
                            std::cout << "Failed to load parallax tile 1" << std::endl;
                        }
                    }
                    else if (key == "p2") {
                        if (!parallaxTile2.loadFromFile(texturePath)) {
                            std::cout << "Failed to load parallax tile 2" << std::endl;
                        }
                    }
                    else if (key == "p3") {
                        if (!parallaxTile3.loadFromFile(texturePath)) {
                            std::cout << "Failed to load parallax tile 3" << std::endl;
                        }
                    }
                    else if (key == "p4") {
                        if (!parallaxTile4.loadFromFile(texturePath)) {
                            std::cout << "Failed to load parallax tile 4" << std::endl;
                        }
                    }
                    else if (key == "p5") {
                        if (!parallaxTile5.loadFromFile(texturePath)) {
                            std::cout << "Failed to load parallax tile 5" << std::endl;
                        }
                    }
                    else if (key == "p6") {
                        if (!parallaxTile6.loadFromFile(texturePath)) {
                            std::cout << "Failed to load parallax tile 6" << std::endl;
                        }
                    }
                }
            }
            
            if (!line.empty() && readingParallaxFar) {
                parallaxFarLines.push_back(line);
            }
            if (!line.empty() && readingParallaxMid) {
                parallaxMidLines.push_back(line);
            }
            if (!line.empty() && readingParallaxNear) {
                parallaxNearLines.push_back(line);
            }
            if (!line.empty() && readingMap) {
                lines.push_back(line);
            }
        }
        file.close();

        for (int i = 0; i < parallaxFarLines.size(); i++) {
            for (int j = 0; j < parallaxFarLines[i].length(); j++) {
                char c = parallaxFarLines[i][j];
                
                sf::Texture* texture = nullptr;
                if (c == '1') texture = &parallaxTile1;
                else if (c == '2') texture = &parallaxTile2;
                
                if (texture != nullptr) {
                    sf::RectangleShape rect(sf::Vector2f(parallaxTileSize, parallaxTileSize));
                    rect.setTexture(texture);
                    rect.setPosition(sf::Vector2f(j * parallaxTileSize, i * parallaxTileSize));
                    parallaxFar.push_back(rect);
                }
            }
        }

        for (int i = 0; i < parallaxMidLines.size(); i++) {
            for (int j = 0; j < parallaxMidLines[i].length(); j++) {
                char c = parallaxMidLines[i][j];
                
                sf::Texture* texture = nullptr;
                if (c == '3') texture = &parallaxTile3;
                else if (c == '4') texture = &parallaxTile4;
                
                if (texture != nullptr) {
                    sf::RectangleShape rect(sf::Vector2f(parallaxTileSize, parallaxTileSize));
                    rect.setTexture(texture);
                    rect.setPosition(sf::Vector2f(j * parallaxTileSize, i * parallaxTileSize));
                    parallaxMid.push_back(rect);
                }
            }
        }

        for (int i = 0; i < parallaxNearLines.size(); i++) {
            for (int j = 0; j < parallaxNearLines[i].length(); j++) {
                char c = parallaxNearLines[i][j];
                
                sf::Texture* texture = nullptr;
                if (c == '5') texture = &parallaxTile5;
                else if (c == '6') texture = &parallaxTile6;
                
                if (texture != nullptr) {
                    sf::RectangleShape rect(sf::Vector2f(parallaxTileSize, parallaxTileSize));
                    rect.setTexture(texture);
                    rect.setPosition(sf::Vector2f(j * parallaxTileSize, i * parallaxTileSize));
                    parallaxNear.push_back(rect);
                }
            }
        }

        if (lines.empty()) {
            std::cout << "ERROR: Map section is empty!" << std::endl;
            return false;
        }
        
        mapHeight = lines.size();
        mapWidth = 0;
        
        for (const auto& line : lines) {
            if (line.length() > mapWidth) {
                mapWidth = line.length();
            }
        }

        for (int i = 0; i < mapHeight; i++)
        {
            for (int j = 0; j < mapWidth; j++)
            {
                char c = (j < lines[i].length()) ? lines[i][j] : ' ';
                
                if (c == 'w')
                {
                    sf::RectangleShape rect(sf::Vector2f(tileSize, tileSize));
                    rect.setTexture(&wallTexture);
                    rect.setPosition(sf::Vector2f(j * tileSize, i * tileSize));
                    walls.push_back(rect);
                }
                else if (c == 'f')
                {
                    sf::RectangleShape rect(sf::Vector2f(tileSize, tileSize));
                    rect.setTexture(&floorTexture);
                    rect.setPosition(sf::Vector2f(j * tileSize, i * tileSize));
                    floors.push_back(rect);
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

        std::cout << "Level loaded! Map: " << mapWidth << "x" << mapHeight << std::endl;

        return true;
    }
    else
    {
        std::cout << "Unable to open file: " << filePath << std::endl;
        return false;
    }
}

void Level::drawParallax(sf::RenderWindow& window, sf::View& view)
{
    sf::Vector2f cameraCenter = view.getCenter();
    
    for (auto& tile : parallaxFar) {
        sf::Vector2f pos = tile.getPosition();
        pos.x -= cameraCenter.x * (1.f - parallaxFarSpeed);
        pos.y += parallaxFarOffsetY;
        sf::RectangleShape copy = tile;
        copy.setPosition(pos);
        window.draw(copy);
    }
    
    for (auto& tile : parallaxMid) {
        sf::Vector2f pos = tile.getPosition();
        pos.x -= cameraCenter.x * (1.f - parallaxMidSpeed);
        pos.y += parallaxMidOffsetY;
        sf::RectangleShape copy = tile;
        copy.setPosition(pos);
        window.draw(copy);
    }
    
    for (auto& tile : parallaxNear) {
        sf::Vector2f pos = tile.getPosition();
        pos.x -= cameraCenter.x * (1.f - parallaxNearSpeed);
        pos.y += parallaxNearOffsetY;
        sf::RectangleShape copy = tile;
        copy.setPosition(pos);
        window.draw(copy);
    }
}

void Level::draw(sf::RenderWindow& window)
{
    for (auto& wall : walls)
    {
        window.draw(wall);
    }
    for (auto& floor : floors)
    {
        window.draw(floor);
    }
    for (auto& point : points)
    {
        window.draw(point);
    }
}