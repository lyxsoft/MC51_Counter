/*

	(*) Read NTC temperature




*/

/*
	Read the temperature

	Return temperature in deg, start from -40 deg. Max is 81 deg.
	for example: return 0 means -40deg, 40 means 0 deg, 61 means 21 deg.
	nLow return the 0.1deg
*/
unsigned char ReadTemperature (unsigned char *);
