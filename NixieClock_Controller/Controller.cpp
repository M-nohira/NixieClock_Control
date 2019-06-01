#include "Controller.h"


int main(void)
{
	Nixie nixie;

	printf("Starting LogicWoker Process...");
	nixie.clockTime = clock();
	std::thread th_logic(&Nixie::LogicWorker, &nixie);
	while (nixie.logicState == false);
	printf("Starting DisplayWoker Process...");
	nixie.clockTime = clock();
	std::thread th_display(&Nixie::DisplayWorker, &nixie);
	th_logic.join();
	th_display.join();

	return 0;
}


/// <summary>
/// 裏側のロジックプロセス
/// </summary>
void Nixie::LogicWorker()
{
	do
	{
		time_t t_time;
		time(&t_time);
		tm tms;
		tms = *localtime(&t_time);
		int hh[8] = { tms.tm_hour / 10 ,tms.tm_hour % 10 ,11 ,tms.tm_min / 10 ,tms.tm_min % 10 ,11 ,tms.tm_sec / 10 ,tms.tm_sec % 10 };

		for (int cnt = 0; cnt < 8; cnt++)
		{
			DisplaySetting(cnt, cnt + 1, hh[cnt]);
		}

		if (logicState == false) printf("Started!! in %d\n\r", clock() - clockTime);	//無理やり表示させるために突っ込みました

		logicState = true;
	} while (true);
	logicState = false;
}

/// <summary>
/// 表示用ロジックプロセス
/// </summary>
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

		if (displayState == false) printf("Started!! in %d\n\r", clock() - clockTime);	//無理やり表示させるために突っ込みました

		displayState = true;
	} while (true);
	displayState = false;
}

/// <summary>
/// 実行時フラグを設定
/// </summary>
/// <param name="flag">フラグ</param>
void Nixie::SetWorkerFlag(bool flag)
{
	workerEnableFlag = flag;
}

/// <summary>
/// HVの接続先の設定
/// </summary>
/// <param name="bcd">0b0000[Gpio_12][Gpio_16][Gpio_20][Gpio_21]</param>
void Nixie::SetTube(uint_fast8_t bcd)
{
	int gpio_tube[4] = { 12, 16, 20, 21 };
	uint8_t diff = bcd ^ lastTubeBcd;
	for (int cnt = 0; cnt < 4; cnt++)
	{
		if (!(diff >> cnt & 0b0001)) continue;
		digitalWrite(gpio_tube[cnt], (bcd>>cnt) & 0b0001);
	}
	lastTubeBcd = bcd;
}

/// <summary>
/// GNDの接続先の設定
/// </summary>
/// <param name="bcd">0b0000[Gpio_23][Gpio_24][Gpio_25][Gpio_8]</param>
void Nixie::SetNum(uint8_t bcd)
{
	int gpio_Num[4] = { 23,24,25,8 };
	uint diff = bcd ^ lastNumBcd;
	for (int cnt = 0; cnt < 4; cnt++)
	{
		if (!(diff >> cnt & 0b0001)) continue;
		digitalWrite(gpio_Num[cnt], (bcd >> cnt) & 0b0001);
	}
	lastNumBcd = bcd;
}

/// <summary>
/// 表示管番号と表示数字の出力を決定する.
/// </summary>
/// <param name="num">表示管番号/表示数字</param>
/// <param name="isTube">trur=表示番号|false = 表示数字</param>
void Nixie::SelectShowing(int num, bool isTube)
{
	if (isTube == false)
	{
		switch (num)
		{
		case 0:
			SetNum(0b0110); return;
		case 1:
			SetNum(0b1001); return;
		case 2:
			SetNum(0b1000); return;
		case 3:
			SetNum(0b0111); return;
		case 4:
			SetNum(0x0); return;
		case 5:
			SetNum(0x1); return;
		case 6:
			SetNum(0b0010); return;
		case 7:
			SetNum(0b0011); return;
		case 8:
			SetNum(0b0100); return;
		case 9:
			SetNum(0b0101); return;
		case 10:
			digitalWrite(17, 1); return;	//LDP表示　(左dot)
		case 11:
			digitalWrite(27, 1); return;	//RDP表示　(右dot)
		default:
			SetNum(0xf);
			digitalWrite(27, 0); return;
			digitalWrite(17, 0); return;
		}
		return;
	}

	switch (num)
	{
	case 0:
		SetTube(0xf); return;
	case 1:
		SetTube(0b0011); return;
	case 2:
		SetTube(0b0001); return;
	case 3:
		SetTube(0b0010); return;
	case 4:
		SetTube(0x0); return;
	case 5:
		SetTube(0b0111); return;
	case 6:
		SetTube(0b0101); return;
	case 7:
		SetTube(0b0110); return;
	case 8:
		SetTube(0b0100); return;
	default:
		SetTube(0xf);
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

