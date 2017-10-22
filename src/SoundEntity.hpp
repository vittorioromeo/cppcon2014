/*
 * SoundEntity.hpp
 *
 *  Created on: Oct 4, 2017
 *      Author: ishan
 */

#ifndef SRC_SOUNDENTITY_HPP_
#define SRC_SOUNDENTITY_HPP_

#include <string>

class SoundEntity
{
protected:
	std::shared_ptr<sf::SoundBuffer> beepSoundBuffer;
	std::shared_ptr<sf::Sound> beepSound;
public:
	SoundEntity():
		beepSoundBuffer(std::make_shared<sf::SoundBuffer>()),beepSound(std::make_shared<sf::Sound>()){}
	virtual void playSound()=0;
	virtual void pauseSound()=0;
	virtual void setPlaybackLoop(bool flag)=0;
	virtual ~SoundEntity(){}
};




#endif /* SRC_SOUNDENTITY_HPP_ */
