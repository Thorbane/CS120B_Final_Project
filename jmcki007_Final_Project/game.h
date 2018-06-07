/*
 * game.h
 *
 * Created: 6/6/2018 8:43:44 PM
 *  Author: mckin
 */ 


#ifndef GAME_H_
#define GAME_H_

#include "game_utils.h"

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
                        display_changed = 1;
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
                display_changed = 1;
            }
            if(down_flag && (cursor_position[1]<7))
            {
                cursor_position[1]++;
                cursor_moved = 1;
                display_changed = 1;
            }
        }
        
        if (right_flag != left_flag)
        {
            if(left_flag && (cursor_position[0]>0))
            {
                cursor_position[0]--;
                cursor_moved = 1;
                display_changed = 1;
            }
            if(right_flag && (cursor_position[0]<7))
            {
                cursor_position[0]++;
                cursor_moved = 1;
                display_changed = 1;
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
                    display_changed = 1;
                }
                if(down_flag && (cursor_position[1]<7) && cursor_position[1]<(pieces[piece_to_move].pos[1]+2))
                {
                    cursor_position[1]++;
                    cursor_moved = 1;
                    display_changed = 1;
                }
            }
        
            if (right_flag != left_flag)
            {
                if(left_flag && (cursor_position[0]>0) && cursor_position[0]>(pieces[piece_to_move].pos[0]-2))
                {
                    cursor_position[0]--;
                    cursor_moved = 1;
                    display_changed = 1;
                }
                if(right_flag && (cursor_position[0]<7) && cursor_position[0]<(pieces[piece_to_move].pos[0]+2))
                {
                    cursor_position[0]++;
                    cursor_moved = 1;
                    display_changed = 1;
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
                up_flag = 1;
                y = 0;
            }
        }
        else if (input > 562)
        {
            y += (input - 512);
            if (y > 475)
            {
                down_flag = 1;
                y = 0;
            }
        }
        else
        {
            y = 0;
        }
        state = X;
        ADMUX = 0;
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
            display_changed = 1;
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
            display_changed = 1;
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


#endif /* GAME_H_ */