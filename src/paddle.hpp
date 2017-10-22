#ifndef SRC_PADDLE_HPP_
#define SRC_PADDLE_HPP_

#include "entity.hpp"
#include "rectangle.hpp"

/**
 * This class represents the paddle entity in the game. It takes care of its display and also its movement
 */
class Paddle : public Rectangle, public Entity
{
public:
	static const sf::Color defColor;
	static constexpr float defheight{10.f};
	static constexpr float defwidth{100.f};
	static constexpr float defVelocity{8.f};

	sf::Vector2f velocity;
	sf::Vector2f lastPosition;

public:
	Paddle(float mX,float mY,bool updateStatus)
	{
		shape.setPosition(mX,mY);
		lastPosition = {mX,mY};
		shape.setSize({defwidth,defheight});
		shape.setFillColor(defColor);
		shape.setOrigin(defwidth/2.f,defheight/2.f);
		updateRequired = updateStatus;
	}

	~Paddle(){}

	virtual void update() override
	{
		processPlayerInputs();
		shape.move(velocity);
		std::cout<<shape.getPosition().x<<"  "<<shape.getPosition().y<<std::endl;
	}

	void movePaddlePosition(float mX,float mY)
	{
		//sf::Vector2f vec = shape.getPosition();

//		shape.setPosition(mX,mY);

		if(lastPosition.x != mX)
		{
			if(lastPosition.x < mX)
			{
				sf::Vector2f vel{1,0};
				while(abs(shape.getPosition().x-mX) != 0)
				{
					std::this_thread::sleep_for (std::chrono::microseconds(400));
					shape.move(vel);
				}
				lastPosition = {mX,mY};
			}else{
				sf::Vector2f vel{-1,0};
				while(abs(shape.getPosition().x-mX) != 0)
				{
					std::this_thread::sleep_for (std::chrono::microseconds(400));
					shape.move(vel);
				}
				lastPosition = {mX,mY};
			}
		}
	}

	virtual void draw(sf::RenderWindow& window) override {window.draw(shape);}

	virtual bool checkEntityDied() override
	{
		return false;
	}

private:
	void processPlayerInputs()
	{
		if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) && left() > 0)
			velocity.x = -defVelocity;
        else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) && right() < wndWidth)
		    velocity.x = defVelocity;
        else
		    velocity.x = 0;
	}
};

const sf::Color Paddle::defColor{sf::Color::White};





#endif /* SRC_PADDLE_HPP_ */
