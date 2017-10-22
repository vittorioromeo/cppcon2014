#ifndef SRC_RECTANGLE_HPP_
#define SRC_RECTANGLE_HPP_

/**
 * The struct which can be used as a base to all entities with rectangle shape
 */

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




#endif /* SRC_RECTANGLE_HPP_ */
