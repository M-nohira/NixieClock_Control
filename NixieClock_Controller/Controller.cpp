#include "Controller.h"
#include <wiringPi.h>

int main(void)
{
	//Nixie nixie;
	//nixie.StartProcess();
	Nixie nixie;

	printf("Starting LogicWoker Process...");
	nixie.clockTime = clock();
	std::thread th_logic(&Nixie::LogicWorker, &nixie);
	while (nixie.logicState == false);
	printf("Starting DisplayWoker Process...");
	nixie.clockTime = clock();
	std::thread th_display(&Nixie::DisplayWorker, &nixie);
	
	//abc
	//abc

	th_logic.join();
	th_display.join();

	return 0;
}



void Nixie::LogicWorker()
{
	
	do
	{
		time_t t_time;
		time(&t_time);
		tm tms;
		tms = *localtime(&t_time);
		int hh[8] = { tms.tm_hour / 10,tms.tm_hour % 10,tms.tm_min / 10 ,tms.tm_min % 10, tms.tm_sec / 10, tms.tm_sec % 10 };
		//printf("%d%d:%d%d:%d%d \n\r", tms.tm_hour / 10, tms.tm_hour % 10, tms.tm_min / 10, tms.tm_min % 10, tms.tm_sec / 10, tms.tm_sec % 10);
		
		for (int cnt = 0; cnt < 8; cnt++)
		{
			DisplaySetting(cnt, cnt + 1, hh[cnt]);
		}

		if (logicState == false) printf("Started!! in %d\n\r", clock() - clockTime);	//無理やり表示させるために突っ込みました

		logicState = true;
	} while (true);
	logicState = false;
}

void Nixie::DisplayWorker()
{
	
	do
	{
		if (true == mutex.try_lock())
		{
			memcpy(suborder, order, sizeof(suborder));
			mutex.unlock();
		}

		DisplayUpdate();

		if(displayState == false) printf("Started!! in %d\n\r", clock() - clockTime);	//無理やり表示させるために突っ込みました

		displayState = true;
	} while (true);
	displayState = false;
}

/// <summary>
/// 実行時フラグを設定
/// </summary>
/// <param name="flag"></param>
void Nixie::SetWorkerFlag(bool flag)
{
	workerEnableFlag = flag;
}

void Nixie::SetTube(int bcd1, int bcd2, int bcd3, int bcd4)
{
	digitalWrite(12, bcd1);
	digitalWrite(16, bcd2);
	digitalWrite(20, bcd3);
	digitalWrite(21, bcd4);
}

void Nixie::SetNum(int bcd1, int bcd2, int bcd3, int bcd4)
{
	digitalWrite(23, bcd1);
	digitalWrite(24, bcd2);
	digitalWrite(25, bcd3);
	digitalWrite(8, bcd4);
}

void Nixie::SelectShowing(int num, bool isTube)
{
	if (isTube == false)
	{
		switch (num)
		{
		case 0:
			SetNum(0, 1, 1, 0); return;
		case 1:
			SetNum(1, 0, 0, 1); return;
		case 2:
			SetNum(1, 0, 0, 0); return;
		case 3:
			SetNum(0, 1, 1, 1); return;
		case 4:
			SetNum(0, 0, 0, 0); return;
		case 5:
			SetNum(0, 0, 0, 1); return;
		case 6:
			SetNum(0, 0, 1, 0); return;
		case 7:
			SetNum(0, 0, 1, 1); return;
		case 8:
			SetNum(0, 1, 0, 0); return;
		case 9:
			SetNum(0, 1, 0, 1); return;
		case 10:
			digitalWrite(17, 1); return;	//LDP表示　(左dot)
		case 11:
			digitalWrite(27, 1); return;	//RDP表示　(右dot)
		default:
			SetNum(1, 1, 1, 1);
			digitalWrite(27, 0); return;
			digitalWrite(17, 0); return;
		}
		return;
	}

	switch (num)
	{
	case 0:
		SetTube(1, 1, 1, 1); return;
	case 1:
		SetTube(0, 0, 1, 1); return;
	case 2:
		SetTube(0, 0, 0, 1); return;
	case 3:
		SetTube(0, 0, 1, 0); return;
	case 4:
		SetTube(0, 0, 0, 0); return;
	case 5:
		SetTube(0, 1, 1, 1); return;
	case 6:
		SetTube(0, 1, 0, 1); return;
	case 7: 
		SetTube(0, 1, 1, 0); return;
	case 8:
		SetTube(0, 1, 0, 0); return;
	default:
		SetTube(1, 1, 1, 1);
	}
	return;
}

/// <summary>
/// 表示用注文行列をもとに表示する.
/// </summary>
void Nixie::DisplayUpdate()
{
	for (int cnt = 0; cnt < 8; cnt++)
	{
		//表示するニキシー管に通電
		SelectShowing(suborder[0][cnt], true);
		//表示する文字をGNDの接続      
		SelectShowing(suborder[1][cnt], false);

		//ゴースト現象回避のために調整する必要アリ
		delayMicroseconds(NIXIE_ON_TIME);//ON　時間

		//表示を例外を用いて消す
		SelectShowing(999, true);
		SelectShowing(999, false);

		delayMicroseconds(NIXIE_OFF_TIME); //OFF 時間
	}
}

/// <summary>
/// 表示用注文行列を編集 要修正
/// </summary>
/// <param name="quecount">列数</param>
/// <param name="tubeNum">表示するニキシー管の番号</param>
/// <param name="showNum">表示する番号</param>
void Nixie::DisplaySetting(int quecount, int tubeNum, int showNum)
{
	int draft[2][8];

	mutex.lock();
	memcpy(draft, order, sizeof(draft));
	mutex.unlock();

	draft[0][quecount] = tubeNum;
	draft[1][quecount] = showNum;

	mutex.lock();
	memcpy(order, draft, sizeof(draft));
	mutex.unlock();
}

