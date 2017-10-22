#ifndef SRC_GAME_HPP_
#define SRC_GAME_HPP_


#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <vector>
#include <memory>
#include <tuple>
#include <cmath>
#include <typeinfo>
#include <chrono>
#include "ball.hpp"
#include "brick.hpp"
#include "paddle.hpp"
#include "bullet.hpp"
#include "entity.hpp"
#include "lives.hpp"
#include "manager.hpp"
#include "clock.hpp"
#include "utility.hpp"
#include <thread>
#include <mutex>
#include <functional>
#include <condition_variable>

#define STRINGIZE(x) #x
#define STRINGIZE_VALUE_OF(x) STRINGIZE(x)
/**
 * Game class: It contains the main game engine loop thread which runs the game.
 * It has a manager as a member variable which takes care of generating the entities and destroying them.
 * This decision is made in this class and actions are taken in the utility functions.
 */
class Game
{
	enum class GameState		// Enum for the state of the game. This is used to take several decision in the game engine loop
	{
		inprocess,paused,victory,lost,newlife,transit,gameover
	};

	sf::Font liberationSans;	// loading the style of font using sf::Font which will be set in sf::Text

	sf::Text textState /*text for displaying state("You won","You lost")*/,
	textLives /*text for displaying the remaining lives on the top right corner*/,
	textScore /*text for displaying the remaining lives on the top left corner*/,
	textTime,
	textStage /*text for displaying the stage of the game when it starts*/;

	Manager manager; /*manager class which will take care of creating and destroying the entities*/

	GameState state{GameState::inprocess}; /*enum variable for setting the state of the game*/

	bool ifGamePaused{false}; /*bool to check if game is paused or not*/

	std::shared_ptr<Clock> timerClock;

	bool readyForupdate{false};
	bool updateDone{false};
	volatile bool startAI{true};

	int currentStage; /*current stage variable which will be incremented as the stage progresses*/

	/*threads responsible for several tasks related to entities*/
	std::thread spawedThread;
	std::thread mainEngineThread;
	std::thread updateEntityThread;
	std::thread timerThread;
	std::thread AIModeThread;
	std::condition_variable updateCV,AIcv;
	std::mutex mtx,AImtx;

	double timeSeconds;
	static constexpr int brickCountX{11};  										// No of bricks in the X direction, no of columns, 11
	static constexpr int brickCountY{4};   										// No of bricks in the Y direction, no of rows , 4
	static constexpr int brickStartCol{1}; 										// start column of the bricks, 1
	static constexpr int brickStartRow{2}; 										// start row of the bricks, 2
	static constexpr int brickSpacing{6};  										// spacing between the bricks
	static constexpr float brickOffsetX{22.f}; 									// brick offset in the X direction

	sf::RenderWindow window{{wndWidth,wndHeight},"Arkanoid - 2"}; 				//Game window with width,height and name string

	int gamescore; 																// Game score

	/**
	 * This function takes std::tuple as a parameter and sets the property of the font
	 */
	int setFontPropertiesCall(std::tuple<sf::Text&,float, float> textProp)
	{
		std::get<0>(textProp).setFont(liberationSans);
		std::get<0>(textProp).setPosition(std::get<1>(textProp),std::get<2>(textProp));
		std::get<0>(textProp).setCharacterSize(15.f);
		std::get<0>(textProp).setColor(sf::Color::White);
		return 1;																// Return value is required because initializer_list is of type "int" in setFontProperties
	}

	/**
	 * Variadic template function which takes any number of arguments and calls the function "setFontPropertiesCall" for every passed argument
	 */
	template<typename...Ts>
	void setFontProperties(Ts&&...args)
	{
		(void)std::initializer_list<int>{(setFontPropertiesCall(args),0)...};
	}

	void createEntities()
	{
		for(int i = 0;i < brickCountX;i++)										// put bricks
		{
			for(int j = 0;j < brickCountY;j++)
			{
				float x{(i + brickStartCol*(0.7f))*(Brick::defWidth + brickSpacing)};
				float y{(j + brickStartRow)*(Brick::defHeight + brickSpacing)};
				if(i%2==0)
					manager.create<Brick>(brickOffsetX +x ,y,sf::Color::Cyan,1,currentStage,false);		// create brick entity which requires an update, so last parameter is false
				else
					manager.create<Brick>(brickOffsetX +x ,y,sf::Color::Magenta,3,currentStage,false);	// create brick entity which requires an update, so last parameter is false
			}
		}

		manager.create<Ball>(wndWidth/2.f,wndHeight/2.f,false,-2.f,2.f);		// create the ball entity
		manager.create<Paddle>(wndWidth/2.f,wndHeight-50,true);				// create the paddle entity
		int offset = 0;															// offset between the lives circles
		for(int i = 0; i < manager.totalLives; i++)
		{
			manager.create<lives>(720.f + offset,12.f,false);					// create the lives entity which is circles in the top right corner
			offset += 2*lives::defRadius + 2.f;
		}

	}

public:
	Game()
	{
		liberationSans.loadFromFile(STRINGIZE_VALUE_OF(FILEPATH)); 				// loading the font file from file system in sf::Font
		gamescore = 0; 															// initial game score
		currentStage = 1; 														// initial stage number
		window.setFramerateLimit(60); 											// setting the frame rate for the window
		timeSeconds = 0;
		setFontProperties(														// This is call the to the function with variadic templates.
				std::make_tuple(std::ref(textLives),650.f,2.f),					// It can take any number of tuples with type <sf::Text&,float,float>
				std::make_tuple(std::ref(textScore),2.f,2.f),
				std::make_tuple(std::ref(textStage),wndWidth/2.f - 70.f,wndHeight/2.f),
				std::make_tuple(std::ref(textState),wndWidth/2.f - 100.f,wndHeight/2.f),
				std::make_tuple(std::ref(textTime),wndWidth/2.f - 100.f,2.f)
		);
	}

