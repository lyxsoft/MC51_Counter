/*

	(*) DS1302 Interface



*/

//-------------------------------------³£Êýºê---------------------------------// 

#define DS1302_SECOND_WRITE	0x80 //Ð´Ê±ÖÓÐ¾Æ¬µÄ¼Ä´æÆ÷Î»ÖÃ 
#define DS1302_MINUTE_WRITE 0x82 
#define DS1302_HOUR_WRITE	0x84 
#define DS1302_WEEK_WRITE	0x8A 
#define DS1302_DAY_WRITE	0x86 
#define DS1302_MONTH_WRITE	0x88 
#define DS1302_YEAR_WRITE	0x8C 
#define DS1302_WP			0x8E
#define DS1302_CLOCK_WRITE	0xBE


#define DS1302_SECOND_READ	0x81 //¶ÁÊ±ÖÓÐ¾Æ¬µÄ¼Ä´æÆ÷Î»ÖÃ 
#define DS1302_MINUTE_READ	0x83 
#define DS1302_HOUR_READ	0x85 
#define DS1302_WEEK_READ	0x8B 
#define DS1302_DAY_READ		0x87 
#define DS1302_MONTH_READ	0x89 
#define DS1302_YEAR_READ	0x8D 
#define DS1302_WP_READ		0x8F
#define DS1302_CLOCK_READ	0xBF

#define DS1302_RAM0_WRITE	0xC0
#define DS1302_RAM1_WRITE	0xC2
#define DS1302_RAM2_WRITE	0xC4
#define DS1302_RAM3_WRITE	0xC6

#define DS1302_RAM0_READ	0xC1
#define DS1302_RAM1_READ	0xC3
#define DS1302_RAM2_READ	0xC5
#define DS1302_RAM3_READ	0xC7

//-----------------------------------²Ù×÷ºê----------------------------------// 

#define DS1302_SCLK_HIGH io_DS1302_SCLK = 1 ; 

#define DS1302_SCLK_LOW io_DS1302_SCLK = 0 ; 

#define DS1302_IO_HIGH io_DS1302_IO = 1 ; 

#define DS1302_IO_LOW io_DS1302_IO = 0 ; 

#define DS1302_IO io_DS1302_IO 



#define DS1302_RST_HIGH io_DS1302_RST = 1 ; 

#define DS1302_RST_LOW io_DS1302_RST = 0 ; 



#define DS1302ReadMyData(nRAM) 	DS1302ReadByte(DS1302_RAM0_READ + (nRAM << 1))

typedef unsigned char uint8;

typedef struct { 
	uint8 Second ; 
	uint8 Minute ; 
	uint8 Hour ; 
	uint8 Day ; 
	uint8 Month ; 
	uint8 Week ; 
	uint8 Year ; 
	uint8 WP;
}	DS1302Time ; 


/****************************************************************************** 

* Function: void ClockInit( void ) * 

* Description:	Initialize the Clock if this is the first time power on

* Parameter: * 

* * 

* Return: * 

******************************************************************************/ 
void ClockInit (void);

/****************************************************************************** 

* Function: void DS1302WriteByte( uint8 Address, uint8 Content ) * 

* Description: Write one BYTE 

* Parameter: Address

* Content: Data to write

* Return: * 

******************************************************************************/ 
void DS1302WriteByte (uint8 nAddress, uint8 nData);

/****************************************************************************** 

* Function: uint8 DS1302ReadByte (uint8 nAddress)

* Description:	Read a BYTE from the address

* Parameter: Address

* 

* Return: Readed value

******************************************************************************/ 
uint8 DS1302ReadByte (uint8 nAddress);

/****************************************************************************** 

* Function: void DS1302ReadTime( DS1302Time * )

* Description: Read the time from DS1302 and save to the data 

* Parameter: *DS1302Time 

* * 

* Return: TRUE  - Read OK

*         FALSE - Read Error 

******************************************************************************/ 
unsigned char DS1302ReadTime ( DS1302Time *);


/****************************************************************************** 

* Function: void DS1302WriteTime( DS1302Time * )

* Description: Write the time into DS1302

* Parameter: *DS1302Time 

* * 

* Return: TRUE  - Write OK

*         FALSE - Write Error 

******************************************************************************/ 
unsigned char DS1302WriteTime ( DS1302Time *);


/****************************************************************************** 

* Function: void DS1302WriteMyData (unsigned char, unsigned char)

* Description: Write my data

* Parameter: My data

* * 

* Return:

******************************************************************************/ 
void DS1302WriteMyData (unsigned char, unsigned char);

