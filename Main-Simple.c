/*
	(*)	Clock

		LyxSoft 13.10.2015

*/

#include "STC\STC15F104E.H"
#include "DS1302.h"
#include "NTC.h"

/*                                      0    1	  2	   3	4	 5	  6	   7	8	 9    a    b    c    d    e    f*/

unsigned char code tblNumbers[]    ={0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90,0x88,0x83,0xC6,0xA1,0x86,0x8E};
unsigned char code tblNumbersRev[] ={0xC0,0xCF,0xA4,0x86,0x8B,0x92,0x90,0xC7,0x80,0x82,0x81,0x98,0xF0,0x8C,0xB0,0xB1};

/*                                      0    1	  2	   3	4	 5	  6	   7	8	 9    a    b    c    d    e    f   10   11   12*/
unsigned char code tblMonthDays[] = {0x00,0x31,0x28,0x31,0x30,0x31,0x30,0x31,0x31,0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x31,0x30,0x31};

#define MAKEBCD(nValue)	((((nValue) / 10) << 4) + ((nValue) % 10))
#define GETBCD(nBCD)    ((((nBCD) >> 4) * 10) + ((nBCD) & 0xF))

#define KEY_S1	P31
#define KEY_S2	P30
#define KEY_OK	P30

#define BEEP	P33

#define MODE_CLOCK		0
#define MODE_HOUR		1
#define MODE_MINUTE		2
#define MODE_HOUR24		3
#define MODE_MONTH		4
#define MODE_DAY		5
#define MODE_WEEK		6
#define MODE_YEAR		7
#define MODE_BEEP		8
#define MODE_MAX		8

#define SMODE_CLOCK				0
#define SMODE_TEMP				1
#define SMODE_DATE				2
#define SMODE_WEEK				3
#define SMODE_YEAR				4
#define SMODE_SECOND			5
#define SMODE_MS				6
#define SMODE_COUNTDOWN1		7
#define SMODE_COUNTDOWN2		8

#define SMODE_MAX				8

#define SMODE_COUNTSEC			10	//10 seconds return to SMODE_CLOCK

bit bBlockClock;

// Clock
static DS1302Time nCurrentTime; 
static unsigned char n100Ms, n10Ms, nMs, nLastSec;
bit bBeep;

// Temperature
static unsigned char nTemp, nTempL;

// Buttons
static bit bPressedS1, bPressedS2;
static unsigned char nMode, nSubMode, nSubModeTime, nCountDownSec, nCountDownMin, nCountDownBeep;

// Display
static unsigned char nLED, nData0, nData1, nData2, nData3;
static bit bDot, bSound, bShow0, bShow1, bShow2, bShow3;

