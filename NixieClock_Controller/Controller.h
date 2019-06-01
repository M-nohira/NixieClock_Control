#include <stdio.h>
#include <mutex>
#include <thread>
#include <wiringPi.h>
#include <string.h>
#include <time.h>


#define NIXIE_ON_TIME   950
#define NIXIE_OFF_TIME  250

class Nixie
{
public:
	Nixie();
	~Nixie();
	
	bool logicState = false;
	bool displayState = false;
	clock_t clockTime; //èàóùéûä‘åvë™ópÅ@ñ≥ë Ç∑Ç¨ÇÈÇØÇ«å„â˜ÇµÇƒÇ»Ç¢

	void DisplayUpdate();
	void DisplaySetting(int quecount, int tubeNum, int showNum);
	void LogicWorker();
	void DisplayWorker();
	void SetWorkerFlag(bool flag);

private:
	int order[2][8];
	int suborder[2][8];
	std::mutex mutex;
	bool workerEnableFlag = false;
	uint8_t lastTubeBcd = 0;
	uint8_t lastNumBcd = 0;

	void SetTube(uint_fast8_t bcd);
	void SetNum(uint8_t bcd);
	void SelectShowing(int num,bool isTube);
};

Nixie::Nixie()
{
	printf("Initializing RapberryPi GPIO\n\r");
	wiringPiSetupGpio();
	pinMode(12, OUTPUT);
	pinMode(16, OUTPUT);
	pinMode(20, OUTPUT);
	pinMode(21, OUTPUT);
	pinMode(23, OUTPUT);
	pinMode(24, OUTPUT);
	pinMode(25, OUTPUT);
	pinMode(8 , OUTPUT);
	pinMode(17, OUTPUT);
	pinMode(27, OUTPUT);
}

Nixie::~Nixie()
{
}