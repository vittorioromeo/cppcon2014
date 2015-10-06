// Copyright (c) 2014 Vittorio Romeo
// License: MIT License | http://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

// The ball moves! However, we need to find a way to stop it from
// leaving the window's bounds.

// By testing if the X coordinate of the ball exceeds the window's
// width or is less than 0 we can check if the ball left the window
// horizontally. The same principle applies for the vertical bounds.
// {Info: ball vs window collision}

#include <SFML/Graphics.hpp>

constexpr unsigned int wndWidth{800}, wndHeight{600};

class Ball
{
public:
    static const sf::Color defColor;
    static constexpr float defRadius{10.f};
    static constexpr float defVelocity{8.f};

    sf::CircleShape shape;
    sf::Vector2f velocity{-defVelocity, -defVelocity};

    Ball(float mX, float mY)
    {
        shape.setPosition(mX, mY);
        shape.setRadius(defRadius);
        shape.setFillColor(defColor);
        shape.setOrigin(defRadius, defRadius);
    }

    // We will need to get very often the ball's left/right/top/bottom
    // bounds. Let's define some simple getters to help us.
    float x() const noexcept { return shape.getPosition().x; }
    float y() const noexcept { return shape.getPosition().y; }
    float left() const noexcept { return x() - shape.getRadius(); }
    float right() const noexcept { return x() + shape.getRadius(); }
    float top() const noexcept { return y() - shape.getRadius(); }
    float bottom() const noexcept { return y() + shape.getRadius(); }

    void update()
    {
        // We need to keep the ball "inside the window".
        // The most common (and probably best) way of doing this, and
        // of dealing with any kind of collision detection, is moving
        // the object first, then checking if it's intersecting
        // something.
        // If the test is positive, we simply respond to the collision
        // by altering the object's position and/or velocity.

        // Therefore, we begin by moving the ball.
        shape.move(velocity);

        // After the ball has moved, it may be "outside the window".
        // We need to check every direction and respond by changing
        // the velocity.

        // If it's leaving towards the left, we need to set
        // horizontal velocity to a positive value (towards the right).
        if(left() < 0) velocity.x = defVelocity;

        // Otherwise, if it's leaving towards the right, we need to
        // set horizontal velocity to a negative value (towards the
        // left).
        else if(right() > wndWidth)
            velocity.x = -defVelocity;

        // The same idea can be applied for top/bottom collisions.
        if(top() < 0)
            velocity.y = defVelocity;
        else if(bottom() > wndHeight)
            velocity.y = -defVelocity;
    }

    void draw(sf::RenderWindow& mTarget) { mTarget.draw(shape); }
};

const sf::Color Ball::defColor{sf::Color::Red};

int main()
{
    Ball ball{wndWidth / 2.f, wndHeight / 2.f};

    sf::RenderWindow window{{wndWidth, wndHeight}, "Arkanoid - 3"};
    window.setFramerateLimit(60);

    while(true)
    {
        window.clear(sf::Color::Black);

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) break;

        ball.update();
        ball.draw(window);

        window.display();
    }

    return 0;
}