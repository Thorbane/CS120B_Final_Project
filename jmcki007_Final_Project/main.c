/*
 * jmcki007_Final_Project.c
 *
 * Created: 5/30/2018 3:06:29 PM
 * Author : mckin
 */ 

#include <avr/io.h>
#include "timer.h"
#include "LED.h"

#define tasksNum 7

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
	
typedef struct move
{
	unsigned char x;
	unsigned char y;
	unsigned char jump;
	unsigned char jumped_piece;
	} move;

task tasks[tasksNum];


gamepiece pieces[24];

const unsigned long tasksPeriodGCD = 1;


unsigned char disp_on = 1;
const unsigned char BOARD_COLOR[3] = {1,0,0};
const unsigned char CURSOR_COLOR[3] = {2,3,3};
const unsigned char POSSIBLE_MOVE_COLOR[3] = {1,1,1};
const unsigned char PLAYR_COLOR[2][3] = {{0,0,2}, {0,2,0}};
const unsigned char CURSOR_START[2] = {3,3};
move Possible_Moves[4];
unsigned char num_possible;
unsigned char cursor_on;
unsigned char cursor_player = 0;
unsigned char piece_to_move = 25;
unsigned char piece_selected_flag, piece_moved, cursor_moved;    // Flags
unsigned char piece_jumped = 25;  // if set to valid index in pieces array, move will take the piece at index out of play
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
            set_pixel(pieces[i].pos[0], pieces[i].pos[1], PLAYR_COLOR[pieces[i].player][0], PLAYR_COLOR[pieces[i].player][1], PLAYR_COLOR[pieces[i].player][2]);
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
					piece_selected_flag = 1;
                    return 1;
                }
            }
        }
    }
    
    return 0;
}

/*unsigned char space_occupied(unsigned char pos[2])
{
    if (pos[0]>7 || pos[1]>7)
    {
        return 1;
    }
    
    else
    {
        for (unsigned char i =0 ; i<24; i++)
        {
            if (pieces[i].pos[0] == pos[0] || pieces[i].pos[1] == pos[1])
            {
                return 1;
            }
        }
    }
    
    return 0;
} */

unsigned char space_occupied(unsigned char x, unsigned char y)
{
    if (x>7 || y>7)
    {
        return 1;
    }
    
    else
    {
        for (unsigned char i = 0 ; i<24; i++)
        {
            if (pieces[i].pos[0] == x || pieces[i].pos[1] == y)
            {
                return 1;
            }
        }
    }
    
    return 0;
}

unsigned char find_moves_for_piece()
{
    move temp[4]; // temporarily store possible move positions for checking
    unsigned char index = 0;
    if (!pieces[piece_to_move].king)
    {
        if (pieces[piece_to_move].player == 0)
        {
            if (pieces[piece_to_move].pos[0]>0)
            {
                temp[index].x = pieces[piece_to_move].pos[0]-1;
                temp[index].y = pieces[piece_to_move].pos[1]-1;
                index++;
            }
            if (pieces[piece_to_move].pos[0]<7)
            {
                temp[index].x = pieces[piece_to_move].pos[0]+1;
                temp[index].y = pieces[piece_to_move].pos[1]-1;
                index++;
            }
            // First 12 pieces in the array are player 0
            for (unsigned char i = 0; i<12; i++)
            {
                for (unsigned char j = 0; j<index; j++)
                {
                    if ((pieces[i].pos[0] == temp[j].x) && (pieces[i].pos[1] == temp[j].y))
                    {
                        temp[j].x = 8;  // set position to invalid value to indicate it's invalid
                    }
                }
            }
			
			for (unsigned char i = 12; i<24; i++)
			{
				for (unsigned char j = 0; j<index; j++)
				{
					if ((pieces[i].pos[0] == temp[j].x) && (pieces[i].pos[1] == temp[j].y))
                    {
						unsigned char x = temp[j].x +(temp[j].x - pieces[i].pos[0]);
						unsigned char y = temp[j].y +(temp[j].y - pieces[i].pos[1]);
						
                        if (!space_occupied(x, y))   //check if space over the piece is open, if so can jump over
						{
							temp[j].x = x;
							temp[j].y = y;
							temp[j].jump = 1;
							temp[j].jumped_piece = i;
						}
                    }
				}
			}
        }
    }
    unsigned char count = 0;
    for (unsigned char i = 0; i<index; i++) // assign found valid moves to glabal array
    {
        if (temp[i].x<8)
        {
            //Possible_Moves[count].x = temp[i].x;
            //Possible_Moves[count].y = temp[i].y;
			Possible_Moves[count] = temp[i];
            count++;
        }
    }
    
    return count;
}

