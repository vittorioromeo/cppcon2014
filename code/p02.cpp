// Copyright (c) 2014 Vittorio Romeo
// License: MIT License | http://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

// In this code segment we'll start implementing the first
// game object: the ball. We will create a class for it and
// learn how to use SFML shapes to display and move the ball.

#include <SFML/Graphics.hpp>

constexpr unsigned int wndWidth{800}, wndHeight{600};

// We'll define a class representing the ball entity.
// The class will deal both with its logic and its rendering.
class Ball
{
public:
    // Let's define some constants for the default values.
    static const sf::Color defColor;
    static constexpr float defRadius{10.f};
    static constexpr float defVelocity{1.f};

    // `sf::CircleShape` is an SFML class that represents
    // a circular shape. By specifying a radius and a position,
    // it is possible to draw a circle on a window.
    sf::CircleShape shape;

    // We'll need a vector to store the current ball velocity.
    // It will be initialized with the default velocity values.
    sf::Vector2f velocity{-defVelocity, -defVelocity};

    // Ball constructor: it takes the starting position of
    // ball as two float arguments.
    Ball(float mX, float mY)
    {
        // SFML uses a coordinate system with the origin in
        // the top-left corner of the window.
        // {Info: coordinate system}

        shape.setPosition(mX, mY);
        shape.setRadius(defRadius);
        shape.setFillColor(defColor);
        shape.setOrigin(defRadius, defRadius);
    }

    // In our design every "game object" will have an `update` method
    // and a `draw` method.

    // The `update` method will update the game object's logic.

    // The `draw` method will draw the game object on the screen.
    // It will take a reference to a `sf::RenderWindow` as a parameter
    // that will be our drawing target.

    void update()
    {
        // SFML's shape classes have a `move` method that takes
        // a velocity float vector as a parameter.
        // {Info: ball movement}
        shape.move(velocity);
    }

    void draw(sf::RenderWindow& mTarget)
    {
        // In the ball's draw method we simply ask the window to
        // draw the shape for us.
        mTarget.draw(shape);
    }
};

// Static data members must be initialized outside of the class.
const sf::Color Ball::defColor{sf::Color::Red};

int main()
{
    // Let's create an instance of `Ball`, positioned in the center.
    Ball ball{wndWidth / 2.f, wndHeight / 2.f};

    sf::RenderWindow window{{wndWidth, wndHeight}, "Arkanoid - 2"};
    window.setFramerateLimit(60);

    while(true)
    {
        window.clear(sf::Color::Black);

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) break;

        // In the game loop, we need to update and draw all game objects.
        ball.update();
        ball.draw(window);

        window.display();
    }

    return 0;
}