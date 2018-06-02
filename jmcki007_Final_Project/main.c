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
const unsigned long tasksPeriodGCD = 10;

unsigned char output = 0;

void TimerISR() {
    unsigned char i;
	TimerFlag =1;
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

int main()
{
	DDRB = 0xFF; // Set port B to output
	PORTB = 0x00; // Init port B to 0s
	
	DDRD = 0xFF;
	PORTD = 0x00;
	
	TimerSet(500);
	TimerOn();
	while(!TimerFlag){}
	TimerFlag = 0;

	set_line(0x00, 0xAA, 0xAA, 0xAA);	
}