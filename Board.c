#include "stdafx.h"

#include "Board.h"

struct Board* Board_create( const char* fen )
{
    struct Board* board = malloc( sizeof( struct Board ) );

    if ( board != NULL )
    {
        // TODO initialise from FEN
        board->whiteToMove = true;
    }

    return board;
}

void Board_shutdown( struct Board* self )
{
    if ( self != NULL )
    {
        free( self );
    }
}
