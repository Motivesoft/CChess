#include <io.h>
#include <ctype.h>
#include <errno.h>
#include <intrin.h>
#include <memory.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "Move.h"

Move Move_createMove( unsigned long from, unsigned long to )
{
    return (from << 6) | to;
}

Move Move_createPromotionMove( unsigned long from, unsigned long to, unsigned long promotion )
{
    return (promotion << 12) | Move_createMove( from, to );
}

struct MoveList* MoveList_createMoveList()
{
    struct MoveList* moveList = malloc( sizeof( struct MoveList ) );

    if ( moveList != NULL )
    {
        moveList->count = 0;
    }

    return moveList;
}

unsigned long Move_from( Move self ) 
{
    return (self & 0b00000000000000000000111111000000) >> 6;
}

unsigned long Move_fromRank( Move self ) 
{
    return ( Move_from( self ) >> 3 ) & 0b00000000000000000000000000000111;
}

unsigned long Move_fromFile( Move self ) 
{
    return Move_from( self ) & 0b00000000000000000000000000000111;
}

unsigned long Move_to( Move self ) 
{
    return self & 0b00000000000000000000000000111111;
}

unsigned long Move_toRank( Move self ) 
{
    return ( Move_to( self ) >> 3 ) & 0b00000000000000000000000000000111;
}

unsigned long Move_toFile( Move self ) 
{
    return Move_to( self ) & 0b00000000000000000000000000000111;
}

unsigned long Move_promotion( Move self )
{
    return (self & 0b00000000000000001111000000000000) >> 12;
}

bool Move_isPromotion( Move self )
{
    return (self & 0b00000000000000001111000000000000) != 0;
}

// MoveList

void MoveList_destroy( struct MoveList* self )
{
    free( self );
}

void MoveList_addMove( struct MoveList* self, Move move )
{
    self->moves[ self->count++ ] = move;
}
