#include "stdafx.h"
#include <sstream>
#include <fstream>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "ZombieArena.h"
#include "Player.h"
#include "TextureHolder.h"
#include "Bullet.h"
#include "Pickup.h"
#include <iostream>
#include "PowerUpTimer.h"
#include <ctime>

using namespace sf;
using namespace std;

int main()
{
	// this is used for debugging, showing cout output
	std::ofstream file;
	file.open("cout/cout.txt");
	std::streambuf* sbuf = std::cout.rdbuf();
	std::cout.rdbuf(file.rdbuf());
	//cout is now pointing to a file

	// Here is the instance of TextureHolder
	TextureHolder holder;

	// The game will always be in one of four states
	enum class State{PAUSED, LEVELING_UP, GAME_OVER, PLAYING};
	// Start with the GAME_OVER state
	State state = State::GAME_OVER;

	// Get the screen resolution and create an SFML window
	Vector2f resolution;
	resolution.x = VideoMode::getDesktopMode().width;
	resolution.y = VideoMode::getDesktopMode().height;

	RenderWindow window(VideoMode(resolution.x, resolution.y), "Zombie Arena", Style::Fullscreen);

	// Create an SFML view for the main action
	View mainView(sf::FloatRect(0, 0, resolution.x, resolution.y));

	
	
	// Here is our clock for timing everything
	Clock clock;

	// Here is our clock for timing the power ups
	Clock powerUpClock;
	Time powerUpTime;
	

	// Create variables for power Up lifetime
	
	int start = NULL;
	int life = NULL;
	int powerUpLifeSpan = 12;


	Time elapsed = clock.getElapsedTime();

	// How long has the PLAYING state been active?
	Time gameTimeTotal;

	// Where is the mouse in relation to our world coordinates
	Vector2f mouseWorldPosition;
	// Where is the mouse in relation to screen coordinates
	Vector2i mouseScreenPosition;

	// Create an instance of the Player class
	Player player;

	// Counter for walking of Player
	int counterWalking = 0;

	// counter for walking animation of zombies
	int zombieWalkingCounter = 0;

	// The boundaries of the arena
	IntRect arena;

	// Create the background
	VertexArray background;
	// Load the texture for our background vertex array
	Texture textureBackground = TextureHolder::GetTexture("graphics/background_sheet.png");

	// Prepare for a horde of zombies
	int numZombies;
	int numZombiesAlive;
	Zombie* zombies = NULL;

	// 100 bullets should do
	Bullet bullets[100];
	int currentBullet = 0;
	int bulletsSpare = 24;
	int bulletsInClip = 6;
	int clipSize = 6;
	float fireRate = 1;

	// When was the fire button last pressed?
	Time lastPressed;

	// Hide the mouse pointer and replace with crosshair
	window.setMouseCursorVisible(true);
	Sprite spriteCrosshair;
	Texture textureCrosshair = TextureHolder::GetTexture("graphics/crosshair.png");
	spriteCrosshair.setTexture(textureCrosshair);
	spriteCrosshair.setOrigin(25, 25);

	// Create a couple of pickups
	Pickup healthPickup(1);
	Pickup ammoPickup(2);
	Pickup fireRatePickup(3);

	

	// About the game
	int score = 0;
	int hiScore = 0;

	// For the home/game screen
	Sprite spriteGameOver;
	Texture textureGameOver = TextureHolder::GetTexture("graphics/background.png");
	spriteGameOver.setTexture(textureGameOver);
	// Scale the background image to resolution
	 // DEBUG TO GET VALUES OF resolution.x and .y // 
	std::cout << "width: " << resolution.x << "px\n" << "height: " << resolution.y;
	spriteGameOver.setPosition(0, 0);
	spriteGameOver.setScale(resolution.x / spriteGameOver.getLocalBounds().width, resolution.y / spriteGameOver.getLocalBounds().height);
	//spriteGameOver.setScale(sf::Vector2f(2560, 1440));

	// Create a view for the HUD
	View hudView(sf::FloatRect(0, 0, resolution.x, resolution.y));

	// Create a sprite for the ammo icon
	Sprite spriteAmmoIcon;
	Texture textureAmmoIcon = TextureHolder::GetTexture("graphics/ammo_icon.png");
	spriteAmmoIcon.setTexture(textureAmmoIcon);
	spriteAmmoIcon.setPosition(20, 1360);

	// Load the font
	Font font;
	font.loadFromFile("fonts/zombiecontrol.ttf");

	// Paused
	Text pausedText;
	pausedText.setFont(font);
	pausedText.setCharacterSize(155);
	pausedText.setColor(Color::White);
	pausedText.setString("Press Enter \n to continue");
	FloatRect textRect = pausedText.getLocalBounds();
	pausedText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
	pausedText.setPosition(resolution.x /2.0f, resolution.y /2.0f);
	

	// Game Over
	Text gameOverText;
	gameOverText.setFont(font);
	gameOverText.setCharacterSize(125);
	gameOverText.setColor(Color::White);
	gameOverText.setString("Press Enter to play");
	FloatRect gameOverTextRect = gameOverText.getLocalBounds();
	gameOverText.setOrigin(gameOverTextRect.left + gameOverTextRect.width / 2.0f, gameOverTextRect.top + gameOverTextRect.height / 2.0f);
	gameOverText.setPosition(resolution.x / 2.0f, resolution.y / 2.0f);
	

	// LEVELING UP
	Text levelUpText;
	levelUpText.setFont(font);
	levelUpText.setCharacterSize(50);
	levelUpText.setColor(Color::White);
	levelUpText.setPosition(150, 450);
	std::stringstream levelUpStream;
	levelUpStream <<
		"Choose a power-up from the list below between waves\n" <<
		"1- Increased rate of fire" <<
		"\n2 Increased clip size (next reload)" <<
		"\n3 Increased max health" <<
		"\n4 Increased run speed" <<
		"\n5 More and better health pickups" <<
		"\n6 More and better ammo pickups";
	levelUpText.setString(levelUpStream.str());

	// Ammo
	Text ammoText;
	ammoText.setFont(font);
	ammoText.setCharacterSize(55);
	ammoText.setColor(Color::White);
	FloatRect ammoTextRect = ammoText.getLocalBounds();
	ammoText.setOrigin(ammoTextRect.left + ammoTextRect.width / 2.0f, ammoTextRect.top + ammoTextRect.height / 2.0f);
	ammoText.setPosition(resolution.x - resolution.x + 100, resolution.y - 80);
	

	// Score
	Text scoreText;
	scoreText.setFont(font);
	scoreText.setCharacterSize(55);
	scoreText.setColor(Color::White);
	FloatRect scoreTextRect = scoreText.getLocalBounds();
	scoreText.setOrigin(scoreTextRect.left + scoreTextRect.width / 2.0f, scoreTextRect.top + scoreTextRect.height / 2.0f);
	scoreText.setPosition(resolution.x - resolution.x + 20, 0);

	// Load the high-score from a text file
	std::ifstream inputFile("gamedata/scores.txt");
	if (inputFile.is_open())
	{
		inputFile >> hiScore;
		inputFile.close();
	}

	// Hi Score
	Text hiScoreText;
	hiScoreText.setFont(font);
	hiScoreText.setCharacterSize(55);
	hiScoreText.setColor(Color::White);
	hiScoreText.setPosition(1400, 0);
	std::stringstream s;
	s << "Hi Score:" << hiScore;
	hiScoreText.setString(s.str());

	FloatRect hiScoreTextRect = hiScoreText.getLocalBounds();
	hiScoreText.setOrigin(hiScoreTextRect.left + hiScoreTextRect.width / 2.0f, hiScoreTextRect.top + hiScoreTextRect.height / 2.0f);
	hiScoreText.setPosition(resolution.x - resolution.x + resolution.x - 200, 30);

	// Zombies Remaining
	Text zombiesRemainingText;
	zombiesRemainingText.setFont(font);
	zombiesRemainingText.setCharacterSize(55);
	zombiesRemainingText.setColor(Color::White);
	zombiesRemainingText.setPosition(resolution.x - resolution.x + 1500, resolution.y - 80);
	zombiesRemainingText.setString("Zombies: 100");

	// Wave number
	int wave = 0;
	Text waveNumberText;
	waveNumberText.setFont(font);
	waveNumberText.setCharacterSize(55);
	waveNumberText.setColor(Color::White);
	waveNumberText.setPosition(resolution.x - resolution.x + 1300, resolution.y - 80);
	waveNumberText.setString("Wave: 0");

	// Health Bar
	RectangleShape healthBar;
	healthBar.setFillColor(Color::Red);
	healthBar.setPosition(resolution.x - resolution.x + 300, resolution.y - 80);
	

	// When did we last update the HUD?
	int frameSinceLastHudUpdate = 0;

	// How often (in frames) should we update the HUD
	int fpsMeasurementFrameInterval = 1000;

	// Prepare the hit sound
	SoundBuffer hitBuffer;
	hitBuffer.loadFromFile("sound/hit.wav");
	Sound hit;
	hit.setBuffer(hitBuffer);

	// Prepare the splat sound
	SoundBuffer splatBuffer;
	splatBuffer.loadFromFile("sound/splat.wav");
	sf::Sound splat;
	splat.setBuffer(splatBuffer);

	// Prepare the shoot sound
	SoundBuffer shootBuffer; shootBuffer.loadFromFile("sound/shoot.wav");
	Sound shoot; 
	shoot.setBuffer(shootBuffer);

	// Prepare the reload sound
	SoundBuffer reloadBuffer;
	reloadBuffer.loadFromFile("sound/reload.wav");
	Sound reload;
	reload.setBuffer(reloadBuffer);

	// Prepare the failed sound
	SoundBuffer reloadFailedBuffer;
	reloadFailedBuffer.loadFromFile("sound/reload_failed.wav");
	Sound reloadFailed;
	reloadFailed.setBuffer(reloadFailedBuffer);

	// Prepare the powerup sound
	SoundBuffer powerupBuffer;
	powerupBuffer.loadFromFile("sound/powerup.wav");
	Sound powerup;
	powerup.setBuffer(powerupBuffer);

	// Prepare the pickup sound
	SoundBuffer pickupBuffer;
	pickupBuffer.loadFromFile("sound/pickup.wav");
	Sound pickup;
	pickup.setBuffer(pickupBuffer);


	// The main game loop
	while (window.isOpen())
	{
		/*
		**************
		 Handle Input
		**************
		*/

		// Handle events by polling
		Event event;
		while (window.pollEvent(event))
		{
			if (event.type == Event::KeyPressed)
			{
				// Pause a game while playing
				if (event.key.code == Keyboard::Return &&
					state == State::PLAYING)
				{
					state = State::PAUSED;
				}

				// Restart while paused
				else if (event.key.code == Keyboard::Return &&
					state == State::PAUSED)
				{
					state = State::PLAYING;
					// Reset the clock so there isn't a frame jump
					clock.restart();
				}
				// Start a new game while in GAME_OVER state
				else if (event.key.code == Keyboard::Return && 
					state == State::GAME_OVER)
				{
					state = State::LEVELING_UP;
					wave = 0;
					score = 0;

					// Prepare the gun and ammo for next game
					currentBullet = 0;
					bulletsSpare = 24;
					bulletsInClip = 6;
					clipSize = 6;
					fireRate = 1;

					// Reset the player's stats
					player.resetPlayerStats();
				}
				if (state == State::PLAYING)
				{
					// Reloading
					if (event.key.code == Keyboard::R)
					{
						if (bulletsSpare >= clipSize)
						{
							// Plenty of bullets. Reload.
							bulletsInClip = clipSize;
							bulletsSpare -= clipSize;
							reload.play();
						}
						else if (bulletsSpare > 0)
						{
							// Only a few bullets left
							bulletsInClip = bulletsSpare;
							bulletsSpare = 0;
							reload.play();
						}
						else
						{
							reloadFailed.play();
						}
					}
				}
			} 
		} // End event polling


		// Handle the player quitting
		if (Keyboard::isKeyPressed(Keyboard::Escape))
		{
			window.close();
		}

		// Handle WASD while playing
		if (state == State::PLAYING)
		{
			// Handle the pressing and releasing of the WASD keys
			if (Keyboard::isKeyPressed(Keyboard::W))
			{
				player.moveUp();
			    player.spriteTextRect((counterWalking / 200) * 96 , 64, 96, 96);
			}
			else
			{
				player.stopUp();
				player.spriteTextRect(0, 64, 96, 96);
			}
			if (Keyboard::isKeyPressed(Keyboard::S))
			{
				player.moveDown();
				player.spriteTextRect((counterWalking / 200) * 96, 64, 96, 96);
			}
			else
			{
				player.stopDown();
				//player.spriteTextRect(0, 64, 96, 96);
			}
			if (Keyboard::isKeyPressed(Keyboard::A))
			{
				player.moveLeft();
				player.spriteTextRect((counterWalking / 200) * 96, 64, 96, 96);
			}
			else
			{
				player.stopLeft();
				//player.spriteTextRect(0, 64, 96, 96);
			}
			if (Keyboard::isKeyPressed(Keyboard::D))
			{
				player.moveRight();
				player.spriteTextRect((counterWalking / 200) * 96, 64, 96, 96);
			}
			else
			{
				player.stopRight();
				//player.spriteTextRect(0, 64, 96, 96);
			}

			counterWalking++;

			if (counterWalking == 1600)
			{
				counterWalking = 0;
			}

			// Fire a bullet
			if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
			{
				if (gameTimeTotal.asMilliseconds()
					- lastPressed.asMilliseconds()
					> 1000 / fireRate && bulletsInClip > 0)
				{
					// Pass the center of the player and center of the crosshair to the shoot function
					bullets[currentBullet].shoot(
						player.getCenter().x, player.getCenter().y,
						mouseWorldPosition.x, mouseWorldPosition.y);

					currentBullet++;
					if (currentBullet > 99)
					{
						currentBullet = 0;
					}
					lastPressed = gameTimeTotal;
					shoot.play();
					bulletsInClip--;
				}
			}// End fire a bullet

		}// End WASD while playing

		// Handle the LEVELING up state
		if (state == State::LEVELING_UP)
		{
			// Handle the player LEVELING up
			if (event.key.code == Keyboard::Num1)
			{
				// Increase fire rate
				fireRate++;
				state = State::PLAYING;
			}

			if (event.key.code == Keyboard::Num2)
			{
				// Increase clip size;
				clipSize++;
				state = State::PLAYING;
			}

			if (event.key.code == Keyboard::Num3)
			{
				// Increase health
				player.upgradeHealth();
				state = State::PLAYING;
			}

			if (event.key.code == Keyboard::Num4)
			{
				// Increase speed
				player.upgradeSpeed();
				state = State::PLAYING;
			}

			if (event.key.code == Keyboard::Num5)
			{
				// Upgrade pickup
				healthPickup.upgrade();
				state = State::PLAYING;
			}

			if (event.key.code == Keyboard::Num6)
			{
				// Upgrade pickup
				ammoPickup.upgrade();
				state = State::PLAYING;
			}

			if (state == State::PLAYING)
			{
							
				// Increase the wave number
				wave++;

				// Prepare the level
				// We will modify the next two lines later
				arena.width = 500 * wave;
				arena.height = 500 * wave;
				arena.left = 0;
				arena.top = 0;

				// Pass the vertex array by reference to the createBackground function
				int tileSize = createBackground(background, arena);

				// Spawn the player in the middle of the arena
				player.spawn(arena, resolution, tileSize);

				// Configure the pickups
				healthPickup.setArena(arena);
				ammoPickup.setArena(arena);
				fireRatePickup.setArena(arena);
			

				// Create a horde of zombies
				numZombies = 5 * wave;

				// Delete the previously allocated memory (if it exists)
				delete[]zombies;
				zombies = createHorde(numZombies, arena);
				numZombiesAlive = numZombies;

				// Play the powerup sound
				powerup.play();

				// Reset the clock so there isnt a frame jump
				clock.restart();


			}
		} // End LEVELING up


		/*
		******************
		 UPDATE THE FRAME
		******************
		*/

		if (state == State::PLAYING)
		{
			// Update the delta time
			Time dt = clock.restart();

			// Update the total game time
			gameTimeTotal += dt;

			// 
			float totalGameTimeSofar = gameTimeTotal.asSeconds();

			// Make a decimal fraction of 1 from the delta time
			float dtAsSeconds = dt.asSeconds();

			// Where is the mouse pointer
			mouseScreenPosition = Mouse::getPosition();

			// Convert mouse position to world coordinates of mainView
			mouseWorldPosition = window.mapPixelToCoords(
				Mouse::getPosition(), mainView);

			// Set the crosshair to the mouse world location
			spriteCrosshair.setPosition(mouseWorldPosition);

			// Update the player
			player.update(dtAsSeconds, Mouse::getPosition());

			// Make a note of the players new position
			Vector2f playerPosition(player.getCenter());

			// Make the view center around the player
			mainView.setCenter(player.getCenter());

			// Loop through each zombie and update them
			for (int i = 0; i < numZombies; i++)
			{
				if (zombies[i].isAlive())
				{
					zombies[i].update(dt.asSeconds(), playerPosition);
					zombies[i].spriteTextRect((zombieWalkingCounter / 3200) * 110, 0, 110, 55);

					zombieWalkingCounter++;

					if (zombieWalkingCounter == 25600)
					{
						zombieWalkingCounter = 0;
					}
				}

			}

			// Update any bullets that are in flight
			for (int i = 0; i < 100; i++)
			{
				if (bullets[i].isInFlight())
				{
					bullets[i].update(dtAsSeconds);
				}
			}

			// Update the pickups
			healthPickup.update(dtAsSeconds);
			ammoPickup.update(dtAsSeconds);
			fireRatePickup.updatePowerUp(dtAsSeconds);





			// Collision detection
			// Have any zombies been shot?
			for (int i = 0; i < 100; i++)
			{
				for (int j = 0; j < numZombies; j++)
				{
					if (bullets[i].isInFlight() && zombies[j].isAlive())
					{
						if (bullets[i].getPosition().intersects(zombies[j].getPosition()))
						{
							// stop the bullet
							bullets[i].stop();
							// Register the hit and see if it was a kill
							if (zombies[j].hit())
							{
								// Not just a hit but a kill too
								score += 10;
								if (score >= hiScore)
								{
									hiScore = score;
								}
								numZombiesAlive--;
								// When all the zombies are dead (again)
								if (numZombiesAlive == 0)
								{
									state = State::LEVELING_UP;
								}
							}

							// Make a splat sound
							splat.play();
						}
					}
				}
			}// End zombie being shot

			 // Have any zombies touched the player
			for (int i = 0; i < numZombies; i++)
			{
				if (player.getPosition().intersects
				(zombies[i].getPosition()) && zombies[i].isAlive())
				{
					if (player.hit(gameTimeTotal))
					{
						hit.play();
					}
					if (player.getHealth() <= 0)
					{
						state = State::GAME_OVER;

						std::ofstream outputFile("gamedata/scores.txt");
						outputFile << hiScore;
						outputFile.close();
					}
				}
			}// End player touched

			// Has the player touched health pickup?
			if (player.getPosition().intersects
			(healthPickup.getPosition()) && healthPickup.isSpawned())
			{
				player.increaseHealthLevel(healthPickup.gotIt());
				// Play a sound
				pickup.play();
			}

			// Has the player touched ammo pickup
			if (player.getPosition().intersects
			(ammoPickup.getPosition()) && ammoPickup.isSpawned())
			{
				bulletsSpare += ammoPickup.gotIt();
				//Play a sound
				reload.play();
			}

			
			// Has the player touched fireRarePickup

				// ***************************** TO DO  ****************************
			

			if (player.getPosition().intersects
			(fireRatePickup.getPosition()) && fireRatePickup.isSpawned())
			{
				fireRatePickup.gotIt();
				//Play a sound
				reload.play();
				powerUpTime = powerUpClock.getElapsedTime();
				start = powerUpTime.asSeconds();
				life = start + powerUpLifeSpan;

				// For debugging
				/*
				cout << "FireRate = " << fireRate << endl;
				cout << "powerUpTime: " << powerUpTime.asSeconds() << endl;
				cout << "start: " << start << endl;
				cout << "life: " << life << endl;*/
			}
			start = start;
			life = life;
			if ((powerUpTime.asSeconds() < life))
					
				{
						powerUpTime = powerUpClock.getElapsedTime();
						fireRate = 5;
						// For debugging
						/*cout << "***************************************LIFE**************************************************" << endl;
						cout << "LIFE = " << life << endl;*/
						
						/*cout << "FireRate = " << fireRate << endl;
						cout << "powerUpTime: " << powerUpTime.asSeconds() << endl;
						cout << "start: " << start << endl;
						cout << "life: " << life << endl;*/
						
						
				}

			if ((powerUpTime.asSeconds() > life)) {
				{
						fireRate = 1;
						// For debugging
						/*cout << "FireRate = " << fireRate << endl;
						cout << "powerUpTime2: " << powerUpTime.asSeconds() << endl;
						cout << "start2: " << start << endl;
						cout << "life2: " << life << endl;*/
				}
			}
		
						

			// size up the health bar
			healthBar.setSize(Vector2f(player.getHealth() * 3, 70));

			// Increment the number of frames since the previous update
			frameSinceLastHudUpdate++;

			// recalculate every fpsMeasurementFrameInterval frames
			if (frameSinceLastHudUpdate > fpsMeasurementFrameInterval)
			{
				// Update game HUD text
				std::stringstream ssAmmo;
				std::stringstream ssScore;
				std::stringstream ssHiScore;
				std::stringstream ssWave;
				std::stringstream ssZombiesAlive;

				// Update the ammo text
				ssAmmo << bulletsInClip << "/" << bulletsSpare;
				ammoText.setString(ssAmmo.str());

				// Update the score text
				ssScore << "Score" << score;
				scoreText.setString(ssScore.str());

				// Update the high score text
				ssHiScore << "Hi Score:" << hiScore;
				hiScoreText.setString(ssHiScore.str());

				// Update the wave text
				ssWave << "Wave:" << wave;
				waveNumberText.setString(ssWave.str());

				// Update the high score text
				ssZombiesAlive << "Zombies:" << numZombiesAlive;
				zombiesRemainingText.setString(ssZombiesAlive.str());

				frameSinceLastHudUpdate = 0;
			}// End HUD update


		} // End updating the scene
		/*
		**************
		Draw the scene
		**************
		*/
		if (state == State::PLAYING)
		{
			window.clear();

			// Set the mainView to be displayed in the window
			// And draw everything related to it
			window.setView(mainView);

			// Draw the background
			window.draw(background, &textureBackground);

			// Draw the zombies
			for (int i = 0; i < numZombies; i++)
			{
				window.draw(zombies[i].getSprite());
			}

			// Draw the bullets
			for (int i = 0; i < 100; i++)
			{
				if (bullets[i].isInFlight())
				{
					window.draw(bullets[i].getShape());
				}
			}

			

			// Draw the player
			window.draw(player.getSprite());
			

			// Draw the pickups, if currently spawned
			if (ammoPickup.isSpawned())
			{
				window.draw(ammoPickup.getSprite());
			}
			if (healthPickup.isSpawned())
			{
				window.draw(healthPickup.getSprite());
			}
			if (fireRatePickup.isSpawned())
			{
				window.draw(fireRatePickup.getSprite());
			}

			

			// Draw the crosshair
			window.draw(spriteCrosshair);

			// Switch to the HUD view
			window.setView(hudView);

			// Draw all the HUD elements
			window.draw(spriteAmmoIcon);
			window.draw(ammoText);
			window.draw(scoreText);
			window.draw(hiScoreText);
			window.draw(healthBar);
			window.draw(waveNumberText);
			window.draw(zombiesRemainingText);

		}

		if (state == State::LEVELING_UP)
		{
			window.draw(spriteGameOver);
			window.draw(levelUpText);
		}
		if (state == State::PAUSED)
		{
			window.draw(pausedText);
		}
		if (state == State::GAME_OVER)
		{
			window.draw(spriteGameOver);
			window.draw(gameOverText);
			window.draw(scoreText);
			window.draw(hiScoreText);
		}
		window.display();

	} // End game loop

	// Delete the previously allocated memory (if it still exists)
	delete[] zombies;


	return 0;
}

