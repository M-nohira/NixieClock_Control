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
/// �����̃��W�b�N�v���Z�X
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

		if (logicState == false) printf("Started!! in %d\n\r", clock() - clockTime);	//�������\�������邽�߂ɓ˂����݂܂���

		logicState = true;
	} while (true);
	logicState = false;
}

/// <summary>
/// �\���p���W�b�N�v���Z�X
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

		if (displayState == false) printf("Started!! in %d\n\r", clock() - clockTime);	//�������\�������邽�߂ɓ˂����݂܂���

		displayState = true;
	} while (true);
	displayState = false;
}

/// <summary>
/// ���s���t���O��ݒ�
/// </summary>
/// <param name="flag">�t���O</param>
void Nixie::SetWorkerFlag(bool flag)
{
	workerEnableFlag = flag;
}

/// <summary>
/// HV�̐ڑ���̐ݒ�
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
/// GND�̐ڑ���̐ݒ�
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
/// �\���ǔԍ��ƕ\�������̏o�͂����肷��.
/// </summary>
/// <param name="num">�\���ǔԍ�/�\������</param>
/// <param name="isTube">trur=�\���ԍ�|false = �\������</param>
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
			digitalWrite(17, 1); return;	//LDP�\���@(��dot)
		case 11:
			digitalWrite(27, 1); return;	//RDP�\���@(�Edot)
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
/// �\���p�����s������Ƃɕ\������.
/// </summary>
void Nixie::DisplayUpdate()
{
	for (int cnt = 0; cnt < 8; cnt++)
	{

		//�\������j�L�V�[�ǂɒʓd
		SelectShowing(suborder[0][cnt], true);
		//�\�����镶����GND�̐ڑ�      
		SelectShowing(suborder[1][cnt], false);

		//�S�[�X�g���ۉ���̂��߂ɒ�������K�v�A��
		delayMicroseconds(NIXIE_ON_TIME);//ON�@����

		//�\�����O��p���ď���
		SelectShowing(999, true);
		SelectShowing(999, false);

		delayMicroseconds(NIXIE_OFF_TIME); //OFF ����
	}
}

/// <summary>
/// �\���p�����s���ҏW �v�C��
/// </summary>
/// <param name="quecount">��</param>
/// <param name="tubeNum">�\������j�L�V�[�ǂ̔ԍ�</param>
/// <param name="showNum">�\������ԍ�</param>
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

