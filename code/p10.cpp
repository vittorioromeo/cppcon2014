// Copyright (c) 2014 Vittorio Romeo
// License: MIT License | http://opensource.org/licenses/MIT
// http://vittorioromeo.info | vittorio.romeo@outlook.com

// The next step that will greatly improve our code architecture
// is creating a system that allows us to quickly create new
// game object types during development, and also instantiate and
// destroy game object instances at run-time. We're gonna need
// a polymorphic hierarchy for our objects, and a manager class
// that will help us deal with them.

// Let's include the `<memory>` header, as we're going to use
// smart pointers.
#include <memory>

// We will also need the `<typeinfo>` header  and the `<map>` header
// to query entities by type.
#include <typeinfo>
#include <map>
#include <SFML/Graphics.hpp>

constexpr unsigned int wndWidth{800}, wndHeight{600};

// All game objects share the same interface.
// They provide a `update` and a `draw` method.
// Let's create a polymorphic hierarchy for our objects.
// The base class will be called `Entity`.
// It will also store the `destroyed` bool value, so that
// we can "mark" dead entities whose memory will be reclaimed.
class Entity
{
public:
    bool destroyed{false};

    // We use the `virtual` keyword to enable polymorphism.
    virtual ~Entity() {}
    virtual void update() {}
    virtual void draw(sf::RenderWindow& mTarget) {}
};

// Now, we need a manager class that will help us create,
// destroy, update and query entities.
class Manager
{
private:
    // Since our entities are now polymorphic, we need to store
    // them on the heap. We'll use a `std::vector` of
    // `std::unique_ptr<Entity>` to enable polymorphism.
    std::vector<std::unique_ptr<Entity>> entities;

    // We also need to get all entities of a certain type
    // during the game loop. For example, we need to check
    // if any ball collides with any brick. Instead of manually
    // checking the type of the entity during the loop, we can
    // store a "database" of game objects, an `std::map` of
    // `std::vector` instances where the key is a `typeid` hash.
    std::map<std::size_t, std::vector<Entity*>> groupedEntities;

public:
    // To properly populate/query these data structures, we'll need
    // some methods. We're gonna use C++11 variadic templates to
    // allow the user to create entities with any constructor signature.

    // The first method we're gonna define is `create`: it will
    // take a game object type `T`, and some `TArgs` constructor
    // arguments types as template parameters, and return a reference
    // to an heap-allocated object. The object itself will be stored
    // as an `std::unique_ptr` in the `entities` vector, and a pointer
    // to it will be stored in `groupedEntities` for easy querying.
    // The key that will be used for the `groupedEntities` storage
    // will be the type hash of `T` retrieved thanks to `<typeinfo>`.
    template <typename T, typename... TArgs>
    T& create(TArgs&&... mArgs)
    {
        // Let's make sure, using a `static_assert`, that the type `T`
        // is a child of the `Entity` type inheritance.
        static_assert(std::is_base_of<Entity, T>::value,
            "`T` must be derived from `Entity`");

        // Let's create the object itself, using `std::make_unique`.
        // We'll use perfect forwarding to make sure the types of the
        // arguments passed to `T`'s constructor will be forwarded
        // properly.
        auto uPtr(std::make_unique<T>(std::forward<TArgs>(mArgs)...));
        auto ptr(uPtr.get());

        // Let's retrieve the `T` type hash with the `typeid` keyword.
        // The retrieved hash code is guaranteed to be the same for `T`.
        // Let's use it as the key for the `groupedEntities` entry.
        groupedEntities[typeid(T).hash_code()].emplace_back(ptr);

        // [07/10/2014 addendum]: `hash_code()` does not actually guarantee
        // that the codes generated for two different types will be
        // unique. Learn more about this issue and a possible solution on
        // cppreference:
        // http://en.cppreference.com/w/cpp/types/type_info/hash_code

        // Now let's move the `std::unique_ptr` in the `entities`
        // vector.
        entities.emplace_back(std::move(uPtr));

        return *ptr;
    }

    // Removal of an entity will work in a different way: instead of
    // directly removing the entity from the storage, we will simply
    // mark it as "destroyed". Another method, called `refresh`, will
    // take care of cleaning up all the "destroyed" entities, at the
    // end of an update. This has major performance advantages, and
    // also allows us to correctly access a soon-to-be-destroyed entity
    // without accessing corrupted memory.
    void refresh()
    {
        // This method will take care of cleaning up the destroyed
        // entities. We begin looking for entities to remove in the
        // `groupedEntities` storage, so that their content will
        // still be accessible.
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

        // After that, we use the same idiom on the `entities` vector.
        // Since `entities` stores smart pointers, the memory will be
        // automatically freed when they are removed from the vector.
        entities.erase(std::remove_if(std::begin(entities), std::end(entities),
                           [](const auto& mUPtr)
                           {
                               return mUPtr->destroyed;
                           }),
            std::end(entities));
    }

