/*
 * jmcki007_Final_Project.c
 *
 * Created: 5/30/2018 3:06:29 PM
 * Author : mckin
 */ 

#include <avr/io.h>
#include "timer.h"
#include "LED.h"
#include "game_utils.h"
#include "game.h"

#define tasksNum 7

typedef struct task {
    int state; // Current state of the task
    unsigned long period; // Rate at which the task should tick
    unsigned long elapsedTime; // Time since task's previous tick
    int (*TickFct)(int); // Function to call for task's tick
} task;



task tasks[tasksNum];

const unsigned long tasksPeriodGCD = 1;


unsigned char disp_on = 1;
const unsigned char BOARD_COLOR[3] = {1,0,0};
const unsigned char CURSOR_COLOR[3] = {2,3,3};
const unsigned char POSSIBLE_MOVE_COLOR[3] = {1,1,1};
const unsigned char PLAYR_COLOR[2][3] = {{0,0,2}, {0,2,0}};
const unsigned char KING_COLOR[2][3] = {{0,2,2}, {2,2,0}};
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

void ADC_init() {
    ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
    // ADEN: setting this bit enables analog-to-digital conversion.
    // ADSC: setting this bit starts the first conversion.
    // ADATE: setting this bit enables auto-triggering. Since we are
    //        in Free Running Mode, a new conversion will trigger whenever
    //        the previous conversion completes.
}

void set_frame()
{
    //Set up Basic board color
    
    for (unsigned char i = 0 ; i<8; i++)
    {
        set_pixels_in_column(i,0xFF,0,0,0);
        set_pixels_in_column(i,(0x55<<(i%2)),BOARD_COLOR[0],BOARD_COLOR[1],BOARD_COLOR[2]);
    }
    
    //Display Pieces
    
    for (unsigned char i = 0; i<24; i++)
    {
        if (pieces[i].in_play)
        {
			if (!pieces[i].king)
			{
				set_pixel(pieces[i].pos[0], pieces[i].pos[1], PLAYR_COLOR[pieces[i].player][0], PLAYR_COLOR[pieces[i].player][1], PLAYR_COLOR[pieces[i].player][2]);
			}
			else
			{
				set_pixel(pieces[i].pos[0], pieces[i].pos[1], KING_COLOR[pieces[i].player][0], KING_COLOR[pieces[i].player][1], KING_COLOR[pieces[i].player][2]);
			}
        }
    }
	
	if (piece_selected_flag && (piece_to_move<24))
	{
		set_pixel(pieces[piece_to_move].pos[0], pieces[piece_to_move].pos[1], PLAYR_COLOR[pieces[piece_to_move].player][0]+1,
		 PLAYR_COLOR[pieces[piece_to_move].player][1]+1, PLAYR_COLOR[pieces[piece_to_move].player][2]+1);
	}
    
    //Display possible moves
    
    if (piece_selected_flag)
    {
        for (unsigned char i = 0; i<num_possible; i++)
        {
            set_pixel(Possible_Moves[i].x, Possible_Moves[i].y, POSSIBLE_MOVE_COLOR[0], POSSIBLE_MOVE_COLOR[1], POSSIBLE_MOVE_COLOR[2]);
        }
    }
    
    //Display Cursor
    
    if (cursor_on)
    {
        set_pixel(cursor_position[0],cursor_position[1],CURSOR_COLOR[0],CURSOR_COLOR[1],CURSOR_COLOR[2]);
    }
}

void game_init()
{
    cursor_position[0] = CURSOR_START[0];
    cursor_position[1] = CURSOR_START[1];
    
    
    //Set up player 1 pieces
    for (unsigned char i = 0; i < 12; i++)
    {
        pieces[i].player = 0;
        pieces[i].king = 0;
        pieces[i].in_play = 1;
    }
    
    unsigned char i = 0;
    for (unsigned char j = 5; j<8 ; j++)
    {
        for (unsigned char k = (j+1)%2; k < 8; k += 2)
        {
            pieces[i].pos[0] = k;
            pieces[i].pos[1] = j;
            i++;
        }
    }
        
    //Set up player 2 pieces
    for (unsigned char i = 12; i < 24; i++)
    {
        pieces[i].player = 1;
        pieces[i].king = 0;
        pieces[i].in_play = 1;
    }
    
    i = 12;
    for (unsigned char j = 0; j<3 ; j++)
    {
        for (unsigned char k = (j+1)%2; k < 8; k += 2)
        {
            pieces[i].pos[0] = k;
            pieces[i].pos[1] = j;
            i++;
        }
    }
    
    
    set_frame();
}



