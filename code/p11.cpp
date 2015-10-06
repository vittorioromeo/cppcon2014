// Copyright (c) 2014 Vittorio Romeo
// License: MIT License | http://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

// In this last code segment, we'll add some features to our game
// and finish it:
// * Text
// * Win/lose states (limited lives)
// * Multi-hit bricks

#include <memory>
#include <typeinfo>
#include <map>
#include <SFML/Graphics.hpp>

constexpr unsigned int wndWidth{800}, wndHeight{600};

class Entity
{
public:
    bool destroyed{false};

    virtual ~Entity() {}
    virtual void update() {}
    virtual void draw(sf::RenderWindow& mTarget) {}
};

class Manager
{
private:
    std::vector<std::unique_ptr<Entity>> entities;
    std::map<std::size_t, std::vector<Entity*>> groupedEntities;

public:
    template <typename T, typename... TArgs>
    T& create(TArgs&&... mArgs)
    {
        static_assert(std::is_base_of<Entity, T>::value,
            "`T` must be derived from `Entity`");

        auto uPtr(std::make_unique<T>(std::forward<TArgs>(mArgs)...));
        auto ptr(uPtr.get());
        groupedEntities[typeid(T).hash_code()].emplace_back(ptr);
        entities.emplace_back(std::move(uPtr));

        return *ptr;
    }

    void refresh()
    {
        for(auto& pair : groupedEntities)
        {
            auto& vector(pair.second);

            vector.erase(std::remove_if(std::begin(vector), std::end(vector),
                             [](auto mPtr)
                             {
                                 return mPtr->destroyed;
                             }),
                std::end(vector));
        }

        entities.erase(std::remove_if(std::begin(entities), std::end(entities),
                           [](const auto& mUPtr)
                           {
                               return mUPtr->destroyed;
                           }),
            std::end(entities));
    }

    void clear()
    {
        groupedEntities.clear();
        entities.clear();
    }

    template <typename T>
    auto& getAll()
    {
        return groupedEntities[typeid(T).hash_code()];
    }

    template <typename T, typename TFunc>
    void forEach(const TFunc& mFunc)
    {
        for(auto ptr : getAll<T>()) mFunc(*reinterpret_cast<T*>(ptr));
    }

    void update()
    {
        for(auto& e : entities) e->update();
    }
    void draw(sf::RenderWindow& mTarget)
    {
        for(auto& e : entities) e->draw(mTarget);
    }
};

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

class Ball : public Entity, public Circle
{
public:
    static const sf::Color defColor;
    static constexpr float defRadius{10.f}, defVelocity{8.f};

    sf::Vector2f velocity{-defVelocity, -defVelocity};

    Ball(float mX, float mY)
    {
        shape.setPosition(mX, mY);
        shape.setRadius(defRadius);
        shape.setFillColor(defColor);
        shape.setOrigin(defRadius, defRadius);
    }

    void update() override
    {
        shape.move(velocity);
        solveBoundCollisions();
    }

    void draw(sf::RenderWindow& mTarget) override { mTarget.draw(shape); }

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
        {
            // If the ball leaves the window towards the bottom,
            // we destroy it.
            destroyed = true;
        }
    }
};

const sf::Color Ball::defColor{sf::Color::Red};

class Paddle : public Entity, public Rectangle
{
public:
    static const sf::Color defColor;
    static constexpr float defWidth{60.f}, defHeight{20.f};
    static constexpr float defVelocity{8.f};

    sf::Vector2f velocity;

    Paddle(float mX, float mY)
    {
        shape.setPosition(mX, mY);
        shape.setSize({defWidth, defHeight});
        shape.setFillColor(defColor);
        shape.setOrigin(defWidth / 2.f, defHeight / 2.f);
    }

    void update() override
    {
        processPlayerInput();
        shape.move(velocity);
    }

    void draw(sf::RenderWindow& mTarget) override { mTarget.draw(shape); }

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

class Brick : public Entity, public Rectangle
{
public:
    static const sf::Color defColorHits1;
    static const sf::Color defColorHits2;
    static const sf::Color defColorHits3;
    static constexpr float defWidth{60.f}, defHeight{20.f};
    static constexpr float defVelocity{8.f};

    // Let's add a field for the required hits.
    int requiredHits{1};

    Brick(float mX, float mY)
    {
        shape.setPosition(mX, mY);
        shape.setSize({defWidth, defHeight});
        shape.setOrigin(defWidth / 2.f, defHeight / 2.f);
    }

