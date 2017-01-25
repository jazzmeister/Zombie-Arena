#include "stdafx.h"
#include <iostream>
#include <ctime>
#include <cstdlib>
#include "PowerUpTimer.h"
#include "SFML\Graphics.hpp"
#include <sstream>
#include <fstream>

using namespace sf;
using namespace std;

PowerUpTimer::PowerUpTimer()
{
	
}

bool PowerUpTimer::RunTimer()
{
	Clock clock;
	Time time;
	float start = 0;
	float life = 5;
	int flag = 1;
	
	while (flag == 1)
	{
		time = clock.getElapsedTime();
		cout << time.asSeconds() << endl;

		if (time.asSeconds() > start && (time.asSeconds() < life)) {
			cout << "***************************************LIFE**************************************************" << endl;
			cout << "LIFE = " << life << endl;
			
			cout << "FireRate = " << endl;
			return 1;
		}
		else
		{
			
			cout << "FireRate = " << endl;
			flag = 0;
			return 0;
		}
		
	}
	
}