#ifndef SRC_UTILITY_HPP_
#define SRC_UTILITY_HPP_

/**
 * Utility functions which are used primarily by the game class to handle the different kinds of collision
 * ball and brick, ball and paddle
 */
template<typename T1,typename T2>
bool isIntersecting(const T1& mA,const T2& mB)
{
	return ((mA.right() >= mB.left())
			&& (mA.left() <= mB.right())
			&& (mA.bottom() >= mB.top())
			&& (mA.top() <= mB.bottom()));
}

void solvePaddleBallCollision(const Paddle& mpaddle, Ball& mball) noexcept
{
	if(!isIntersecting(mpaddle,mball)) return;
	mball.velocity.y = -abs(mball.velocity.y);
	mball.velocity.x = (mball.x() < mpaddle.x()) ? (-abs(mball.velocity.x)) : (mball.velocity.x);
}

void solveBrickBulletCollision(Brick& mbrick,Bullet& mbullet) noexcept
{
	if(!isIntersecting(mbrick,mbullet))return;
	--mbrick.hitsRequired;
	if(!mbrick.hitsRequired)
	{
		mbrick.flingBrick();
	}
	mbullet.destroyed = true;
}

void solveBallBrickCollision(Brick& mbrick, Ball& mball) noexcept
{
	if(!mbrick.isFlying())
	{
		if(!isIntersecting(mbrick,mball)) return;
		--mbrick.hitsRequired;
		mball.beepSound->playSound();
		if(!mbrick.hitsRequired)
		{
			mbrick.flingBrick();
		}

		float overlapLeft{mball.right()-mbrick.left()};
		float overlapRight{mbrick.right()-mball.left()};
		float overlapTop{mball.bottom()-mbrick.top()};
		float overlapBottom{mbrick.bottom()-mball.top()};

		bool ballFromLeft{std::abs(overlapLeft) < std::abs(overlapRight)};
		bool ballFromTop{std::abs(overlapTop) < std::abs(overlapBottom)};

		float minOverlapX{ballFromLeft ? overlapLeft : overlapRight};
		float minOverlapY{ballFromTop ? overlapTop : overlapBottom};

		if(std::abs(minOverlapX) < std::abs(minOverlapY))
		{
			mball.velocity.x = ballFromLeft ? (mball.velocity.x) : -mball.velocity.x;
		}
		else
		{
			mball.velocity.y = ballFromTop ? (mball.velocity.y) : -mball.velocity.y;
		}
	}
}

#endif /* SRC_UTILITY_HPP_ */
