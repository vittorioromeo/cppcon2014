/*
 * BallSound.hpp
 *
 *  Created on: Oct 4, 2017
 *      Author: ishan
 */

#ifndef SRC_BALLSOUND_HPP_
#define SRC_BALLSOUND_HPP_

#include "SoundEntity.hpp"
#include "entity.hpp"

class BallSound : public SoundEntity
{
	std::string filepath;
public:
	BallSound(const std::string& fpath):SoundEntity()
	{
		filepath = fpath;
		beepSoundBuffer->loadFromFile(filepath);
		beepSound->setBuffer(*beepSoundBuffer);

	}

    void playSound() override
	{
		beepSound->play();
	}

	void pauseSound() override
	{
		beepSound->pause();
	}

	void setPlaybackLoop(bool flag) override
	{
		beepSound->setLoop(flag);
	}


};




#endif /* SRC_BALLSOUND_HPP_ */