enum GAME_STATES{GAME_INIT, PLAYER_1_TURN, PLAYER_2_TURN, PLAYER_1_WIN, PLAYER_2_WIN, RESET};

int tick_game(int state)
{
    switch (state)
    {
        case GAME_INIT:
            state = PLAYER_1_TURN;
            break;
        case PLAYER_1_TURN:
            if (piece_jumped<24)
            {
                pieces[piece_jumped].in_play = 0;
                piece_jumped = 25;  //
            }
            if (piece_moved)
            {
                piece_moved = 0;
                state = PLAYER_2_TURN;
            }
            break;
        case PLAYER_2_TURN:
            if (piece_jumped<24)
            {
                pieces[piece_jumped].in_play = 0;
                piece_jumped = 25;
            }
            if (piece_moved)
            {
                piece_moved = 0;
                state = PLAYER_1_TURN;
            }
            break;
        case PLAYER_1_WIN:
            if (!reset_flag)
            {
                state = PLAYER_1_WIN;
            }
            else
            {
                state = GAME_INIT;
            }
            break;
            
        case PLAYER_2_WIN:
            if (!reset_flag)
            {
                state = PLAYER_2_WIN;
            }
            else
            {
                state = GAME_INIT;
            }
            break;
            
        default:
            state = GAME_INIT;
            break;
    }
    
    switch (state) // actions
    {
        case GAME_INIT:
            game_init();
            cursor_player = 0;
            piece_moved = 0;
            break;
        case PLAYER_1_TURN:
            cursor_player = 0;
            break;
        case PLAYER_2_TURN:
            cursor_player = 1;
            break;
        case PLAYER_1_WIN:
            break;
        case PLAYER_2_WIN:
            break;
    }
    return state;
}

int set_display(int state)
{
    if (display_changed)
    {
        display_changed = 0;
        set_frame();
    }    
    return state;
}

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
                if (pwm<(pwm_div-1))
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
    DDRA = 0x00;
    PORTA = 0xFF;
    
	DDRB = 0xFF; // Set port B to output
	PORTB = 0x00; // Init port B to 0s
	
	DDRD = 0xFF;
	PORTD = 0x00;
    
    unsigned char i = 0;
    tasks[i].state = -1;
    tasks[i].period = 500;
    tasks[i].elapsedTime = 500;
    tasks[i].TickFct = &tick_button_input;
    i++;
    tasks[i].state = -1;
    tasks[i].period = 250;
    tasks[i].elapsedTime = 250;
    tasks[i].TickFct = &tick_joystick_input;
    i++;
    tasks[i].state = -1;
    tasks[i].period = 500;
    tasks[i].elapsedTime = 500;
    tasks[i].TickFct = &tick_cursor;
    i++;
    tasks[i].state = -1;
    tasks[i].period = 1000;
    tasks[i].elapsedTime = 1000;
    tasks[i].TickFct = &tick_cursor_blink;
    i++;
    tasks[i].state = -1;
    tasks[i].period = 500;
    tasks[i].elapsedTime = 500;
    tasks[i].TickFct = &tick_game;
    i++;
    tasks[i].state = -1;
    tasks[i].period = 500;
    tasks[i].elapsedTime = 500;
    tasks[i].TickFct = &set_display;
    i++;
    tasks[i].state = -1;
    tasks[i].period = 1;
    tasks[i].elapsedTime = 1;
    tasks[i].TickFct = &tick_display;
    
	ADC_init();
	
	TimerSet(1);
	TimerOn();
    
    while(1){}
}