void PreDisplay (void)
{
	unsigned char nDisplay;

	switch (nMode)
	{
	case MODE_CLOCK:
		bShow0 = 1;
		bShow1 = 1;
		bShow2 = 1;
		bShow3 = 1;
		nDisplay = nSubMode;
		break;

	case MODE_HOUR:
		bShow0 = 1;
		bShow1 = 1;
		bShow2 = bPressedS1 || bDot;
		bShow3 = bShow2;

		nDisplay = SMODE_CLOCK;
		break;
	case MODE_MINUTE:
		bShow0 = bPressedS1 || bDot;
		bShow1 = bShow0;
		bShow2 = 1;
		bShow3 = 1;

		nDisplay = SMODE_CLOCK;
		break;
	case MODE_MONTH:
		bShow0 = 1;
		bShow1 = 1;
		bShow2 = bPressedS1 || bDot;
		bShow3 = bShow2;

		nDisplay = SMODE_DATE;
		break;
	case MODE_DAY:
		bShow0 = bPressedS1 || bDot;
		bShow1 = bShow0;
		bShow2 = 1;
		bShow3 = 1;

		nDisplay = SMODE_DATE;
		break;
	case MODE_WEEK:
		bShow0 = bPressedS1 || bDot;
		bShow1 = bShow0;
		bShow2 = bShow0;
		bShow3 = bShow0;

		nDisplay = SMODE_WEEK;
		break;
	case MODE_YEAR:
		bShow0 = bPressedS1 || bDot;
		bShow1 = bShow0;
		bShow2 = bShow0;
		bShow3 = bShow0;

		nDisplay = SMODE_YEAR;
		break;
	case MODE_HOUR24:
		bShow0 = bDot;
		bShow1 = bDot;
		bShow2 = bDot;
		bShow3 = 0;
		
		if (nCurrentTime.Hour & 0x80)	// 12 HOUR
		{
			nData1 = tblNumbersRev [2];
			nData2 = tblNumbers    [1];
		}
		else
		{
			nData1 = tblNumbersRev [4];
			nData2 = tblNumbers    [2];
		}
		nData0 = 0x89; 	//'H'
		nData3 = 0xFF; 	//Hide
		return;
	case MODE_BEEP:
		bShow0 = bDot;
		bShow1 = bDot;
		bShow2 = bDot;
		bShow3 = bDot;
		
		if (bBeep)
		{
			nData0 = 0x8C; //'P'
			nData1 = 0xB0; //'E'
			nData2 = 0x86; //'E'
			nData3 = 0x83; //'b'
		}
		else
		{
			nData0 = 0xBF; //'-'
			nData1 = 0xBF;
			nData2 = 0xBF;
			nData3 = 0xBF;
		}
		return;
	default:
		bShow0 = bDot;
		bShow1 = bDot;
		bShow2 = bDot;
		bShow3 = bDot;

		nDisplay = 0xFF;  //NA
		break;		
	}

	switch (nDisplay)
	{
	case SMODE_CLOCK:
		nData1 = nCurrentTime.Minute;
		nData2 = nCurrentTime.Hour;

		nData0 = tblNumbers   [nData1 & 0x0F];
		nData1 = tblNumbersRev[nData1 >> 4] & (bDot ? 0x7F : 0xFF);

		if ((nData2 & 0xA0) == 0xA0)	//12 Hour Mode and PM
			nData0 &= 0x7F;
		if (nData2 & 0x80) //12 Hour mode
		{
			nData2 &= 0x1F;
			bShow3 &= nData2 >= 0x10;
		}
		nData3 = tblNumbers [nData2 >> 4];
		nData2 = tblNumbers   [nData2 & 0x0F] & (bDot ? 0x7F : 0xFF);
		break;
	case SMODE_TEMP:
		nData0 = 0x9C;	//'Deg
		nData1 = nTempL;
		nData2 = nTemp;

		if (nData2 >= 40 && nData1 < 10)	//Higher than 0
		{
			nData2 -= 40;
			bShow3 &= nData2 >= 0x10;

			nData1 = tblNumbersRev[nData1];
			nData3 = tblNumbers   [nData2 / 10];
			nData2 = tblNumbers   [nData2 % 10] & 0x7F;

		}
		else if (nData2 < 40)
		{
			nData2 = 40 - nData2;
			nData3 = 0xBF; // '-'
			nData1 = tblNumbers   [nData2 % 10];
			nData2 = tblNumbers   [nData2 / 10];
		}
		else
		{
			nData0 = tblNumbers   [8] & 0x7F;
			nData1 = nData0;
			nData2 = nData0;
			nData3 = nData0;
		}
		break;

	case SMODE_DATE:
		nData1 = nCurrentTime.Day;
		nData2 = nCurrentTime.Month;
		bShow3 &= nData2 >= 0x10;

		nData0 = tblNumbers   [nData1 & 0x0F];
		nData1 = tblNumbersRev[nData1 >> 4] & 0x7F;
		nData3 = tblNumbers   [nData2 >> 4];
		nData2 = tblNumbers   [nData2 & 0x0F];
		break;

	case SMODE_WEEK:
		switch (nCurrentTime.Week)
		{
		case 1:
			nData0 = 0xAB;	//n
			nData1 = 0x9c;	//o
			nData2 = 0xC8;	//N
			nData3 = 0xF9;	//|N = M
			break;
		case 2:
			nData0 = 0x86;	//E
			nData1 = 0xC8;	//U
			nData2 = 0xCE;	//T
			nData3 = 0xFE;	//T
			break;
		case 3:
			nData0 = 0xA1;	//d
			nData1 = 0xB0;	//E
			nData2 = 0xC1;	//U
			nData3 = 0xF9;	//|U=W
			break;
		case 4:
			nData0 = 0xE3;	//u
			nData1 = 0x99;	//h
			nData2 = 0xCE;	//T
			nData3 = 0xFE;	//T
			break;
		case 5:
			nData0 = 0xFB;	//i
			nData1 = 0xBD;	//r
			nData2 = 0x8E;	//F
			nData3 = 0xFF;	// 
			break;
		case 6:
			nData0 = 0x87;	//t
			nData1 = 0x81;	//A
			nData2 = 0x92;	//S
			nData3 = 0xFF;	// 
			break;
		case 7:
			nData0 = 0xAB;	//n
			nData1 = 0xDC;	//u
			nData2 = 0x92;	//S
			nData3 = 0xFF;	// 
			break;
		default:
			nData0 = tblNumbers   [8] & 0x7F;
			nData1 = nData0;
			nData2 = nData0;
			nData3 = nData0;
			break;
		}
		break;
	case SMODE_YEAR:
		nData1 = nCurrentTime.Year;
		nData0 = tblNumbers   [nData1 & 0x0F];
		nData1 = tblNumbersRev[nData1 >> 4];
		nData2 = tblNumbers   [0];
		nData3 = tblNumbers   [2];
		break;
	case SMODE_SECOND:
		nData1 = nCurrentTime.Second;
		nData2 = nCurrentTime.Minute;


		nData0 = tblNumbers   [nData1 & 0x0F];
		nData1 = tblNumbersRev[nData1 >> 4] & (bDot ? 0x7F : 0xFF);
		nData3 = tblNumbers   [nData2 >> 4];
		nData2 = tblNumbers   [nData2 & 0x0F] & (bDot ? 0x7F : 0xFF);
		break;
	case SMODE_MS:
		nData0 = tblNumbers[0];
		nData1 = tblNumbersRev[n100Ms];

		nData2 = nCurrentTime.Second;

		nData3 = tblNumbers   [nData2 >> 4];
		nData2 = tblNumbers   [nData2 & 0x0F] & 0x7F;
		break;
	case SMODE_COUNTDOWN1:
		nData0 = tblNumbers   [nCountDownSec % 10];
		nData1 = tblNumbersRev[nCountDownSec / 10] & 0x7F;
		nData2 = tblNumbers   [nCountDownMin % 10] & 0x7F;
		nData3 = tblNumbers   [nCountDownMin / 10];

		bShow3 = nCountDownMin >= 10;
		break;
	case SMODE_COUNTDOWN2:

		nData0 = tblNumbers   [nCountDownSec % 10];
		nData1 = tblNumbersRev[nCountDownSec / 10] & ((nCountDownBeep == 0 || bDot) ? 0x7F : 0xFF);
		nData2 = tblNumbers   [nCountDownMin % 10] & ((nCountDownBeep == 0 || bDot) ? 0x7F : 0xFF);
		nData3 = tblNumbers   [nCountDownMin / 10];

		bShow3 = nCountDownMin >= 10;
		break;
	default:
		nData0 = tblNumbers   [8] & 0x7F;
		nData1 = nData0;
		nData2 = nData0;
		nData3 = nData0;
		break;
	}
}

