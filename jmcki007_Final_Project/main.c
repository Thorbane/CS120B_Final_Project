/*
 * jmcki007_Final_Project.c
 *
 * Created: 5/30/2018 3:06:29 PM
 * Author : mckin
 */ 

#include <avr/io.h>
#include "timer.h"
#include "LED.h"

typedef struct task {
    int state; // Current state of the task
    unsigned long period; // Rate at which the task should tick
    unsigned long elapsedTime; // Time since task's previous tick
    int (*TickFct)(int); // Function to call for task's tick
} task;

typedef struct gamepiece
{
    unsigned char player;
    unsigned char in_play;
    unsigned char king;
    unsigned char pos[2];
    } gamepiece;

task tasks[1];

unsigned char frame[8][3][5];  //8 columns, 3 colors, 5 PWM
gamepiece pieces[24];

const unsigned char tasksNum = 1;
const unsigned long tasksPeriodGCD = 1;


unsigned char disp_on = 1;
const unsigned char BOARD_COLOR[3] = {1,0,0};
const unsigned char CURSOR_COLOR[3] = {2,2,2};
const unsigned char PLAYR_COLOR[2][3] = {{0,2,0}, {0,0,2}};
const unsigned char CURSOR_START[2] = {3,3};


void TimerISR() {
    unsigned char i;
	TimerFlag = 1;
    for (i = 0; i < tasksNum; ++i)
    {
        if ( tasks[i].elapsedTime >= tasks[i].period )
        {
            tasks[i].state = tasks[i].TickFct(tasks[i].state);
            tasks[i].elapsedTime = 0;
        }
        tasks[i].elapsedTime += tasksPeriodGCD;
    }
}

void set_pixels_in_column(unsigned char col, unsigned char pixel_mask, unsigned char red, unsigned char green, unsigned char blue)
{
    for (unsigned char i=0; i<5; i++)
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

void set_pixel(unsigned char row, unsigned char col, unsigned char red, unsigned char green, unsigned char blue)
{
    for (unsigned char i=0; i<5; i++)
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

void disp_line(unsigned char col, unsigned char pwm)
{
    unsigned char red = frame[col][0][pwm];
    unsigned char green = frame[col][1][pwm];
    unsigned char blue = frame[col][2][pwm];
    
    set_line(col, red, green, blue);
}

void set_frame()
{
    //Set up Basic board color
    
    for (unsigned char i = 0 ; i<8; i++)
    {
        set_pixels_in_column(i,(0x55<<(i%2)),BOARD_COLOR[0],BOARD_COLOR[1],BOARD_COLOR[2]);
    }
    
    //Display Pieces
    
    for (unsigned char i = 0; i<24; i++)
    {
        if (pieces[i].in_play)
        {
            set_pixel(pieces[i].pos[0], pieces[i].pos[1], PLAYR_COLOR[pieces[i].player][0], PLAYR_COLOR[pieces[i].player][1], PLAYR_COLOR[pieces[i].player][2]);
        }
    }
    
    //Display Cursor
}

enum ON_OFF_states{INIT, ON, OFF};

int tick_display(int state)
{
    static unsigned char col;
    static unsigned char pwm;
    switch(state)
    {
        case ON:
            if (disp_on)
            {
                state = ON;
            }
            else
            {
                state = OFF;
            }
            break;
        case OFF:
            if (disp_on)
            {
                state = ON;
            }
            else
            {
                state = OFF;
            }
            break;
        case INIT:
            if (disp_on)
            {
                state = ON;
            }
            else
            {
                state = OFF;
            }
            break;
        default:
            state = INIT;
            break;
    }
    
    switch (state) //actions
    {
        case ON:
            disp_line(col, pwm);
            if (col<7)
            {
                col++;
            }
            else
            {
                col = 0;
                if (pwm<4)
                {
                    pwm ++;
                }
                else
                {
                    pwm = 0;
                }                    
            }
            break;
        case OFF:
            break;
        case INIT:
            col = 0;
            break;
        default:
            break;
    }
    
    return state;
}

int main()
{
	DDRB = 0xFF; // Set port B to output
	PORTB = 0x00; // Init port B to 0s
	
	DDRD = 0xFF;
	PORTD = 0x00;
    
    unsigned char i = 0;
    tasks[i].state = -1;
    tasks[i].period = 1;
    tasks[i].elapsedTime = 1;
    tasks[i].TickFct = &tick_display;
    
    set_pixel(1,3,3,0,5);
    set_frame();
	
	TimerSet(1);
	TimerOn();
    
    while(1){}
}