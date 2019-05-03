/*
	(*)	Clock

		LyxSoft 13.10.2015

*/

#include "STC\STC15F104E.H"
//#include "DS1302.h"

#define __REVERSE__

/*                                      0    1	  2	   3	4	 5	  6	   7	8	 9    
																						
                                        0    1    2    3    4    5    6    7    8    9   10*/

unsigned char code tblNumbers[]    ={0xC0,0xF9,0xA4,0xB0,0x99,0x92,0x82,0xF8,0x80,0x90,0xBF};
unsigned char code tblNumbersRev[] ={0xC0,0xCF,0xA4,0x86,0x8B,0x92,0x90,0xC7,0x80,0x82,0xBF};

#define MAKEBCD(nValue)	((((nValue) / 10) << 4) + ((nValue) % 10))
#define GETBCD(nBCD)    ((((nBCD) >> 4) * 10) + ((nBCD) & 0xF))

//50 us	 @ 33Mhz System Clock
#define TIME_LOW    0x8E
#define TIME_HIGH   0xF9

#define KEY_START	P30
#define KEY_EC11_D	P31
#define KEY_EC11_A	P15
#define KEY_EC11_B	P14

#define BEEP	P33


#define DotOn()		{nData1 &= 0x7F; nData2 &= 0x7F;}
#define DotOff()	{nData1 |= 0x80; nData2 |= 0x80;}

#define COUNTDOWN_BEEP 3
#define COUNTDOWN_FLASH 3

bit bBlockClock;

// Clock
//static DS1302Time nCurrentTime; 
static unsigned char n100Ms, nMs;


// Counting
static bit bCount;
static unsigned char nCountDownSec, nCountDownMin, nLastMin, nLastSec;

// Display
static unsigned char nLED, nData0, nData1, nData2, nData3, nFocusFlags;
static bit bDot, bSound;


#ifdef __REVERSE__
void DisplayDecNumL (unsigned char nNumLow)
{
	nData3 = tblNumbersRev [nNumLow & 0xF];
	nData2 = tblNumbersRev [nNumLow >> 4];
}

void DisplayDecNumH (unsigned char nNumHigh)
{
	nData1 = tblNumbers [nNumHigh & 0xF];
	nData0 = tblNumbersRev [nNumHigh >> 4];
}

void DisplayDecNum (unsigned char nNumLow, unsigned char nNumHigh)
{
	nData3 = tblNumbersRev [nNumLow & 0xF];
	nData2 = tblNumbersRev [nNumLow >> 4];
	nData1 = tblNumbers [nNumHigh & 0xF];
	nData0 = tblNumbersRev [nNumHigh >> 4];
}
#else
void DisplayDecNumL (unsigned char nNumLow)
{
	nData0 = tblNumbers [nNumLow & 0xF];
	nData1 = tblNumbersRev [nNumLow >> 4];
}

void DisplayDecNumH (unsigned char nNumHigh)
{
	nData2 = tblNumbers [nNumHigh & 0xF];
	nData3 = tblNumbers [nNumHigh >> 4];
}

void DisplayDecNum (unsigned char nNumLow, unsigned char nNumHigh)
{
	nData0 = tblNumbers [nNumLow & 0xF];
	nData1 = tblNumbersRev [nNumLow >> 4];
	nData2 = tblNumbers [nNumHigh & 0xF];
	nData3 = tblNumbers [nNumHigh >> 4];
}
#endif

void OnTimer () interrupt 1
{
	//static unsigned char nLastSec;
	static unsigned char nShowCount = 0, nUs = 0;
	static unsigned char nBeepCount = 0, nFlashCount = 0;
		

	if (bBlockClock)
		return;

	bBlockClock = 1;
	// Clock

	nUs ++;
	if (nUs >= 20)
	{
		nUs = 0;

		if (nBeepCount)
		{
			nBeepCount --;
			if (!nBeepCount)
			{
				bSound = 0;
				BEEP = 1;
			}
		}
		else if (bSound)
		{
			nBeepCount = 100;
			BEEP = 0;
		}

		nMs ++;
		if (nMs >= 100)
		{
			nMs = 0;

			n100Ms ++;
			if (n100Ms == 5)
			{
				bDot = 0;
				if (bCount)
					DotOff ();
			}
			else if (n100Ms >= 10)
			{
				// Time Changed
				n100Ms = 0;
				bDot = 1;
				if (bCount)
				{
					if (nFlashCount)
					{
						nFlashCount --;
						if (!nFlashCount)
						{
							bCount = 0;
							nCountDownMin = nLastMin;
							nCountDownSec = nLastSec;
							DisplayDecNum (MAKEBCD(nCountDownSec), MAKEBCD(nCountDownMin));
						}
					}
					else if (nCountDownMin)
					{
						if (!nCountDownSec)
						{
							nCountDownMin --;
							nCountDownSec = 59;
							DisplayDecNum (MAKEBCD(nCountDownSec), MAKEBCD(nCountDownMin));
						}
						else
						{
							nCountDownSec --;
							DisplayDecNumL (MAKEBCD(nCountDownSec));
						}
					}
					else if (nCountDownSec)
					{
						nCountDownSec --;
						DisplayDecNumL (MAKEBCD(nCountDownSec));

						if (nCountDownSec <= COUNTDOWN_BEEP)
							bSound = 1;
					}
					else
					{
						nFlashCount = COUNTDOWN_FLASH - 1;
						DisplayDecNum (0xAA, 0xAA);
					}
				}				
				DotOn ();
			}
			//PreDisplay ();
		}	// Per 100 MS
	}

	if (nUs == 0 || nUs == 10)
	{
		// Display

		switch (nLED)
		{
		case 0:
			P34 = 1;
			if (nShowCount > 3 || (nFocusFlags & 0x1))
			{
				P2 = nData0;
				P37 = 0; //Lighton LED
			}
			nLED ++;
			break;
		case 1:
			P37 = 1;
			if (nShowCount > 3 || (nFocusFlags & 0x2))
			{
				P2 = nData1;
				P36 = 0;
			}
			nLED ++;
			break;
		case 2:
			P36 = 1;
			if (nShowCount > 3 || (nFocusFlags & 0x4))
			{
				P2 = nData2;
				P35 = 0;
			}
			nLED ++;
			break;
		case 3:
			P35 = 1;
			if (nShowCount > 3 || (nFocusFlags & 0x8))
			{
				P2 = nData3;
				P34 = 0;
			}
			nLED = 0;
			break;
		}
		nShowCount ++;
		if (nShowCount >= 8)
			nShowCount = 0;
	}

	bBlockClock = 0;
}