void OnTimer () interrupt 1
{
	
	// Timer reset
	TL0 = 0xCD;
	TH0 = 0xD4;			// 1ms

	nLastSec = nCurrentTime.Second;

	// Clock
	nMs ++;
	if (nMs >= 10)
	{
		nMs = 0;
		n10Ms ++;

		if (n10Ms >= 10)
		{
			n10Ms = 0;
			n100Ms ++;
			if (n100Ms == 5)
			{
				bDot = 1;
			}
			else if (n100Ms >= 10)
			{
				bDot = 0;
				n100Ms = 0;
			}

			// Close Tick Sound
			if (bSound)
			{
				if (bBeep && BEEP)
					BEEP = 0;
				else
				{
					BEEP = 1;
					bSound = 0;
				}
			}
		}

		// Read DS1302
		if (!bBlockClock)
			if (!DS1302ReadTime (&nCurrentTime))	//Read Time
				return;
	}

	if (nCurrentTime.Second != nLastSec)
	{
		n100Ms = 0;
		n10Ms = 0;
		nMs = 0;
		bDot = 0;

		if (nMode == MODE_CLOCK)
		{
			switch (nSubMode)
			{
			case SMODE_COUNTDOWN2:
				if (!nCountDownSec)
				{
					if (nCountDownMin)
					{
						nCountDownMin --;
						nCountDownSec = 59;
					}
					else if (nCountDownBeep)
					{
						bSound = 1;
						nCountDownBeep --;
					}
				}
				else
					nCountDownSec --;
				break;
			case SMODE_TEMP:
				if (nSubModeTime)
				{
					nTemp = ReadTemperature (&nTempL);
					nSubModeTime --;
				}
				else
					nSubMode = SMODE_CLOCK;
				break;
			default:
				break;
			}
		}
	}

	// Display
	if (nLED == 0)
		PreDisplay ();

	switch (nLED)
	{
	case 0:
		P34 = 1;
		if (bShow0)
		{
			P2 = nData0;
			P37 = 0; //Lighton LED
		}
		nLED ++;
		break;
	case 1:
		P37 = 1;
		if (bShow1)
		{
			P2 = nData1;
			P36 = 0;
		}
		nLED ++;
		break;
	case 2:
		P36 = 1;
		if (bShow2)
		{
			P2 = nData2;
			P35 = 0;
		}
		nLED ++;
		break;
	case 3:
		P35 = 1;
		if (bShow3)
		{
			P2 = nData3;
			P34 = 0;
		}
		nLED = 0;
		break;
	}
}


