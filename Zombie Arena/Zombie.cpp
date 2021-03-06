#include "stdafx.h"
#include "Zombie.h"
#include "TextureHolder.h"
#include <cstdlib>
#include <ctime>
#include <SFML/Graphics.hpp>

using namespace std;

void Zombie::spawn(float startX, float startY, int type, int seed)
{
	switch (type)
	{
		case 0:
			// Bloater
			m_Sprite = Sprite(TextureHolder::GetTexture("graphics/zombie3.png"));

			m_Speed = 40;
			m_Health = 5;
			break;

		case 1:
			// Chaser
			m_Sprite = Sprite(TextureHolder::GetTexture("graphics/zombie1.png"));

			m_Speed = 70;
			m_Health = 1;
			break;

		case 2:
			// Crawler
			m_Sprite = Sprite(TextureHolder::GetTexture("graphics/zombie2.png"));

			m_Speed = 20;
			m_Health = 3;
			break;
	}

	// Modify the speed to make the zombie unique
	// Every zombie is unique. Create a speed modifier
	srand((int)time(0) * seed);

	// Somewhere between 80 and 100
	float modifier = (rand() % MAX_VARIANCE) + OFFSET;

	// Express this as a fraction of 1
	// (This makes each zombie's speed unique, so they dont bunch up)
	modifier /= 100; // Now equal between .7 and 1
	m_Speed *= modifier;

	// Initialise a zombie's location
	m_Position.x = startX;
	m_Position.y = startY;

	// Set a zombie's origin to its center
	m_Sprite.setOrigin(25, 25);

	// Set zombie's position
	m_Sprite.setPosition(m_Position);

}

// Function to test if zombie has been hit and is dead or alive
bool Zombie::hit()
{
	m_Health--;

	if (m_Health < 0)
	{
		// dead
		m_Alive = false;
		m_Sprite.setTexture(TextureHolder::GetTexture("graphics/blood1.png"));

		return true;
	}

	// injured but not dead yet
	return false;
}

bool Zombie::isAlive()
{
	return m_Alive;
}

FloatRect Zombie::getPosition()
{
	return m_Sprite.getGlobalBounds();
}

Sprite Zombie::getSprite()
{
	return m_Sprite;
}


void Zombie::update(float elapsedTime, Vector2f playerLocation)
{
	float playerX = playerLocation.x;
	float playerY = playerLocation.y;

	// Update the zombie position variables
	if (playerX > m_Position.x)
	{
		m_Position.x = m_Position.x + m_Speed * elapsedTime;
	}

	if (playerY > m_Position.y)
	{
		m_Position.y = m_Position.y + m_Speed * elapsedTime;
	}

	if (playerX < m_Position.x)
	{
		m_Position.x = m_Position.x - m_Speed * elapsedTime;
	}

	if (playerY < m_Position.y)
	{
		m_Position.y = m_Position.y - m_Speed * elapsedTime;
	}

	// Move the sprite
	m_Sprite.setPosition(m_Position);

	// Face the sprite in the correct direction
	float angle = (atan2(playerY - m_Position.y, playerX - m_Position.x) * 180) / 3.141;

	m_Sprite.setRotation(angle);
}

void Zombie::spriteTextRect(int a, int b, int x, int y)
{
	m_Sprite.setTextureRect(IntRect(a, b, x, y));

	//m_a = a;
	//m_b = b;
	//m_x = x;
	//m_y = y;

	//return a, b, x, y;
}















































