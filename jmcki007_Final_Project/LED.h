/*
* jmcki007_LED_lib.h
*
* Created: 5/30/2018 3:19:15 PM
* Author : mckin
 
*  output pin 0 = line register serial
*  output pin 1 = red register serial
*  output pin 2 = green register serial
*  output pin 3 = blue register serial
*  output pin 4 = output enable (active low)    SR pin 13
*  output pin 5 = register clock                SR pin 12
*  output pin 6 = shift register clock          SR pin 11
*  output pin 7 = shift reg Clear (active low)            SR pin 10

        7   |   6   |   5   |   4   |   3   |   2   |   1   |   0
    ~SRCLR  | SRCLK | RCLK  |  ~OE  | blue  | green |  red  | column
*/  

#ifndef LED_MATRIX
#define LED_MATRIX

#define port PORTB
#define column PORTD

#define pwm_div 5

unsigned char frame[8][3][pwm_div];  //8 columns, 3 colors, 5 PWM

void set_line(unsigned char line, unsigned char red, unsigned char green, unsigned char blue)
{
    red = ~red;
    green = ~green;
    blue = ~blue;
	unsigned char output;
	for (unsigned char i = 0; i<8; i++)
	{
		output = (0x80 | ((red & 0x01)<<1) | ((green & 0x01)<<2) | ((blue & 0x01)<<3));
		PORTB = output;
		red = red >> 1;
		green = green >> 1;
		output = output | 0x40;
		PORTB = output; 
		blue = blue >> 1;
	}
	
	output = output | 0x30;
	PORTB = output;
    PORTD = 0x01 << line;
    PORTB = output;
}

void disp_line(unsigned char col, unsigned char pwm)
{
    unsigned char red = frame[col][0][pwm];
    unsigned char green = frame[col][1][pwm];
    unsigned char blue = frame[col][2][pwm];
    
    set_line(col, red, green, blue);
}

void set_pixels_in_column(unsigned char col, unsigned char pixel_mask, unsigned char red, unsigned char green, unsigned char blue)
{
    for (unsigned char i=0; i<pwm_div; i++)
    {
        // Set up red
        if ((i)<red)
        {
            frame[col][0][i] |= pixel_mask;
        }
        else
        {
            frame[col][0][i] &= ~pixel_mask;
        }
        
        //Green
        if ((i)<green)
        {
            frame[col][1][i] |= pixel_mask;
        }
        else
        {
            frame[col][1][i] &= ~pixel_mask;
        }
        
        //Blue
        if ((i)<blue)
        {
            frame[col][2][i] |= pixel_mask;
        }
        else
        {
            frame[col][2][i] &= ~pixel_mask;
        }
    }
}

void set_pixel(unsigned char col, unsigned char row, unsigned char red, unsigned char green, unsigned char blue)
{
    for (unsigned char i=0; i<pwm_div; i++)
    {
        // Set up red
        if ((i)<red)
        {
            frame[col][0][i] |= 0x01<<row;
        }
        else
        {
            frame[col][0][i] &= ~(0x01<<row);
        }
        
        //Green
        if ((i)<green)
        {
            frame[col][1][i] |= 0x01<<row;
        }
        else
        {
            frame[col][1][i] &= ~(0x01<<row);
        }
        
        //Blue
        if ((i)<blue)
        {
            frame[col][2][i] |= 0x01<<row;
        }
        else
        {
            frame[col][2][i] &= ~(0x01<<row);
        }
    }
}


#endif

 