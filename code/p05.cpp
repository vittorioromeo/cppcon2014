// Copyright (c) 2014 Vittorio Romeo
// License: MIT License | http://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

// In this code segment we'll deal with the interactions
// between the ball and the paddle. We'll need to check
// eventual collisions and respond to them.

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

class Paddle
{
public:
    static const sf::Color defColor;
    static constexpr float defWidth{60.f};
    static constexpr float defHeight{20.f};
    static constexpr float defVelocity{8.f};

    sf::RectangleShape shape;
    sf::Vector2f velocity;

    Paddle(float mX, float mY)
    {
        shape.setPosition(mX, mY);
        shape.setSize({defWidth, defHeight});
        shape.setFillColor(defColor);
        shape.setOrigin(defWidth / 2.f, defHeight / 2.f);
    }

    void update()
    {
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
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) && left() > 0)
            velocity.x = -defVelocity;
        else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) &&
                right() < wndWidth)
            velocity.x = defVelocity;
        else
            velocity.x = 0;
    }
};

const sf::Color Paddle::defColor{sf::Color::Red};

// Let's begin by defining a generic function template that
// detects when two game objects collide.
// The requirement for the types `T1` and `T2` is having the
// `left()`, `right()`, `top()`, `bottom()` members.
// Therefore, we can use this template function both on our `Ball`
// and our `Paddle` class.
template <typename T1, typename T2>
bool isIntersecting(const T1& mA, const T2& mB) noexcept
{
    // {Info: AABB vs AABB collision}
    return mA.right() >= mB.left() && mA.left() <= mB.right() &&
           mA.bottom() >= mB.top() && mA.top() <= mB.bottom();
}

// Now, let's also define a function that will executed every game
// frame. This function will check if a paddle and a ball are
// colliding, and if they are it will resolve the collision by
// making the ball go upwards and in the direction opposite to the
// collision.
void solvePaddleBallCollision(const Paddle& mPaddle, Ball& mBall) noexcept
{
    // If there's no intersection, exit the function.
    if(!isIntersecting(mPaddle, mBall)) return;

    // Otherwise let's "push" the ball upwards.
    mBall.velocity.y = -Ball::defVelocity;

    // And let's direct it dependently on the position where the
    // paddle was hit.

    // If the ball's center was to the left of the paddle's center,
    // the ball will move towards the left. Otherwise, it will move
    // towards the right.
    // {Info: ball vs paddle collision}
    mBall.velocity.x =
        mBall.x() < mPaddle.x() ? -Ball::defVelocity : Ball::defVelocity;
}

int main()
{
    Ball ball{wndWidth / 2.f, wndHeight / 2.f};
    Paddle paddle{wndWidth / 2, wndHeight - 50};

    sf::RenderWindow window{{wndWidth, wndHeight}, "Arkanoid - 5"};
    window.setFramerateLimit(60);

    while(true)
    {
        window.clear(sf::Color::Black);

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) break;

        ball.update();
        paddle.update();

        // After updating the ball and the paddle, let's check and
        // resolve eventual collisions.
        solvePaddleBallCollision(paddle, ball);

        ball.draw(window);
        paddle.draw(window);

        window.display();
    }

    return 0;
}