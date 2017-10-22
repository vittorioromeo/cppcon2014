#include "game.hpp"
#include <X11/Xlib.h>

/**
 * main function
 */
int main()
{
	XInitThreads();
	Game game;
	game.restart();
	game.run();

	return 0;
}
