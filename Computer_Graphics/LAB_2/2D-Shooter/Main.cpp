#include <iostream>
#include "Game.h"

int main() {

	Game game(800, 800);
	bool success = game.Initialize();

	if(success) {
		game.RunLoop();
	}

	game.Shutdown();

	return 0;
}