/*
 * clock.hpp
 *
 *  Created on: Oct 8, 2017
 *      Author: ishan
 */

#ifndef SRC_CLOCK_HPP_
#define SRC_CLOCK_HPP_

#include "entity.hpp"
#include<string>
#include <thread>

#define STRINGIZE(x) #x
#define STRINGIZE_VALUE_OF(x) STRINGIZE(x)
class Clock
{
int min;
int sec;
sf::Font liberationSans;
sf::Text textTime;
std::thread timerThread;

public:
	Clock(){}

	~Clock()
	{
		if(timerThread.joinable())
		{
			timerThread.join();
		}
	}

	Clock(int a_min,int a_sec):min(a_min),sec(a_sec)
	{
		liberationSans.loadFromFile(STRINGIZE_VALUE_OF(FILEPATH));
		textTime.setCharacterSize(15.f);
		textTime.setPosition(800/2.f - 100.f,2.f);
		textTime.setFont(liberationSans);
		textTime.setColor(sf::Color::White);
	}

	void start(sf::RenderWindow & window)
	{
		timerThread =  std::thread(&Clock::update,this,std::ref(window));
	}


	void update(sf::RenderWindow & gameWindow)
	{
		while(1)
		{
			std::this_thread::sleep_for (std::chrono::seconds(1));
			sec--;
			if(sec == 0)
			{
				min--;
				if(min >= 0)
				{
					sec = 59;
				}
			}
			std::string timetext = std::to_string(min) + ":" + std::to_string(sec);
			std::cout<<"in clock:"<<timetext<<std::endl;
			textTime.setString(timetext);
			gameWindow.draw(textTime);
			gameWindow.display();
		}
	}
};

#endif /* SRC_CLOCK_HPP_ */
