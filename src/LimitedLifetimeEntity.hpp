#ifndef SRC_LIMITEDLIFETIMEENTITY_HPP_
#define SRC_LIMITEDLIFETIMEENTITY_HPP_

#include "entity.hpp"

class LimitedLifetimeEntity
{
public:
	bool destroyed{false};
	virtual bool checkEntityDied()=0;
};



#endif /* SRC_LIMITEDLIFETIMEENTITY_HPP_ */
