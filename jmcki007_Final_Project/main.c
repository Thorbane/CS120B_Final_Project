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

task tasks[1];

const unsigned char tasksNum = 1;
const unsigned long tasksPeriodGCD = 1;
unsigned char disp_on = 1;


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

void disp_line(unsigned char col)
{
    unsigned char red = 0xAA>>(col%2);
    unsigned char green = 0x00;
    unsigned char blue = 0x00;
    
    set_line(col, red, green, blue);
}

enum ON_OFF_states{INIT, ON, OFF};

int tick_display(int state)
{
    static unsigned char col;
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
            disp_line(col);
            if (col<7)
            {
                col++;
            }
            else
            {
                col = 0;
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

int update_board(int state)
{
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
	
	TimerSet(1);
	TimerOn();
    
    while(1){}
}