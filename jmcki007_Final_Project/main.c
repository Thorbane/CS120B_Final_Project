/*
 * jmcki007_Final_Project.c
 *
 * Created: 5/30/2018 3:06:29 PM
 * Author : mckin
 */ 

#include <avr/io.h>
#include "timer.h"
#include "LED.h"

#define tasksNum 5

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

task tasks[tasksNum];


gamepiece pieces[24];

const unsigned long tasksPeriodGCD = 1;


unsigned char disp_on = 1;
const unsigned char BOARD_COLOR[3] = {1,0,0};
const unsigned char CURSOR_COLOR[3] = {2,3,3};
const unsigned char PLAYR_COLOR[2][3] = {{0,0,2}, {0,2,0}};
const unsigned char CURSOR_START[2] = {3,3};
unsigned char cursor_on;
unsigned char cursor_player = 0;
unsigned char piece_to_move = 25;
unsigned char piece_selected, piece_moved, cursor_moved;    // Flags
unsigned char select_flag, reset_flag, up_flag, down_flag, right_flag, left_flag = 0; // Input Flags
unsigned char cursor_position[2];


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
            set_pixel(pieces[i].pos[0], pieces[i].pos[1], PLAYR_COLOR[pieces[i].player][0], PLAYR_COLOR[pieces[i].player][1], PLAYR_COLOR[pieces[i].player][2]);
        }
    }
	
	if (piece_selected && (piece_to_move<25))
	{
		set_pixel(pieces[piece_to_move].pos[0], pieces[piece_to_move].pos[1], PLAYR_COLOR[pieces[piece_to_move].player][0]+1,
		 PLAYR_COLOR[pieces[piece_to_move].player][1]+1, PLAYR_COLOR[pieces[piece_to_move].player][2]+1);
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

unsigned char select_piece()
{
    for (unsigned char i = 0; i < 24; i++)
    {
        if (pieces[i].pos[0] == cursor_position[0])
        {
            if(pieces[i].pos[1] == cursor_position[1])
            {
                if (pieces[i].player == cursor_player)
                {
                    piece_to_move = i;
					piece_selected = 1;
                    return 1;
                }
            }
        }
    }
    
    return 0;
}

unsigned char is_valid_move(unsigned char p_x, unsigned char p_y, unsigned char c_x, unsigned char c_y)
{
    return 1;
}

void move_piece()
{
	pieces[piece_to_move].pos[0] = cursor_position[0];
	pieces[piece_to_move].pos[1] = cursor_position[1];
}

enum ON_OFF_states{INIT, ON, OFF};
enum CURSOR_STATES{INI, MOVE, SELECT_PIECE};

int tick_cursor(int state)
{
    switch(state)
    {
        case INI:
            state = MOVE;
            break;
        case MOVE:
            if(select_flag)
            {
                select_flag = 0;
                if(select_piece())
                {
                    state = SELECT_PIECE;
                    piece_selected = 1;
                }
                else
                {
                    state = MOVE;
                }
            }
            else
            {
                state = MOVE;
            }
            break;
        case SELECT_PIECE:
            if (!select_flag && !reset_flag)
			{
				state = SELECT_PIECE;
			}
			else if (select_flag && cursor_position[0] == pieces[piece_selected].pos[0] && cursor_position[1] == pieces[piece_selected].pos[1])
			{
				state = MOVE;
                select_flag = 0;
                piece_to_move = 25;
                piece_selected = 0;
			}
			else
			{
				if (is_valid_move(pieces[piece_to_move].pos[0], pieces[piece_to_move].pos[1], cursor_position[0], cursor_position[1]))
				{
                    move_piece(piece_to_move);
					piece_moved = 1;
                    select_flag = 0;
                    state = MOVE;
                    piece_to_move = 25;
                    piece_selected = 0;
                    break;
				}
                state = MOVE;
            }                
            break;
        default:
            state = INIT;
            break;
    }
    
    switch (state)
    {
        case INI:
            break;
            
        case MOVE:        
            if (up_flag != down_flag)
            {
                if(up_flag && (cursor_position[1]>0))
                {
                    cursor_position[1]--;
                    cursor_moved = 1;
                }
                if(down_flag && (cursor_position[1]<7))
                {
                    cursor_position[1]++;
                    cursor_moved = 1;
                }
            }
            
            if (right_flag != left_flag)
            {
                if(left_flag && (cursor_position[0]>0))
                {
                    cursor_position[0]--;
                    cursor_moved = 1;
                }
                if(right_flag && (cursor_position[0]<7))
                {
                    cursor_position[0]++;
                    cursor_moved = 1;
                }
            }
            up_flag = down_flag = left_flag = right_flag = 0;
            break;
        
        case SELECT_PIECE:
            if (up_flag != down_flag)
            {
                if(up_flag && (cursor_position[1]>0))
                {
                    cursor_position[1]--;
                    cursor_moved = 1;
                }
                if(down_flag && (cursor_position[1]<7))
                {
                    cursor_position[1]++;
                    cursor_moved = 1;
                }
            }
            
            if (right_flag != left_flag)
            {
                if(left_flag && (cursor_position[0]>0))
                {
                    cursor_position[0]--;
                    cursor_moved = 1;
                }
                if(right_flag && (cursor_position[0]<7))
                {
                    cursor_position[0]++;
                    cursor_moved = 1;
                }
            }
            up_flag = down_flag = left_flag = right_flag = 0;
            break;
    }
    
    return state;
}

int tick_button_input(int state)
{
    if (!(PINA & 0x04))
    {
        up_flag = 1;
    }        
    if (!(PINA & 0x08))
    {
        down_flag = 1;
    }
    if (!(PINA & 0x10))
    {
        right_flag = 1;
    }
    if (!(PINA & 0x20))
    {
        left_flag = 1;
    }
    if (!(PINA & 0x40))
    {
        select_flag = 1;
    }
    if (!(PINA & 0x80))
    {
        reset_flag = 1;
    }
    
    return state;
}      
    
int tick_cursor_blink(int state)
{
    static unsigned char i;
    
    switch(state)      //Transitions
    {
        case ON:
            if (cursor_moved)
            {
                i = 0;
                cursor_moved = 0;
            }
            if (i<2)
            {
                state = ON;
            }
            else
            {
                state = OFF;
                i = 0;
                cursor_on = 0;
            }
            break;
        case OFF:
            if (i<2 && !cursor_moved)
            {
                state = OFF;
            }
            else
            {
                state = ON;
                cursor_on = 1;
                cursor_moved = 0;
                i = 0;
            }
            break;
        case INIT:
            state = ON;
            cursor_on = 1;
            i = 0;
            break;
        default:
            state = INIT;
            break;
    }
    
    switch (state)  //actions
    {
        case ON:
            i++;
            break;
        case OFF:
            i++;
            break;
        case INIT:
            break;
    }
    
    return state;
}

int set_display(int state)
{
    set_frame();
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
    tasks[i].TickFct = &set_display;
    i++;
    tasks[i].state = -1;
    tasks[i].period = 1;
    tasks[i].elapsedTime = 1;
    tasks[i].TickFct = &tick_display;
    
    game_init();
	//ADC_init();
	
	TimerSet(1);
	TimerOn();
    
    while(1){}
}