// Copyright (c) 2014 Vittorio Romeo
// License: MIT License | http://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

// To finish up our game logic we need to check and respond to
// "brick vs ball" collisions.

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

class Brick
{
public:
    static const sf::Color defColor;
    static constexpr float defWidth{60.f};
    static constexpr float defHeight{20.f};
    static constexpr float defVelocity{8.f};

    sf::RectangleShape shape;
    bool destroyed{false};

    Brick(float mX, float mY)
    {
        shape.setPosition(mX, mY);
        shape.setSize({defWidth, defHeight});
        shape.setFillColor(defColor);
        shape.setOrigin(defWidth / 2.f, defHeight / 2.f);
    }

    void update() {}
    void draw(sf::RenderWindow& mTarget) { mTarget.draw(shape); }

    float x() const noexcept { return shape.getPosition().x; }
    float y() const noexcept { return shape.getPosition().y; }
    float width() const noexcept { return shape.getSize().x; }
    float height() const noexcept { return shape.getSize().y; }
    float left() const noexcept { return x() - width() / 2.f; }
    float right() const noexcept { return x() + width() / 2.f; }
    float top() const noexcept { return y() - height() / 2.f; }
    float bottom() const noexcept { return y() + height() / 2.f; }
};

const sf::Color Brick::defColor{sf::Color::Yellow};

template <typename T1, typename T2>
bool isIntersecting(const T1& mA, const T2& mB) noexcept
{
    return mA.right() >= mB.left() && mA.left() <= mB.right() &&
           mA.bottom() >= mB.top() && mA.top() <= mB.bottom();
}

void solvePaddleBallCollision(const Paddle& mPaddle, Ball& mBall) noexcept
{
    if(!isIntersecting(mPaddle, mBall)) return;

    mBall.velocity.y = -Ball::defVelocity;
    mBall.velocity.x =
        mBall.x() < mPaddle.x() ? -Ball::defVelocity : Ball::defVelocity;
}

// Here's the most complex part of our game: brick-ball collision.
// We need to find out from what direction the ball hit the brick,
// and respond accordingly.
void solveBrickBallCollision(Brick& mBrick, Ball& mBall) noexcept
{
    // If there's no intersection, exit the function.
    if(!isIntersecting(mBrick, mBall)) return;

    // Otherwise, the brick has been hit! Mark it.
    mBrick.destroyed = true;

    // Let's calculate how much the ball intersects the brick
    // in every direction.
    // {Info: ball vs brick collision}
    float overlapLeft{mBall.right() - mBrick.left()};
    float overlapRight{mBrick.right() - mBall.left()};
    float overlapTop{mBall.bottom() - mBrick.top()};
    float overlapBottom{mBrick.bottom() - mBall.top()};

    // If the magnitude of the left overlap is smaller than the
    // right one we can safely assume the ball hit the brick
    // from the left.
    bool ballFromLeft(std::abs(overlapLeft) < std::abs(overlapRight));

    // We can apply the same idea for top/bottom collisions.
    bool ballFromTop(std::abs(overlapTop) < std::abs(overlapBottom));

    // Let's store the minimum overlaps for the X and Y axes.
    float minOverlapX{ballFromLeft ? overlapLeft : overlapRight};
    float minOverlapY{ballFromTop ? overlapTop : overlapBottom};

    // If the magnitude of the X overlap is less than the magnitude
    // of the Y overlap, we can safely assume the ball hit the brick
    // horizontally - otherwise, the ball hit the brick vertically.

    // Then, upon our assumptions, we change either the X or Y velocity
    // of the ball, creating a "realistic" response for the collision.
    if(std::abs(minOverlapX) < std::abs(minOverlapY))
    {
        mBall.velocity.x =
            ballFromLeft ? -Ball::defVelocity : Ball::defVelocity;
    }
    else
    {
        mBall.velocity.y = ballFromTop ? -Ball::defVelocity : Ball::defVelocity;
    }
}

int main()
{
    Ball ball{wndWidth / 2.f, wndHeight / 2.f};
    Paddle paddle{wndWidth / 2, wndHeight - 50};
    std::vector<Brick> bricks;

    constexpr int brkCountX{11};
    constexpr int brkCountY{4};
    constexpr int brkStartColumn{1};
    constexpr int brkStartRow{2};
    constexpr float brkSpacing{3.f};
    constexpr float brkOffsetX{22.f};

    for(int iX{0}; iX < brkCountX; ++iX)
        for(int iY{0}; iY < brkCountY; ++iY)
        {
            float x{(iX + brkStartColumn) * (Brick::defWidth + brkSpacing)};
            float y{(iY + brkStartRow) * (Brick::defHeight + brkSpacing)};

            bricks.emplace_back(brkOffsetX + x, y);
        }

    sf::RenderWindow window{{wndWidth, wndHeight}, "Arkanoid - 7"};
    window.setFramerateLimit(60);

    while(true)
    {
        window.clear(sf::Color::Black);

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) break;

        ball.update();
        paddle.update();
        for(auto& brick : bricks)
        {
            brick.update();

            // Let's test collision for every brick.
            solveBrickBallCollision(brick, ball);
        }

        // After testing the collision, it is possible that some bricks
        // are now marked as "destroyed". We need to get rid of all the
        // destroyed bricks.

        // We will use the "erase-remove idiom" to remove all destroyed
        // bricks from the brick vector - using a generic C++14 lambda.

        // `std::remove_if` re-arranges the elements of a container
        // in such a way that elements to be erased are moved towards
        // the end of a vector.

        // By calling `std::vector::erase` with the iterator returned
        // by `std::remove_if` and the end iterator, we remove all the
        // destroyed bricks.
        bricks.erase(std::remove_if(std::begin(bricks), std::end(bricks),
                         [](const auto& mBrick)
                         {
                             return mBrick.destroyed;
                         }),
            std::end(bricks));

        solvePaddleBallCollision(paddle, ball);

        ball.draw(window);
        paddle.draw(window);
        for(auto& brick : bricks) brick.draw(window);

        window.display();
    }

    return 0;
}