void Delay10ms()
{
	unsigned char n10MsSave, nMsSave;

	n10MsSave = n10Ms;
	nMsSave = nMs;
	while (n10Ms == n10MsSave);
	n10MsSave = n10Ms;
	while (nMsSave != nMs && n10Ms == n10MsSave);
}

void HourInc ()
{
	unsigned char nHour;

	nHour = nCurrentTime.Hour;

	if (nHour & 0x80) // 12 Hour mode
	{
		if ((nHour & 0xA0) == 0xA0)	//PM
		{
			nHour = GETBCD (nHour & 0x1F);
			if (nHour != 12) //12 PM means 12 aclock
				nHour += 12;
		}
		else					   //AM
		{
			nHour = GETBCD (nHour & 0x1F);
			if (nHour == 12)			//12 AM means 0 aclock
				nHour = 0;
		}

		if (++nHour > 23)
			nCurrentTime.Hour = 0x92;		//12AM
		else if (nHour >= 12)				//PM
		{
			nHour -= 12;
			if (nHour == 0)
				nCurrentTime.Hour = 0xB2;	//12 PM
			else
				nCurrentTime.Hour = 0xA0 | MAKEBCD (nHour);
		}
		else
			nCurrentTime.Hour = 0x80 | MAKEBCD (nHour);
	}
	else
	{
		nHour = GETBCD (nHour);
		if (++nHour > 23)
			nHour = 0;
		nCurrentTime.Hour = MAKEBCD (nHour);
	}
}

void HourDec ()
{
	unsigned char nHour;

	nHour = nCurrentTime.Hour;

	if (nHour & 0x80) // 12 Hour mode
	{
		if ((nHour & 0xA0) == 0xA0)	//PM
		{
			nHour = GETBCD (nHour & 0x1F);
			if (nHour != 12) //12 PM means 12 aclock
				nHour += 12;
		}
		else					   //AM
		{
			nHour = GETBCD (nHour & 0x1F);
			if (nHour == 12)			//12 AM means 0 aclock
				nHour = 0;
		}

		if (nHour > 0)
			nHour --;
		else
			nHour = 23;

		if (nHour == 0)
			nCurrentTime.Hour = 0x92;		//12AM
		else if (nHour >= 12)				//PM
		{
			nHour -= 12;
			if (nHour == 0)
				nCurrentTime.Hour = 0xB2;	//12 PM
			else
				nCurrentTime.Hour = 0xA0 | MAKEBCD (nHour);
		}
		else
			nCurrentTime.Hour = 0x80 | MAKEBCD (nHour);
	}
	else
	{
		nHour = GETBCD (nHour);

		if (nHour > 0)
			nHour --;
		else
			nHour = 23;
		nCurrentTime.Hour = MAKEBCD (nHour);
	}
}

