#ifndef SRC_BULLET_HPP_
#define SRC_BULLET_HPP_

#include "entity.hpp"
#include "circle.hpp"

/**
 * This class takes care of the bullet entity. This class just creates the bullets and moves them upwards
 */
class Bullet : public Entity, public Circle
{
	static constexpr float defRadius{5.f};
	static constexpr float defVelocity{10.f};
	sf::Vector2f velocity{0.f,-defVelocity};
public:
	bool isStruck{false};
	Bullet(float mX,float mY,bool updateStatus)
	{
		shape.setPosition(mX,mY);
		shape.setRadius(defRadius);
		shape.setFillColor(sf::Color::Red);
		shape.setOrigin(defRadius,defRadius);
		updateRequired = updateStatus;
	}
	~Bullet(){}
	virtual void update() override{shape.move(velocity);}
	// When the bullet is outside the window then its dead entity
	virtual bool checkEntityDied(){return ( shape.getPosition().y < 0 || isStruck);}
	virtual void draw(sf::RenderWindow& mTarget) override {mTarget.draw(shape);}
};
#endif /* SRC_BULLET_HPP_ */
