/*
* jmcki007_LED_lib.h
*
* Created: 5/30/2018 3:19:15 PM
* Author : mckin
 
*  output pin 0 = line register serial
*  output pin 1 = red register serial
*  output pin 2 = green register serial
*  output pin 3 = blue register serial
*  output pin 4 = output enable
*  output pin 5 = shift register clock
*  output pin 6 = register clock
*/  

#ifndef LED_MATRIX
#define LED_MATRIX

#define port PORTB
#define column PORTD

void set_line(unsigned char line, unsigned char red, unsigned char green, unsigned char blue)
{
	unsigned char output;
	for (unsigned char i = 0; i<8; i++)
	{
		output = (0x00 | ((red & 0x01<<i)<<1) | ((green & 0x01<<i)<<2) | ((blue & 0x01<<i)<<3));
		PORTB = output;
		PORTD = 0x01 << line;
		red = red >> 1;
		green = green >> 1;
		output = output | 0x30;
		PORTB = output; 
		blue = blue >> 1;
	}
	
	output = output | 0x40;
	PORTB = output;
}

#endif

 