unsigned char BCDInc (unsigned char nBCD, unsigned char nBCDLimit, unsigned char nBCDLow)
{
	if (nBCD >= nBCDLimit || nBCD == 0x99)
		return nBCDLow;

	if ((nBCD & 0xF) == 9)
		return (nBCD & 0xF0) + 0x10;
	return (nBCD + 1);
}

unsigned char BCDDec (unsigned char nBCD, unsigned char nBCDLimit, unsigned char nBCDLow)
{
	if (nBCD <= nBCDLow || nBCD == 0)
		return nBCDLimit;

	if ((nBCD & 0xF) == 0)
		return nBCD - 0x7; //0x10 + 0x9
	return (nBCD - 1);
}

unsigned char BCDHour24 (unsigned char nBCD)
{
	if ((nBCD & 0xA0) == 0xA0)	//PM
	{
		nBCD &= 0x1F;
		nBCD = GETBCD (nBCD);
		if (nBCD != 12) //12 PM means 12 aclock
			nBCD += 12;
		return (MAKEBCD (nBCD));
	}
	else						//AM
	{
		nBCD &= 0x1F;
		if (nBCD == 0x12)		//12 AM means 0 aclock
			return (0);
		return (nBCD);
	}
}

unsigned char BCDHour12 (unsigned char nBCD)
{
	if (nBCD >= 0x12)			//PM
	{
		nBCD = GETBCD (nBCD) - 12;
		if (nBCD == 0)
			return (0xB2);		//12 PM
		else
			return (0xA0 | MAKEBCD (nBCD));
	}
	else if (nBCD == 0)
		return (0x92);			// 12 AM
	else
		return (0x80 | nBCD);
}

void AdjustTime (bit bIncrease)
{
	bBlockClock = 1;

	switch (nMode)
	{
	case MODE_HOUR:
		if (bIncrease)
			HourInc ();
		else
			HourDec ();
		break;
	case MODE_MINUTE:
		if (bIncrease)
			nCurrentTime.Minute = BCDInc (nCurrentTime.Minute, 0x59, 0);
		else
			nCurrentTime.Minute = BCDDec (nCurrentTime.Minute, 0x59, 0);
		nCurrentTime.Second = 0;
		break;
	case MODE_MONTH:
		if (bIncrease)
			nCurrentTime.Month = BCDInc (nCurrentTime.Month, 0x12, 1);
		else
			nCurrentTime.Month = BCDDec (nCurrentTime.Month, 0x12, 1);
		break;
	case MODE_DAY:
		if (bIncrease)
			nCurrentTime.Day = BCDInc (nCurrentTime.Day, tblMonthDays[nCurrentTime.Month] + (nCurrentTime.Month == 2 && (GETBCD(nCurrentTime.Year) % 4) == 0), 1);
		else
			nCurrentTime.Day = BCDDec (nCurrentTime.Day, tblMonthDays[nCurrentTime.Month] + (nCurrentTime.Month == 2 && (GETBCD(nCurrentTime.Year) % 4) == 0), 1);
		break;
	case MODE_WEEK:
		if (bIncrease)
			nCurrentTime.Week = BCDInc (nCurrentTime.Week, 7, 1);
		else
			nCurrentTime.Week = BCDDec (nCurrentTime.Week, 7, 1);
		break;
	case MODE_YEAR:
		if (bIncrease)
			nCurrentTime.Year = BCDInc (nCurrentTime.Year, 0x99, 0);
		else
			nCurrentTime.Year = BCDDec (nCurrentTime.Year, 0x99, 0);
		break;
	case MODE_HOUR24:
		if (nCurrentTime.Hour & 0x80)	// 12 HOUR
			nCurrentTime.Hour = BCDHour24 (nCurrentTime.Hour);
		else
			nCurrentTime.Hour = BCDHour12 (nCurrentTime.Hour);
		break;
	case MODE_BEEP:
		bBeep = !bBeep;
		DS1302WriteMyData (bBeep);
		bBlockClock = 0;
		return;
	default:
		bBlockClock = 0;
		return;
	}

	DS1302WriteTime (&nCurrentTime);//Write Time
	n100Ms = 0;
	n10Ms = 0;
	nMs = 0;
	bBlockClock = 0;
}