    void update() override
    {
        // Let's alter the color of the brick depending on the
        // required hits.
        if(requiredHits == 1)
            shape.setFillColor(defColorHits1);
        else if(requiredHits == 2)
            shape.setFillColor(defColorHits2);
        else
            shape.setFillColor(defColorHits3);
    }
    void draw(sf::RenderWindow& mTarget) override { mTarget.draw(shape); }
};

const sf::Color Brick::defColorHits1{255, 255, 0, 80};
const sf::Color Brick::defColorHits2{255, 255, 0, 170};
const sf::Color Brick::defColorHits3{255, 255, 0, 255};

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

    // Instead of immediately destroying the brick upon collision,
    // we decrease and check its required hits first.
    --mBrick.requiredHits;
    if(mBrick.requiredHits <= 0) mBrick.destroyed = true;

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

class Game
{
private:
    // There now are two additional game states: `GameOver`
    // and `Victory`.
    enum class State
    {
        Paused,
        GameOver,
        InProgress,
        Victory
    };

    static constexpr int brkCountX{11}, brkCountY{4};
    static constexpr int brkStartColumn{1}, brkStartRow{2};
    static constexpr float brkSpacing{3.f}, brkOffsetX{22.f};

    sf::RenderWindow window{{wndWidth, wndHeight}, "Arkanoid - 11"};
    Manager manager;

    // SFML offers an easy-to-use font and text class that we
    // can use to display remaining lives and game status.
    sf::Font liberationSans;
    sf::Text textState, textLives;

    State state{State::GameOver};
    bool pausePressedLastFrame{false};

    // Let's keep track of the remaning lives in the game class.
    int remainingLives{0};

public:
    Game()
    {
        window.setFramerateLimit(60);

        // We need to load a font from file before using
        // our text objects.
        liberationSans.loadFromFile(
            R"(/usr/share/fonts/TTF/LiberationSans-Regular.ttf)");

        textState.setFont(liberationSans);
        textState.setPosition(10, 10);
        textState.setCharacterSize(35.f);
        textState.setColor(sf::Color::White);
        textState.setString("Paused");

        textLives.setFont(liberationSans);
        textLives.setPosition(10, 10);
        textLives.setCharacterSize(15.f);
        textLives.setColor(sf::Color::White);
    }

    void restart()
    {
        // Let's remember to reset the remaining lives.
        remainingLives = 3;

        state = State::Paused;
        manager.clear();

        for(int iX{0}; iX < brkCountX; ++iX)
            for(int iY{0}; iY < brkCountY; ++iY)
            {
                float x{(iX + brkStartColumn) * (Brick::defWidth + brkSpacing)};
                float y{(iY + brkStartRow) * (Brick::defHeight + brkSpacing)};

                auto& brick(manager.create<Brick>(brkOffsetX + x, y));

                // Let's set the required hits for the bricks.
                brick.requiredHits = 1 + ((iX * iY) % 3);
            }

        manager.create<Ball>(wndWidth / 2.f, wndHeight / 2.f);
        manager.create<Paddle>(wndWidth / 2, wndHeight - 50);
    }

    void run()
    {
        while(true)
        {
            window.clear(sf::Color::Black);

            if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) break;

            if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::P))
            {
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

            if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R)) restart();

            // If the game is not in progress, do not draw or update
            // game elements and display information to the player.
            if(state != State::InProgress)
            {
                if(state == State::Paused)
                    textState.setString("Paused");
                else if(state == State::GameOver)
                    textState.setString("Game over!");
                else if(state == State::Victory)
                    textState.setString("You won!");

                window.draw(textState);
            }
            else
            {
                // If there are no more balls on the screen, spawn a
                // new one and remove a life.
                if(manager.getAll<Ball>().empty())
                {
                    manager.create<Ball>(wndWidth / 2.f, wndHeight / 2.f);

                    --remainingLives;
                }

                // If there are no more bricks on the screen,
                // the player won!
                if(manager.getAll<Brick>().empty()) state = State::Victory;

                // If the player has no more remaining lives,
                // it's game over!
                if(remainingLives <= 0) state = State::GameOver;

                manager.update();

                manager.forEach<Ball>([this](auto& mBall)
                    {
                        manager.forEach<Brick>([&mBall](auto& mBrick)
                            {
                                solveBrickBallCollision(mBrick, mBall);
                            });
                        manager.forEach<Paddle>([&mBall](auto& mPaddle)
                            {
                                solvePaddleBallCollision(mPaddle, mBall);
                            });
                    });

                manager.refresh();

                manager.draw(window);

                // Update lives string and draw it.
                textLives.setString("Lives: " + std::to_string(remainingLives));

                window.draw(textLives);
            }

            window.display();
        }
    }
};

int main()
{
    Game game;
    game.restart();
    game.run();
    return 0;
}