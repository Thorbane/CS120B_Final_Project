/*
 * game_utils.h
 *
 * Created: 6/6/2018 8:33:10 PM
 *  Author: mckin
 */ 


#ifndef GAME_UTILS_H_
#define GAME_UTILS_H_

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


move Possible_Moves[4];
unsigned char num_possible;
unsigned char cursor_player;
unsigned char piece_to_move = 25;
unsigned char piece_selected_flag, piece_moved, cursor_moved, display_changed;    // Flags
unsigned char cursor_position[2];


gamepiece pieces[24];

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
            if (pieces[i].pos[0] == x && pieces[i].pos[1] == y && pieces[i].in_play)
            {
                return 1;
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


unsigned char select_piece()
{
    for (unsigned char i = 0; i < 24; i++)
    {
        if (pieces[i].in_play)
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
    }
    
    return 0;
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
    display_changed = 1;  //set diplay changed flag
}

unsigned char find_moves_for_piece()
{
    move temp_to_test[4]; // temporarily store possible move positions for checking
    unsigned char index = 0;
    if (!pieces[piece_to_move].king)
    {
        if (pieces[piece_to_move].player == 0)
        {
            if (pieces[piece_to_move].pos[0]>0) // bounds check
            {
                temp_to_test[index].x = pieces[piece_to_move].pos[0]-1;
                temp_to_test[index].y = pieces[piece_to_move].pos[1]-1;
                temp_to_test[index].jump = 0;
                temp_to_test[index].jumped_piece = 25;
                index++;
            }
            if (pieces[piece_to_move].pos[0]<7)  // bounds check 
            {
                temp_to_test[index].x = pieces[piece_to_move].pos[0]+1;
                temp_to_test[index].y = pieces[piece_to_move].pos[1]-1;
                temp_to_test[index].jump = 0;
                temp_to_test[index].jumped_piece = 25;
                index++;
            }
            // First 12 pieces in the array are player 0
            for (unsigned char i = 0; i<12; i++)
            {
                if (pieces[i].in_play)
                {
                    for (unsigned char j = 0; j<index; j++)
                    {
                        if (pieces[i].in_play && (pieces[i].pos[0] == temp_to_test[j].x) && (pieces[i].pos[1] == temp_to_test[j].y))
                        {
                            temp_to_test[j].x = 8;  // set position to invalid value to indicate it's invalid
                        }
                    }
                }                
            }
            
            for (unsigned char i = 12; i<24; i++) //for each opposing piece
            {
                if (pieces[i].in_play)
                {
                    for (unsigned char j = 0; j<index; j++) // check position against each possible move
                    {
                        // if position of piece and possible move
                        if ((pieces[i].pos[0] == temp_to_test[j].x) && (pieces[i].pos[1] == temp_to_test[j].y))
                        {
                            unsigned char x = temp_to_test[j].x +(temp_to_test[j].x - pieces[piece_to_move].pos[0]);
                            unsigned char y = temp_to_test[j].y +(temp_to_test[j].y - pieces[piece_to_move].pos[1]);
                        
                            if (!space_occupied(x, y))   //check if space over the piece is open, if so it can jump over
                            {
                                temp_to_test[j].x = x;
                                temp_to_test[j].y = y;
                                temp_to_test[j].jump = 1;
                                temp_to_test[j].jumped_piece = i;
                            }
                            else // otherwise move is not valid
                            {
                                temp_to_test[j].x = 8;
                            }
                        }
                    }
                }                                
            }
        }
        
        if (pieces[piece_to_move].player == 1)  // same checks for the second player
        {
            if (pieces[piece_to_move].pos[0]>0) //bounds check
            {
                temp_to_test[index].x = pieces[piece_to_move].pos[0]-1;
                temp_to_test[index].y = pieces[piece_to_move].pos[1]+1;
                temp_to_test[index].jump = 0;
                temp_to_test[index].jumped_piece = 25;
                index++;
            }
            if (pieces[piece_to_move].pos[0]<7)
            {
                temp_to_test[index].x = pieces[piece_to_move].pos[0]+1;
                temp_to_test[index].y = pieces[piece_to_move].pos[1]+1;
                temp_to_test[index].jump = 0;
                temp_to_test[index].jumped_piece = 25;
                index++;
            }
            // Cannot move onto or jump over a square holding a piece from the same team
            for (unsigned char i = 12; i<24; i++)
            {
                if (pieces[i].in_play)
                {
                    for (unsigned char j = 0; j<index; j++)
                    {
                        if ((pieces[i].pos[0] == temp_to_test[j].x) && (pieces[i].pos[1] == temp_to_test[j].y))
                        {
                            temp_to_test[j].x = 8;  // set position to invalid value to indicate it's invalid
                        }
                    }
                }
            }
            
            for (unsigned char i = 0; i<12; i++)
            {
                if (pieces[i].in_play)
                {
                    for (unsigned char j = 0; j<index; j++)
                    {
                        if ((pieces[i].pos[0] == temp_to_test[j].x) && (pieces[i].pos[1] == temp_to_test[j].y))
                        {
                            unsigned char x = temp_to_test[j].x + (temp_to_test[j].x - pieces[piece_to_move].pos[0]);
                            unsigned char y = temp_to_test[j].y + (temp_to_test[j].y - pieces[piece_to_move].pos[1]);
                        
                            if (!space_occupied(x, y))   //check if space over the piece is open, if so can jump over
                            {
                                temp_to_test[j].x = x;
                                temp_to_test[j].y = y;
                                temp_to_test[j].jump = 1;
                                temp_to_test[j].jumped_piece = i;
                            }
                            else // otherwise move is not valid
                            {
                                temp_to_test[j].x = 8;
                            }
                        }
                    }                    
                }
            }
        }
    }
	else
	{
		if (pieces[piece_to_move].pos[0]>0) // bounds check
		{
			temp_to_test[index].x = pieces[piece_to_move].pos[0]-1;
			temp_to_test[index].y = pieces[piece_to_move].pos[1]-1;
			temp_to_test[index].jump = 0;
			temp_to_test[index].jumped_piece = 25;
			index++;
			temp_to_test[index].x = pieces[piece_to_move].pos[0]-1;
			temp_to_test[index].y = pieces[piece_to_move].pos[1]+1;
			temp_to_test[index].jump = 0;
			temp_to_test[index].jumped_piece = 25;
			index++;
		}
		if (pieces[piece_to_move].pos[0]<7)  // bounds check
		{
			temp_to_test[index].x = pieces[piece_to_move].pos[0]+1;
			temp_to_test[index].y = pieces[piece_to_move].pos[1]-1;
			temp_to_test[index].jump = 0;
			temp_to_test[index].jumped_piece = 25;
			index++;
			temp_to_test[index].x = pieces[piece_to_move].pos[0]+1;
			temp_to_test[index].y = pieces[piece_to_move].pos[1]+1;
			temp_to_test[index].jump = 0;
			temp_to_test[index].jumped_piece = 25;
			index++;
		}
		
		for (unsigned char i; i<24; i++)
		{
			for (unsigned char j = 0; j<index; j++)
			{
				if ((pieces[i].pos[0] == temp_to_test[j].x) && (pieces[i].pos[1] == temp_to_test[j].y))
				{
					if (pieces[i].player == pieces[piece_to_move].player)
					{
						temp_to_test[j].x = 8;
					}
					else
					{
						unsigned char x = temp_to_test[j].x + (temp_to_test[j].x - pieces[piece_to_move].pos[0]);
						unsigned char y = temp_to_test[j].y + (temp_to_test[j].y - pieces[piece_to_move].pos[1]);
						
						if (!space_occupied(x, y))   //check if space over the piece is open, if so can jump over
						{
							temp_to_test[j].x = x;
							temp_to_test[j].y = y;
							temp_to_test[j].jump = 1;
							temp_to_test[j].jumped_piece = i;
						}
						else // otherwise move is not valid
						{
							temp_to_test[j].x = 8;
						}
					}
				}				
			}
		}
	}
	
    unsigned char count = 0;
    for (unsigned char i = 0; i<index; i++) // assign found valid moves to global array
    {
        if (temp_to_test[i].x<8)
        {
            //Possible_Moves[count].x = temp[i].x;
            //Possible_Moves[count].y = temp[i].y;
            Possible_Moves[count] = temp_to_test[i];
            count++;
        }
    }
    
    return count;
}

#endif /* GAME_UTILS_H_ */