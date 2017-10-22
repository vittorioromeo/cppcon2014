#ifndef SRC_CIRCLE_HPP_
#define SRC_CIRCLE_HPP_

/**
 * Base struct which can be inherited by those who have circular shape
 */
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





#endif /* SRC_CIRCLE_HPP_ */