	void restart()
	{
		manager.clear(); 														// clear all the entities from the container while restart
		showStageNumberScreen();
		timeSeconds = 1000;														// showStageNumberScreen takes 2 seconds. so reset seconds count to zero after that, not before
		createEntities();
		Ball* mball = manager.getSingleEntity<Ball>();
		std::cout<<"entities created"<<std::endl;
	}

	void changeState(const GameState& s)
	{
		std::this_thread::sleep_for (std::chrono::milliseconds(200));			// This Delay is required because when spacebar is pressed, the bullet can not be shot immediately
		state = s;
	}

	void showStageNumberScreen()
	{
		window.clear(sf::Color::Black);
		textStage.setString("Stage: " + std::to_string(currentStage));
		window.draw(textStage);
    	window.display();
		std::this_thread::sleep_for(std::chrono::seconds(2));
	}

	void manageText()
	{
		textStage.setString("Stage: " + std::to_string(currentStage));		// setting the stage text string which will be shown in the begining for 2 seconds and then fades away.
		while(1)															// while loop which set the stage string to null when other thread sets the string to non-null
		{
			std::this_thread::sleep_for(std::chrono::seconds(2));
			textStage.setString("");
		}
	}

	void updateEntities()
	{
		std::cout<<"update entities thread started"<<std::endl;
		while(1)
		{
			std::unique_lock<std::mutex> lck(mtx);
			updateCV.wait(lck,[this](){return readyForupdate;});
			readyForupdate = false;
			manager.update();
			updateDone = true;
			lck.unlock();
			updateCV.notify_one();
		}
	}

	void automateGame()
	{
		std::cout<<"AI thread started"<<std::endl;
		while(1)
		{
			std::unique_lock<std::mutex> lck(AImtx);
			AIcv.wait(lck,[this](){return startAI;});
			manager.predictedPaddlePosition();
			lck.unlock();
		}
	}

	void showTime()
		{
		std::cout<<"timer thread started"<<std::endl;
			while(1)
			{
				std::this_thread::sleep_for (std::chrono::seconds(1));			// This Delay is required because when spacebar is pressed, the bullet can not be shot immediately
				timeSeconds--;
				if(timeSeconds == 0) break;
				textTime.setString(std::string("Time:" + std::to_string(static_cast<int>(timeSeconds)) + "          "+ "Stage:"+std::to_string(static_cast<int>(currentStage))));
			}
		}

	void run()
	{
		mainEngineThread  = std::thread([this](){startEngineLoop();});
		updateEntityThread = std::thread([this](){updateEntities();});
		timerThread = std::thread([this](){showTime();});
		AIModeThread = std::thread([this](){automateGame();});
		mainEngineThread.join();
		updateEntityThread.join();
		timerThread.join();
		AIModeThread.join();

		//      TODO: Make a timer class work, problem is it flickers the screen
		//		timerClock = std::make_shared<Clock>(2,2);
		//		timerClock->start(window);

	}