    // We'll also need a `clear` method to destroy all entities.
    void clear()
    {
        groupedEntities.clear();
        entities.clear();
    }

    // And a template method to query the grouped storage.
    // A good candidate for C++14's automatic function
    // return type deduction.
    template <typename T>
    auto& getAll()
    {
        return groupedEntities[typeid(T).hash_code()];
    }

    // Another useful method will allow the user to execute arbitrary
    // code on all entities of a certain type.
    template <typename T, typename TFunc>
    void forEach(const TFunc& mFunc)
    {
        // Retrieve all entities of type `T`.
        auto& vector(getAll<T>());

        // For each pointer in the entity vector, simply cast the
        // pointer to its "real" type then call the function with the
        // casted pointer, dereferenced.
        for(auto ptr : vector) mFunc(*reinterpret_cast<T*>(ptr));
    }

    // Lastly, we'll implement a method to update all entities, and a
    // method to draw all entities.

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

// Let's now adapt our classes to the new architecture.

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

    // The `override` C++11 keyword is incredibly useful.
    // It makes sure that you're overriding a virtual method
    // of the base class.
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
            velocity.y = -defVelocity;
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
    static const sf::Color defColor;
    static constexpr float defWidth{60.f}, defHeight{20.f};
    static constexpr float defVelocity{8.f};

    Brick(float mX, float mY)
    {
        shape.setPosition(mX, mY);
        shape.setSize({defWidth, defHeight});
        shape.setFillColor(defColor);
        shape.setOrigin(defWidth / 2.f, defHeight / 2.f);
    }

    void draw(sf::RenderWindow& mTarget) override { mTarget.draw(shape); }
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

class Game
{
private:
    enum class State
    {
        Paused,
        InProgress
    };

    static constexpr int brkCountX{11}, brkCountY{4};
    static constexpr int brkStartColumn{1}, brkStartRow{2};
    static constexpr float brkSpacing{3.f}, brkOffsetX{22.f};

    sf::RenderWindow window{{wndWidth, wndHeight}, "Arkanoid - 10"};

    // The `Game` class will now store our manager.
    Manager manager;

    State state{State::InProgress};
    bool pausePressedLastFrame{false};

public:
    Game() { window.setFramerateLimit(60); }

    void restart()
    {
        // Restarting will clear the manager and re-create all entities.
        state = State::Paused;
        manager.clear();

        for(int iX{0}; iX < brkCountX; ++iX)
            for(int iY{0}; iY < brkCountY; ++iY)
            {
                float x{(iX + brkStartColumn) * (Brick::defWidth + brkSpacing)};
                float y{(iY + brkStartRow) * (Brick::defHeight + brkSpacing)};

                // As you can see, creating entities using the manager is
                // really straightforward.
                manager.create<Brick>(brkOffsetX + x, y);
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

            if(state != State::Paused)
            {
                // Instead of manually updating every entity, we just
                // ask the manager to do the work for us.
                manager.update();

                // The game logic is now much more generic: we ask the
                // manager to give us all instances of a certain game
                // object type, then we run the collision functions.
                // This is very flexible as we can have any number
                // of balls and bricks, and adding new types of game
                // objects is extremely easy.
                manager.forEach<Ball>([this](auto& mBall)
                    {
                        manager.forEach<Brick>([this, &mBall](auto& mBrick)
                            {
                                solveBrickBallCollision(mBrick, mBall);
                            });
                        manager.forEach<Paddle>([this, &mBall](auto& mPaddle)
                            {
                                solvePaddleBallCollision(mPaddle, mBall);
                            });
                    });

                // Now we ask the manager to clean-up the destroyed
                // entities.
                manager.refresh();
            }

            manager.draw(window);
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

// The code is now much more complex, but the advantages the programmer
// gets from this kind of design are significant.
// Using a manager to deal with entities greatly simplifies the addition
// of new game object types, and also allows the developer to pay less
// attention to memory-management.

// In the next (and last) code segment, we'll add some completely optional
// finishing touches to our simple arkanoid clone:
// * Text
// * Win/lose states (limited lives)
// * Multi-hit bricks