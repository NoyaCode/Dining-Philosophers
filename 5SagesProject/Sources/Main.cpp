#include <thread> 
#include <chrono> 
#include <mutex>  
#include <iostream> 
#include <random>
#include <vector>


#define NC "\033[0m"
#define RED "\033[1;31m"
#define GRN "\033[1;32m"
#define BLU "\033[1;34m"
#define CYN "\033[1;36m"
#define YLW "\033[1;33m"


using namespace std;

class GameData
{
public:
	int sageCount = 0;
	int minThink = 0;
	int maxThink = 0;
	int minEat = 0;
	int maxEat = 0;
	int eatingTime = 0;
	int days = 0;

public:
	GameData() {}
	~GameData(){}

	void InitData()
	{
		while (sageCount <= 1)
		{
			system("CLS");
			cout << "\n	Entrer the " YLW "number of sages" NC " : \n	";
			cin >> sageCount;
		}

		while (minThink < 1)
		{
			system("CLS");
			cout << "\n	Enter the " YLW "MINIMUM" NC " time to " CYN "THINK" NC " : \n	";
			cin >> minThink;
		}
		while (maxThink < minThink)
		{
			system("CLS");
			cout << "\n	Enter the " YLW "MAXIMUM" NC " time to " CYN "THINK" NC " (must be " YLW "higher or equal to " GRN << minThink << NC ") : \n	";
			cin >> maxThink;
		}
		while (minEat < 1)
		{
			system("CLS");
			cout << "\n	Enter the " YLW "MINIMUM" NC " time to " RED "EAT" NC " : \n	";
			cin >> minEat;
		}
		while (maxEat < minEat)
		{
			system("CLS");
			cout << "\n	Enter the " YLW "MAXIMUM" NC " time to " RED "EAT" NC " (must be " YLW "higher or equal to " GRN << minEat << NC ") : \n	";
			cin >> maxEat;
		}
		while (eatingTime < 1)
		{
			system("CLS");
			cout << "\n	Entrer the " YLW "total meal time" NC " : \n	";
			cin >> eatingTime;
		}
		while (days < 1)
		{
			system("CLS");
			cout << "\n	Entrer the " YLW "number of days" NC " : \n	";
			cin >> days;
		}
	}
};

enum class State
{
	THINKING,
	HUNGRY,
	EATING,
	SLEEPING,
};

struct StateData
{
	State state = State::THINKING;
	int timer = 0;
};

void PrintState(const GameData& data, vector<StateData>& stateData, const int currentDay)
{
	bool loop = true;

	while (loop)
	{
		loop = false;
		for (int id = 0; id < data.sageCount; id++)
		{
			if (stateData[id].state != State::SLEEPING)
			{
				loop = true;
				break;
			}
		}

		system("CLS");
		cout << YLW"\n		     DAY " << currentDay << "/" << data.days << NC;
		cout << "\n   -----------------------------------------------\n";
		for (int id = 0; id < data.sageCount; id++)
		{
			switch (stateData[id].state)
			{
			case State::THINKING:
				cout << "	     Sage " << id << " is" CYN" thinking" NC "..." YLW " (" << stateData[id].timer << "s)" NC "\n\n";
				break;
			case State::HUNGRY:
				cout << "		Sage " << id << " is" GRN" hungry" NC "!\n\n";
				break;
			case State::EATING:
				cout << "	  Sage " << id << " is" RED" eating" NC "... Yummy!" YLW " (" << stateData[id].timer << "s)" NC "\n\n";
				break;
			default:
				cout << "     Sage " << id << " finished his day, he is" BLU" sleeping" NC " zzZ\n\n";
				break;
			}

			if (stateData[id].timer > 0)
				stateData[id].timer--;
		}
		cout << "   -----------------------------------------------\n";

		this_thread::sleep_for(std::chrono::seconds(1));
	}
}

void DailyRoutine(const int id, GameData& data, vector<StateData>& stateData, mutex *forks)
{
	int currentEatingTime = 0;

	while (currentEatingTime < data.eatingTime)
	{
		srand(time(NULL) + id );

		// THINKING State

		stateData[id].state = State::THINKING;
		int timeToThink = rand() % (data.maxThink + 1 - data.minThink) + data.minThink;
		stateData[id].timer = timeToThink;
		this_thread::sleep_for(std::chrono::seconds(timeToThink));
		

		// HUNGRY State

		stateData[id].state = State::HUNGRY;

		// Thread stuck in a loop while he can't eat
		bool canEat = false;
		while (!canEat)
		{
			if (forks[id].try_lock())
			{
				if (forks[(id + 1) % data.sageCount].try_lock())
				{
					canEat = true;
				}
				else
				{
					forks[id].unlock();
				}
			}
		}

		//EATING State

		stateData[id].state = State::EATING;
		int timeToEat = rand() % (data.maxEat + 1 - data.minEat) + data.minEat;
		currentEatingTime += timeToEat;
		stateData[id].timer = timeToEat;
		this_thread::sleep_for(std::chrono::seconds(timeToEat));

		forks[id].unlock();
		forks[(id + 1) % data.sageCount].unlock();
	}

	//SLEEPING State

	stateData[id].state = State::SLEEPING;
}

int main()
{
	GameData data;
	vector<thread> sages;
	vector<StateData> stateData;
	
	data.InitData();
	mutex* forks = new mutex[data.sageCount];

	// Fill StateData
	for (int i = 0; i < data.sageCount; i++)
	{
		stateData.push_back(StateData());
	}

	//Loop the process "number of days" times
	for (int day = 0; day < data.days; day++)
	{
		// Create Threads (One per sage)
		for (int i = 0; i < data.sageCount; i++)
		{
			sages.push_back(thread(DailyRoutine, i, ref(data), ref(stateData), ref(forks)));
		}

		// Create Print Thread ( Updates states and timers )
		thread printThread(PrintState, ref(data), ref(stateData), day+1);

		for (int i = 0; i < data.sageCount; i++)
		{
			sages[i].join();
		}
		printThread.join();

		sages.clear();
	}
	

	delete[] forks;
}