void Delay100ms()
{
	unsigned char n100MsSave, nMsSave;

	n100MsSave = n100Ms;
	nMsSave = nMs;

	while (n100Ms == n100MsSave);
	while (nMsSave != nMs);
}

void Delay1ms()
{
	unsigned char nMsSave;

	nMsSave = nMs;

	while (nMsSave == nMs);
}


void InitTimer ()
{
	bBlockClock = 1;

	bDot = 1;
	n100Ms = 0;
	nMs = 0;
	nLED = 0;

	nCountDownSec = 0;
	nCountDownMin = 0;
	nLastSec = 0;
	nLastMin = 0;
	bCount = 0;
	DisplayDecNum (0, 0);
	DotOn ();
	//ClockInit ();

	bBlockClock = 0;

	AUXR |= 0x80;		// 1T mode
	TMOD = 0;			// Timer Mode
	TL0 = TIME_LOW;
	TH0 = TIME_HIGH;			// 1ms
	TCON = 0x10;		// Timer
	IE = 0x82;			// INT

}

void main()
{
	// Buttons
	bit bStatusEC11A = 1, bStatusEC11B = 1, bPause = 0;

	InitTimer ();

	bSound = 1;
	nFocusFlags = 0xF;


	// Init Inputs
	// P1M1 = 1; P1M0 = 0; Only Input
	// P1M1 = 0; P1M0 = 0; I/O
	// P1M1 = 0; P1M0 = 1; 20ma Output
	// P1M1 = 1; P1M0 = 1; Open Drain

	P1M1 = 0xCF; //11001111
	P1M0 = 0x0;  //00000000
	//P3M1 = 0xFC;  //00000011
	//P3M0 = 0x0;	 //00000000

	while (1)
	{
		if (KEY_EC11_A && KEY_EC11_B)
		{
			if (!bCount)
			{
				if (!bStatusEC11A)
				{
					// Decrease
					if (nFocusFlags == 0xF)
					{
						if (nCountDownMin)
						{
							nCountDownMin --;
							DisplayDecNumH (MAKEBCD(nCountDownMin));
						}
					}
					else
					{
						if (nCountDownSec)
						{
							nCountDownSec --;
							DisplayDecNumL (MAKEBCD(nCountDownSec));
						}
					}
				}
				else if (!bStatusEC11B)
				{
					// Increase
					if (nFocusFlags == 0xF)
					{
						if (nCountDownMin < 99)
						{
							nCountDownMin ++;
							DisplayDecNumH (MAKEBCD(nCountDownMin));
						}
					}
					else
					{
						if (nCountDownSec < 59)
						{
							nCountDownSec ++;
							DisplayDecNumL (MAKEBCD(nCountDownSec));
						}
					}
				}
				DotOn ();
			}
			bStatusEC11A = 1;
			bStatusEC11B = 1;
		}
		else
		{
			bStatusEC11A = KEY_EC11_A;
			bStatusEC11B = KEY_EC11_B;
		}


 		if (!KEY_START)
		{
			Delay100ms ();
			if (!KEY_START)
			{
				while (!KEY_START);

				// On Press Start
				if (bCount)
				{
					//Stop
					bCount = 0;
					nCountDownMin = nLastMin;
					nCountDownSec = nLastSec;
					DisplayDecNum (MAKEBCD(nCountDownSec), MAKEBCD(nCountDownMin));
					DotOn ();
				}
				else if (nCountDownMin || nCountDownSec)
				{
					//Start
					if (!bPause)
					{
						nLastMin = nCountDownMin;
						nLastSec =  nCountDownSec;
					}
					bPause = 0;
					nMs = 0;
					n100Ms = 0;
					bCount = 1;
					nFocusFlags = 0xF;
				}
				else
				{
					// Error
					bSound = 1;
				}
			}
		}

		if (!KEY_EC11_D)
		{
			Delay100ms ();
			if (!KEY_EC11_D)
			{
				while (!KEY_EC11_D);

				if (bCount)
				{
					//Pause
					bCount = 0;
					bPause = 1;
					DotOn ();
				}
				else if (nFocusFlags == 0xF)
					nFocusFlags = 0xC; //Go in Sec
				else
					nFocusFlags = 0xF;
			}
		}

	}
}