#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <SFML/Graphics.hpp>


class Game {

public:
	int windowWidth;
	int windowHeight;

	Game(int windowWidth, int windowHeight);
	~Game();
};