	void startEngineLoop()
	{
		std::cout<<"main engine thread started"<<std::endl;
		while(true)
		{
			window.clear(sf::Color::Black);
			window.draw(textTime);
			if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) exit(0);
			manager.checkBallDropped();
			if(manager.checkBallDropped())
			{
				manager.handleBallDrop();
				--manager.totalLives;
				if(0 >= manager.totalLives)
				{
					window.clear(sf::Color::Black);
					state = GameState::lost;
					textState.setString("You Lost!!");
			    	manager.draw(window);
					window.draw(textState);
			    	window.display();
				}else{
					state = GameState::newlife;
					Paddle* paddleentity = manager.getSingleEntity<Paddle>();
			     	manager.create<Ball>(paddleentity->x(),paddleentity->y()-2*Ball::defRadius,true,2.f,-2.f);
				}
			}

			// When time is up, you lost the game
			if(timeSeconds == 0)
			{
				window.clear(sf::Color::Black);
				state = GameState::lost;
				textState.setString("You Lost!!");
				window.draw(textState);
		    	window.display();
			}

			// If game is in progress and space bar is hit, then shoot bullets
			if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) && state == GameState::inprocess)
			{
				Paddle* paddleentity = manager.getSingleEntity<Paddle>();
				manager.create<Bullet>(paddleentity->x(),paddleentity->y(),false);
			}

			if(state == GameState::newlife)
			{
		    	{
		    		std::lock_guard<std::mutex> lk(mtx);
					Ball* mball = manager.getSingleEntity<Ball>();
					mball->solveBallPaddleRelativeMotion();
		    		readyForupdate = true;
		    	}
		    	updateCV.notify_one();
		    	{
		    		std::unique_lock<std::mutex> lk(mtx);
		    		updateCV.wait(lk,[this](){return updateDone;});
		    		updateDone=false;
		    	}
		    	textScore.setString("Score:"+std::to_string(gamescore));
                textLives.setString("Balls:");
                window.draw(textScore);
                window.draw(textLives);
		    	manager.refresh();
		    	manager.draw(window);
		    	window.display();

			}

			if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) && state == GameState::newlife)
			{
		    	{
		    		std::lock_guard<std::mutex> lk(mtx);
					Ball* ballentity = manager.getSingleEntity<Ball>();
					ballentity->velocity.x = -2.f;
					ballentity->velocity.y = 2.f;
					ballentity->updateRequired = false;
		    		readyForupdate = true;
		    	}
		    	updateCV.notify_one();
		    	{
		    		std::unique_lock<std::mutex> lk(mtx);
		    		updateCV.wait(lk,[this](){return updateDone;});
		    		updateDone=false;
		    	}
				spawedThread = std::thread([this](){changeState(GameState::inprocess);});

		    	textScore.setString("Score:"+std::to_string(gamescore));
                textLives.setString("Balls:");
                window.draw(textScore);
                window.draw(textLives);
		    	manager.refresh();
		    	manager.draw(window);
		    	window.display();

			}

			if(manager.getAll<Brick>().empty())
			{
//	    		std::unique_lock<std::mutex> lk(AImtx);
//				startAI = false;
				textState.setString("You Won!!");
				manager.draw(window);
				window.draw(textState);
		    	window.display();
		    	state = GameState::inprocess;
		    	currentStage++;
		    	restart();
//		    	lk.unlock();
//		    	startAI = true;
//		    	AIcv.notify_one();
			}

			if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::P) && !ifGamePaused)
			{
				state = GameState::paused;
				textState.setString("Paused");
				ifGamePaused = true;
		    	manager.draw(window);
				window.draw(textState);
		    	window.display();
			}

			if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::R) && ifGamePaused)
			{
				state = GameState::inprocess;
				ifGamePaused = false;
			}

		    if(state == GameState::inprocess)
		    {
		    	{
		    		std::lock_guard<std::mutex> lk(mtx);
		    		readyForupdate = true;
		    	}
		    	updateCV.notify_one();
		    	{
		    		std::unique_lock<std::mutex> lk(mtx);
		    		updateCV.wait(lk,[this](){return updateDone;});
		    		updateDone=false;
		    	}


                manager.forEach<Ball>([this](Ball& mBall)		    	// resolve ball and brick collision for each brick
                {
                	manager.forEach<Brick>([&mBall](Brick& mBrick)
                    {
                		solveBallBrickCollision(mBrick, mBall);
                    });
                });

                manager.forEach<Ball>([this](Ball& mball)                // resolve ball and paddle collision
                		{
                			manager.forEach<Paddle>([&mball](Paddle& mPaddle)
                			{
                				solvePaddleBallCollision(mPaddle, mball);
                			});
                		});


                if(manager.getAll<Bullet>().size()>0)
                {
					manager.forEach<Brick>([this](Brick& mbrick)
							{
						manager.forEach<Bullet>([&mbrick](Bullet& mbullet)
								{
									solveBrickBulletCollision(mbrick,mbullet);
								});
							});
                }

		    	manager.forEach<Brick>([this](Brick& brick)
		    			{
		    				if(brick.destroyed)
		    				{
		    					gamescore += brick.strength;
		    				}
		    			});

		    	manager.forEach<Brick>([this](Brick& mbrick)
		    	{
		    		if(mbrick.destroyed)
		    			{

		    			}
		    	}
		    	);

                textScore.setString("Score:"+std::to_string(gamescore));
                textLives.setString("Balls:");

                window.draw(textScore);
                window.draw(textLives);
		    	manager.refresh();
		    	manager.draw(window);
		    	window.display();

		    }
			if(spawedThread.joinable())
			{
				spawedThread.join();
			}
		}

	}
};

#endif /* SRC_GAME_HPP_ */
