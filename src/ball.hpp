#ifndef SRC_BALL_HPP_
#define SRC_BALL_HPP_

#include "entity.hpp"
#include "circle.hpp"
#include "SoundEntity.hpp"
#include "BallSound.hpp"
#include <memory>
#define STRINGIZE(x) #x
#define STRINGIZE_VALUE_OF(x) STRINGIZE(x)

constexpr unsigned int wndWidth{800},wndHeight{600};

/**
 * Ball entity which is resonsible for the ball in the game.
 * This class takes care of the circle shape, its properties and its movement
 */

class Ball : public Circle, public Entity
{
public:
	static const sf::Color defColor;
	static constexpr float defRadius{5.f};
	sf::Vector2f velocity;
	std::shared_ptr<SoundEntity> beepSound;

//	bool isLeftCrossed(){ return((x() - shape.getRadius()) < 0);}
//	bool isRightCrossed(){return((x() + shape.getRadius()) > wndWidth);}
//	bool isTopCrossed(){return((y() - shape.getRadius()) < 0);}
//	bool isBottomCrossed(){return((y() + shape.getRadius()) > wndHeight);}

	Ball(float mX,float mY,bool updateStatus,float xvector,float yvector)
	{
		velocity.x = xvector;
		velocity.y = yvector;
		shape.setPosition(mX,mY);
		shape.setRadius(defRadius);
		shape.setFillColor(defColor);
		shape.setOrigin(defRadius,defRadius);
		updateRequired = updateStatus;
		beepSound = std::make_shared<BallSound>(STRINGIZE_VALUE_OF(BEEPSOUNDFILE));
	}

	~Ball(){}

	sf::Vector2f getVelocity(){ return velocity; }

	void setVelocity(float x,float y)
	{
		velocity.x = x;
		velocity.y = y;
	}

	void setPosition(float mX,float mY)
	{
		shape.setPosition(mX,mY);
	}

	virtual void update() override
	{
		solveBoundCollisions();
		shape.move(velocity);
	}

	virtual void draw(sf::RenderWindow& mTarget) override
	{
		mTarget.draw(shape);
	}

	virtual bool checkEntityDied() override
	{
		return(bottom() > wndHeight);
	}

	/**
	 * This function takes care of moving the ball along with paddle when its sitting on the paddle.
	 * So it creates the relative motion between ball and paddle
	 */
	void solveBallPaddleRelativeMotion()
	{
		if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) && left() > 0)
		{
			velocity.x = -8.f;
			velocity.y = 0;
		}
        else if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) && right() < wndWidth)
        {
		    velocity.x = 8.f;
        }
        else
        {
		    velocity.x = 0;
			velocity.y = 0;
        }
		shape.move(velocity);

	}


private:
	// This function keeps the ball inside the window and does not let it go out
	void solveBoundCollisions() noexcept
	{

        if(left() < 0)
        {
    		beepSound->playSound();
            velocity.x = -velocity.x;
        }
        else if(right() > wndWidth)
        {
    		beepSound->playSound();
            velocity.x = -velocity.x;
        }

        if(top() < 0)
        {
            velocity.y = -velocity.y;
    		beepSound->playSound();
        }
        else if(bottom() > wndHeight)
        {
            velocity.y = -velocity.y;
    		beepSound->playSound();
        }

//		if(isLeftCrossed() || isRightCrossed())
//		{
//			velocity.x = -velocity.x;
//		}
//		else if(isTopCrossed() || isBottomCrossed())
//		{
//			velocity.y = -velocity.y;
//		}

	}

};

// Green color of the ball is set fixed as a static variable
const sf::Color Ball::defColor{sf::Color::Green};




#endif /* SRC_BALL_HPP_ */
