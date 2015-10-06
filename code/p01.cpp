// Copyright (c) 2014 Vittorio Romeo
// License: MIT License | http://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

// Let's begin the development of our arkanoid clone.

// We'll start by creating a blank window using the SFML library.
// The window will obtain input and display the game graphics.

// The <SFML/Graphics.hpp> module is required to deal with graphics.
// It also includes common STL classes such as `std::vector` and
// the <SFML/Window.hpp> module required for window management.
#include <SFML/Graphics.hpp>

// Let's define some constants for the window.
constexpr unsigned int wndWidth{800}, wndHeight{600};

int main()
{
    // Now we'll create the window.

    // The class is named `sf::RenderWindow`, and the constructor
    // requires an `sf::Vector2u` (size of the window), and an
    // `std::string` (title of the window).
    sf::RenderWindow window{{wndWidth, wndHeight}, "Arkanoid - 1"};

    // Instead of explicitly specifying the `sf::Vector2u` type,
    // we used the {...} uniform initialization syntax.

    // We'll also set a limit to the framerate, ensuring that the
    // game logic will run at a constant speed.
    window.setFramerateLimit(60);

    // The next step is "keeping the window alive".
    // This is where the "game loop" comes into play.
    // {Info: game loop}

    while(true)
    {
        // Every iteration of this loop is a "frame" of our game.
        // We'll begin our frame by clearing the window from previously
        // drawn graphics.
        window.clear(sf::Color::Black);

        // Then we'll check the input state. In this case, if the
        // player presses the "Escape" key, we'll jump outside of the
        // loop, destroying the window and terminating the program.
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) break;

        // Show the window's contents.
        window.display();
    }

    return 0;
}