// Copyright (c) 2014 Vittorio Romeo
// License: MIT License | http://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

// We're still missing two important elements: the paddle
// and the bricks. We'll start by implementing a player
// controlled paddle in this code segment.

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

    void update()
    {
        shape.move(velocity);
        solveBoundCollisions();
    }

    void draw(sf::RenderWindow& mTarget) { mTarget.draw(shape); }

    float x() const noexcept { return shape.getPosition().x; }
    float y() const noexcept { return shape.getPosition().y; }
    float left() const noexcept { return x() - shape.getRadius(); }
    float right() const noexcept { return x() + shape.getRadius(); }
    float top() const noexcept { return y() - shape.getRadius(); }
    float bottom() const noexcept { return y() + shape.getRadius(); }

private:
    // Some minor refactoring always helps readability.
    void solveBoundCollisions() noexcept
    {
        if(left() < 0)
            velocity.x = defVelocity;
        else if(right() > wndWidth)
            velocity.x = -defVelocity;

        if(top() < 0)
            velocity.y = defVelocity;
        else if(bottom() > wndHeight)
            velocity.y = -defVelocity;
    }
};

const sf::Color Ball::defColor{sf::Color::Red};

// Like the ball, the `Paddle` class will represent a game object,
// with its own `update` and `draw` methods.
class Paddle
{
public:
    static const sf::Color defColor;
    static constexpr float defWidth{60.f};
    static constexpr float defHeight{20.f};
    static constexpr float defVelocity{8.f};

    // This time we'll use a `sf::RectangleShape`.
    sf::RectangleShape shape;
    sf::Vector2f velocity;

    // As with the ball, we construct the paddle with
    // arguments for the initial position and initialize
    // the SFML rectangle shape.
    Paddle(float mX, float mY)
    {
        shape.setPosition(mX, mY);
        shape.setSize({defWidth, defHeight});
        shape.setFillColor(defColor);
        shape.setOrigin(defWidth / 2.f, defHeight / 2.f);
    }

    void update()
    {
        // Before moving the paddle, we'll process player input,
        // changing the paddle's velocity.
        processPlayerInput();
        shape.move(velocity);
    }

    void draw(sf::RenderWindow& mTarget) { mTarget.draw(shape); }

    float x() const noexcept { return shape.getPosition().x; }
    float y() const noexcept { return shape.getPosition().y; }
    float width() const noexcept { return shape.getSize().x; }
    float height() const noexcept { return shape.getSize().y; }
    float left() const noexcept { return x() - width() / 2.f; }
    float right() const noexcept { return x() + width() / 2.f; }
    float top() const noexcept { return y() - height() / 2.f; }
    float bottom() const noexcept { return y() + height() / 2.f; }

private:
    void processPlayerInput()
    {
        // We will change the paddle's velocity depending on what
        // the user is currently pressing on its keyboard:
        // * If the left arrow key is being pressed, we set X velocity
        //   to a negative value.
        // * If the right arrow key is being pressed, we set X velocity
        //   to a positive value.
        // * If no arrow keys are being pressed, we set X velocity to
        //   zero.

        // To avoid making having the paddle go "outside the window",
        // we will only apply the above velocity changes if the paddle
        // is inside the window.

        // So, if the user is trying to move the paddle towards the
        // right, but the paddle has already "escaped" the window
        // in that direction, we won't change the velocity.

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) && left() > 0)
        {
            velocity.x = -defVelocity;
        }
        else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) &&
                right() < wndWidth)
        {
            velocity.x = defVelocity;
        }
        else
        {
            velocity.x = 0;
        }
    }
};

const sf::Color Paddle::defColor{sf::Color::Red};

int main()
{
    Ball ball{wndWidth / 2.f, wndHeight / 2.f};

    // Let's create a `Paddle` instance.
    Paddle paddle{wndWidth / 2, wndHeight - 50};

    sf::RenderWindow window{{wndWidth, wndHeight}, "Arkanoid - 4"};
    window.setFramerateLimit(60);

    while(true)
    {
        window.clear(sf::Color::Black);

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) break;

        // And let's update and draw it.

        ball.update();
        paddle.update();

        ball.draw(window);
        paddle.draw(window);

        window.display();
    }

    return 0;
}