void move_piece(unsigned char index)  // Moves Selected piece.
{
	pieces[piece_to_move].pos[0] = Possible_Moves[index].x;
	pieces[piece_to_move].pos[1] = Possible_Moves[index].y;
    
    if (pieces[piece_to_move].pos[1] == 0 && pieces[piece_to_move].player == 0)
    {
        pieces[piece_to_move].king = 1;
    }
    
    if (pieces[piece_to_move].pos[1] == 7 && pieces[piece_to_move].player == 1)
    {
        pieces[piece_to_move].king = 1;
    }
    piece_moved = 1;  // set global piece moved flag
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
                    num_possible = find_moves_for_piece();
                    piece_selected_flag = 1;
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
			else if (select_flag && cursor_position[0] == pieces[piece_to_move].pos[0] && cursor_position[1] == pieces[piece_to_move].pos[1])
			{
				state = MOVE;
				num_possible = 0;
				piece_selected_flag = 0;
				piece_to_move = 25;
				select_flag = 0;
			}                
			else if (select_flag)
			{
                for (unsigned char i = 0; i<num_possible; i++)
                {
                    if (cursor_position[0] == Possible_Moves[i].x && cursor_position[1] == Possible_Moves[i].y)
                    {
						if (Possible_Moves[i].jump)
						{
							piece_jumped = Possible_Moves[i].jumped_piece;
						}
                        move_piece(i);
                        state = MOVE;
                        num_possible = 0;
                        piece_selected_flag = 0;
                        piece_to_move = 25;
                        select_flag = 0;
                        break;
                    }
                }
                if (!piece_moved)
                {
                    state = SELECT_PIECE;
                    select_flag = 0;
                }                    
            }
            else if (reset_flag)
            {
                select_flag = 0;
                piece_to_move = 25;
                piece_selected_flag = 0;
                num_possible = 0;
            }
            else
            {
                state = MOVE;
                num_possible = 0;
                piece_selected_flag = 0;
                piece_to_move = 25;
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
        
        case SELECT_PIECE:  // Much the same actions as the MOVE state, except cursor is tethered to within 2 tiles of selected piece
            if (up_flag != down_flag)
            {
                if(up_flag && (cursor_position[1]>0) && cursor_position[1]>(pieces[piece_to_move].pos[1]-2))
                {
                    cursor_position[1]--;
                    cursor_moved = 1;
                }
                if(down_flag && (cursor_position[1]<7) && cursor_position[1]<(pieces[piece_to_move].pos[1]+2))
                {
                    cursor_position[1]++;
                    cursor_moved = 1;
                }
            }
            
            if (right_flag != left_flag)
            {
                if(left_flag && (cursor_position[0]>0) && cursor_position[0]>(pieces[piece_to_move].pos[0]-2))
                {
                    cursor_position[0]--;
                    cursor_moved = 1;
                }
                if(right_flag && (cursor_position[0]<7) && cursor_position[0]<(pieces[piece_to_move].pos[0]+2))
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
    if (!(PINA & 0x80))
    {
        reset_flag = 1;
    }
    
    return state;
}      

enum JOYSTICK_STATES{J_INIT, X, Y, SEL};

int tick_joystick_input(int state)
{
    unsigned long input = ADC;
    static long x, y;
    switch (state)
    {
        case J_INIT:
            x = 0;
            y = 0;
            ADMUX = 0;
            state = X;
            break;
        case X:
            if (!(PINA & 0x40))
            {
                state = SEL;
                select_flag = 1;
                break;
            }
            if (input < 462)
            {
                x += (input - 512);
                if (x < -475)
                {
                    left_flag = 1;
                    x = 0;
                }
            }
            else if (input > 562)
            {
                x += (input - 512);
                if (x > 475)
                {
                    right_flag = 1;
                    x = 0;
                }
            }
            else
            {
                x = 0;
            }
            state = Y;
            ADMUX = 1;
            break;
        case Y:
            if (!(PINA & 0x40))
            {
                state = SEL;
                select_flag = 1;
                break;
            }
            if (input < 462)
            {
                y += (input - 512);
                if (y < -475)
                {
                    down_flag = 1;
                    y = 0;
                }
            }
            else if (input > 562)
            {
                y += (input - 512);
                if (y > 475)
                {
                    up_flag = 1;
                    y = 0;
                }
            }
            else
            {
                y = 0;
            }
            state = X;
            ADMUX = 1;
            break;
        case SEL:
            if (PINA & 0x40)
            {
                state = X;
                x = 0;
                y = 0;
            }
            else {state = SEL;}
            break;
		default:
			state = J_INIT;
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

enum GAME_STATES{GAME_INIT, PLAYER_1_TURN, PLAYER_2_TURN, PLAYER_1_WIN, PLAYER_2_WIN, RESET};

int tick_game(int state)
{
    switch (state)
    {
        case GAME_INIT:
            game_init();
            cursor_player = 0;
            state = PLAYER_1_TURN;
            break;
        case PLAYER_1_TURN:
            if (piece_jumped)
            {
                piece_moved = 0;  //set flag to 0
                state = PLAYER_1_TURN;  // continue turn
            }
            if (piece_moved)
            {
                piece_moved = 0;
                state = PLAYER_2_TURN;
            }
            break;
        case PLAYER_2_TURN:
            if (piece_jumped)
            {
                piece_moved = 0;  //set flag to 0
                state = PLAYER_2_TURN;  // continue turn
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
    
    game_init();
	ADC_init();
	
	TimerSet(1);
	TimerOn();
    
    while(1){}
}