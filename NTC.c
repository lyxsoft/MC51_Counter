/*

	(*) Read NTC temperature


		Connection:
		Vcc---R(4.7K)---NTC(10K)---GND
		              |
		            ADC P1.6

*/

#include "STC\STC15F104E.H"
#include <INTRINS.H>
#include "NTC.h"

sbit PIN_NTC = P1 ^6;		//P1.6
#define P1ASF_NTC 0x40	    //P1.6
#define NUM_NTC 0x6			//P1.6

#define ADC_POWER   0x80    //ADC电源控制位
#define ADC_SPEED3  0x60    //ADC转换速度，一次转换需要 90个时钟
#define ADC_SPEED2  0x40    //ADC转换速度，一次转换需要180个时钟
#define ADC_SPEED1  0x20    //ADC转换速度，一次转换需要360个时钟
#define ADC_SPEED0  0x00    //ADC转换速度，一次转换需要540个时钟
#define ADC_FLAG    0x10    //ADC完成标志位
#define ADC_START   0x08    //ADC启动控制位


static bit bGate = 0;

/*
	NTC Value & Temp
*/
#define ADC_81		253		//ADC = 253 at 81 deg
#define TABLE_COUNT 120		//-40 to 80 deg, had 120 numbers
unsigned char code nTempTable[] = {
	1,	// -40
	1,	// -39
	1,	// -38
	2,	// -37
	1,	// -36
	2,	// -35
	2,	// -34
	1,	// -33
	2,	// -32
	2,	// -31
	2,	// -30
	2,	// -29
	3,	// -28
	2,	// -27
	2,	// -26
	3,	// -25
	2,	// -24
	3,	// -23
	3,	// -22
	2,	// -21
	3,	// -20
	3,	// -19
	4,	// -18
	3,	// -17
	3,	// -16
	4,	// -15
	3,	// -14
	4,	// -13
	4,	// -12
	4,	// -11
	4,	// -10
	5,	// -9
	4,	// -8
	5,	// -7
	5,	// -6
	5,	// -5
	5,	// -4
	5,	// -3
	5,	// -2
	6,	// -1
	5,	// 0
	6,	// 1
	6,	// 2
	5,	// 3
	6,	// 4
	6,	// 5
	7,	// 6
	6,	// 7
	6,	// 8
	6,	// 9
	1,	// 10
	3,	// 11
	5,	// 12
	6,	// 13
	8,	// 14
	8,	// 15
	9,	// 16
	10,	// 17
	10,	// 18
	10,	// 19
	11,	// 20
	10,	// 21
	10,	// 22
	10,	// 23
	10,	// 24
	10,	// 25
	9,	// 26
	10,	// 27
	9,	// 28
	9,	// 29
	9,	// 30
	8,	// 31
	9,	// 32
	9,	// 33
	9,	// 34
	8,	// 35
	9,	// 36
	8,	// 37
	9,	// 38
	9,	// 39
	8,	// 40
	9,	// 41
	9,	// 42
	9,	// 43
	9,	// 44
	8,	// 45
	9,	// 46
	9,	// 47
	9,	// 48
	9,	// 49
	8,	// 50
	9,	// 51
	9,	// 52
	9,	// 53
	9,	// 54
	9,	// 55
	9,	// 56
	9,	// 57
	9,	// 58
	8,	// 59
	6,	// 60
	5,	// 61
	6,	// 62
	6,	// 63
	7,	// 64
	7,	// 65
	8,	// 66
	7,	// 67
	7,	// 68
	8,	// 69
	7,	// 70
	7,	// 71
	7,	// 72
	6,	// 73
	6,	// 74
	6,	// 75
	6,	// 76
	6,	// 77
	5,	// 78
	5,	// 79
	5	// 80
};


static bit bInited;

/*
	Init the ADC
*/
void InitADC(void)
{
    P1ASF = P1ASF_NTC;	//Open the ADC of the pin
    PIN_NTC = 0;        //Set to low to measure the pin
    ADC_RES = 0;        //Clear the result
	ADC_RESL = 0;
    ADC_CONTR = ADC_POWER | ADC_SPEED3 | NUM_NTC; //Power on, speed 3, pin #6

	bInited = 1;
}

/*
	Read the ADC value

	Return High 8bit only
*/
unsigned int GetADCResult (void)
//unsigned char Get_ADC_Result(void)
{
    ADC_RES = 0;        //Clear the result
	ADC_RESL = 0;

	if (!bInited)
		InitADC ();

    ADC_CONTR =ADC_POWER|ADC_SPEED3|NUM_NTC|ADC_START;//Power on, speed 3, pin #6, Start ADC

	//Wait for a while to start
    _nop_();    _nop_();
    _nop_();    _nop_();

    while (!(ADC_CONTR & ADC_FLAG));	//Waiting for ADC finished
    
	ADC_CONTR &= ~ADC_FLAG;				//Clear the flag
    
	return (((unsigned int)ADC_RES << 2) | ADC_RESL);
	//return ADC_RES;
}

/*
	Read the temperature

	Return temperature in deg, start from -40 deg. Max is 81 deg.
	for example: return 0 means -40deg, 40 means 0 deg, 61 means 21 deg.
	nLow return the 0.1deg
*/
unsigned char ReadTemperature (unsigned char *nLow)
{
	unsigned int nADC;
	unsigned char nStep;
	unsigned char nCount;

	nADC = GetADCResult ();
	nStep = ADC_81;		//Start from 81 deg
	nCount = TABLE_COUNT + 1;

	while (nADC >= nStep && nCount)
	{
		nADC -= nStep;
		nCount --;
		nStep = nTempTable [nCount];
	}
	
	*nLow = 0;
	if (nADC == 0)
		return (nCount);	
	if (nCount == 0)
		return (0);

	*nLow = 10 - nADC * 10 / nStep;
	if (*nLow >= 10)
	{
		*nLow = 0;
		return (nCount);
	}
	return (nCount - 1); 
}