void InitTimer ()
{
	unsigned char nMyData;

	bBlockClock = 0;

	bDot = 0;
	n100Ms = 0;
	n10Ms = 0;
	nMs = 0;
	nLED = 0;

	nMyData = DS1302ReadMyData ();
	bBeep = nMyData & 0x1;

	ClockInit ();

	AUXR |= 0x80;		// 1T mode
	TL0 = 0xCD;
	TH0 = 0xD4;			// 1ms
	TMOD = 1;			// Timer Mode
	TCON = 0x10;		// Timer
	IE = 0x82;			// INT
}

void main()
{
	unsigned char nPressedSec, nPressedMs, nDiffMs, nLimitMs;
	bit bIncrease;

	nTemp = ReadTemperature (&nTempL);
	InitTimer ();

	BEEP = 1;
	nMode = 0;
	bPressedS1 = 0;
	bPressedS2 = 0;
	bIncrease = 1;


	while (1)
	{
		if (!KEY_S1)
		{
			Delay10ms ();
			if (!KEY_S1)
			{
				if (nMode == MODE_CLOCK)
				{
					while (!KEY_S1)
						nSubModeTime = SMODE_COUNTSEC;
					if (++nSubMode > SMODE_MAX)
						nSubMode = 0;
					if (nSubMode == SMODE_COUNTDOWN1)
					{
						nCountDownMin = 1;
						nCountDownSec = 0;
						nCountDownBeep = 3;
					}
					bSound = 1;
					nSubModeTime = SMODE_COUNTSEC;
				}
				else
				{
					nLimitMs = 20;  //1.5sec
					nPressedSec = nCurrentTime.Second;
					nPressedMs = n100Ms;
					bPressedS1 = 0;
					while (!KEY_S1)
					{				
						if (!KEY_OK)	// Press OK & S1
						{
							while (!KEY_OK || !KEY_S1);	//Until all released
							bIncrease = !bIncrease;
							break;
						}

						nDiffMs = (((nCurrentTime.Second < nPressedSec) ? 60 : 0) + nCurrentTime.Second - nPressedSec) * 10 + ((nCurrentTime.Second == nPressedSec && n100Ms < nPressedMs) ? 10 : 0) + n100Ms - nPressedMs;
						//nDiffMs = 0.1sec
	
						if (nDiffMs > nLimitMs)
						{
							AdjustTime (bIncrease);
							bPressedS1 = 1;
							nPressedSec = nCurrentTime.Second;
							nPressedMs = n100Ms;
							nLimitMs = 5;  //0.3 sec
							bSound = 1;
						}
					}
					if (!bPressedS1) //Single Click
					{
						AdjustTime (bIncrease);
						bSound = 1;
					}
					else
						bPressedS1 = 0;
				}
			}
		}
		if (!KEY_OK)
		{
			Delay10ms ();
			if (!KEY_OK)
			{
				if (nMode != MODE_CLOCK)
				{
					while (!KEY_OK);
					// Press S2
					if (++nMode > MODE_MAX)
					{
						nMode = 0;
						nSubMode = 0;
					}
					bIncrease = 1;	//Reset to Increasing
					bSound = 1;
				}
				else
				{
					while (!KEY_OK)
					{
						if (!KEY_S1) // Press S1 & OK
						{
							while (!KEY_OK || !KEY_S1);
							nMode = 1;
							bIncrease = 1;	//Reset to Increasing
							bSound = 1;
							break;
						}
					}
				}
			}
		}
	}
}