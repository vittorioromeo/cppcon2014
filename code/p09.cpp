// Copyright (c) 2014 Vittorio Romeo
// License: MIT License | http://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

// In this code segment we'll start improving the game's code
// architecture. The first step will be creating a `Game` class
// that will encapsulate the game's state.

#include <SFML/Graphics.hpp>

constexpr unsigned int wndWidth{800}, wndHeight{600};

struct Rectangle
{
    sf::RectangleShape shape;

    float x() const noexcept { return shape.getPosition().x; }
    float y() const noexcept { return shape.getPosition().y; }
    float width() const noexcept { return shape.getSize().x; }
    float height() const noexcept { return shape.getSize().y; }
    float left() const noexcept { return x() - width() / 2.f; }
    float right() const noexcept { return x() + width() / 2.f; }
    float top() const noexcept { return y() - height() / 2.f; }
    float bottom() const noexcept { return y() + height() / 2.f; }
};

struct Circle
{
    sf::CircleShape shape;

    float x() const noexcept { return shape.getPosition().x; }
    float y() const noexcept { return shape.getPosition().y; }
    float radius() const noexcept { return shape.getRadius(); }
    float left() const noexcept { return x() - radius(); }
    float right() const noexcept { return x() + radius(); }
    float top() const noexcept { return y() - radius(); }
    float bottom() const noexcept { return y() + radius(); }
};

class Ball : public Circle
{
public:
    static const sf::Color defColor;
    static constexpr float defRadius{10.f};
    static constexpr float defVelocity{8.f};

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

class Paddle : public Rectangle
{
public:
    static const sf::Color defColor;
    static constexpr float defWidth{60.f};
    static constexpr float defHeight{20.f};
    static constexpr float defVelocity{8.f};

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

class Brick : public Rectangle
{
public:
    static const sf::Color defColor;
    static constexpr float defWidth{60.f};
    static constexpr float defHeight{20.f};
    static constexpr float defVelocity{8.f};

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

void solveBrickBallCollision(Brick& mBrick, Ball& mBall) noexcept
{
    if(!isIntersecting(mBrick, mBall)) return;
    mBrick.destroyed = true;

    float overlapLeft{mBall.right() - mBrick.left()};
    float overlapRight{mBrick.right() - mBall.left()};
    float overlapTop{mBall.bottom() - mBrick.top()};
    float overlapBottom{mBrick.bottom() - mBall.top()};

    bool ballFromLeft(std::abs(overlapLeft) < std::abs(overlapRight));
    bool ballFromTop(std::abs(overlapTop) < std::abs(overlapBottom));

    float minOverlapX{ballFromLeft ? overlapLeft : overlapRight};
    float minOverlapY{ballFromTop ? overlapTop : overlapBottom};

    if(std::abs(minOverlapX) < std::abs(minOverlapY))
        mBall.velocity.x =
            ballFromLeft ? -Ball::defVelocity : Ball::defVelocity;
    else
        mBall.velocity.y = ballFromTop ? -Ball::defVelocity : Ball::defVelocity;
}

// The `Game` class will store game constants and elements, and
// will keep track of the game's state. Also, it will provide
// functionality to pause and restart the game.
class Game
{
private:
    // Let's create an enum for the possible game states.
    enum class State
    {
        Paused,
        InProgress
    };

    static constexpr int brkCountX{11}, brkCountY{4};
    static constexpr int brkStartColumn{1}, brkStartRow{2};
    static constexpr float brkSpacing{3.f}, brkOffsetX{22.f};

    sf::RenderWindow window{{wndWidth, wndHeight}, "Arkanoid - 9"};

    Ball ball{wndWidth / 2.f, wndHeight / 2.f};
    Paddle paddle{wndWidth / 2, wndHeight - 50};
    std::vector<Brick> bricks;

    // Let's add some fields to keep track of the game status.
    State state{State::InProgress};
    bool pausePressedLastFrame{false};

public:
    Game() { window.setFramerateLimit(60); }

    // The `restart` method will re-create all game objects and restore
    // the game to a default state.
    void restart()
    {
        state = State::Paused;

        for(int iX{0}; iX < brkCountX; ++iX)
            for(int iY{0}; iY < brkCountY; ++iY)
            {
                float x{(iX + brkStartColumn) * (Brick::defWidth + brkSpacing)};
                float y{(iY + brkStartRow) * (Brick::defHeight + brkSpacing)};

                bricks.emplace_back(brkOffsetX + x, y);
            }

        ball = Ball{wndWidth / 2.f, wndHeight / 2.f};
        paddle = Paddle{wndWidth / 2, wndHeight - 50};
    }

    // The `run` method will start the game loop.
    void run()
    {
        while(true)
        {
            window.clear(sf::Color::Black);

            if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) break;

            // The `P` key will toggle the pause. To prevent continuous
            // use of the pause button, we need to check if the input
            // was pressed last frame.
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::P))
            {
                // If `P` was not pressed last frame, we can toggle
                // the state.
                if(!pausePressedLastFrame)
                {
                    if(state == State::Paused)
                        state = State::InProgress;
                    else if(state == State::InProgress)
                        state = State::Paused;
                }

                pausePressedLastFrame = true;
            }
            else
                pausePressedLastFrame = false;

            // Let's also use the `R` key to restart the game.
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R)) restart();

            // If the game is paused, we'll only draw game elements,
            // without updating them.
            if(state != State::Paused)
            {
                // The rest of the game loop code is exactly the same.

                ball.update();
                paddle.update();
                for(auto& brick : bricks)
                {
                    brick.update();
                    solveBrickBallCollision(brick, ball);
                }

                bricks.erase(
                    std::remove_if(std::begin(bricks), std::end(bricks),
                        [](const auto& mBrick)
                        {
                            return mBrick.destroyed;
                        }),
                    std::end(bricks));

                solvePaddleBallCollision(paddle, ball);
            }

            ball.draw(window);
            paddle.draw(window);
            for(auto& brick : bricks) brick.draw(window);

            window.display();
        }
    }
};

int main()
{
    // To start our game, we create a `Game` instance,
    // restart it and run it.
    Game game;
    game.restart();
    game.run();
    return 0;
}