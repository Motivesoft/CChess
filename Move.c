#include "stdafx.h"

#include "Move.h"

struct Move* Move_createMove( unsigned long from, unsigned long to )
{
    struct Move* move = malloc( sizeof( struct Move ) );

    if ( move != NULL )
    {
        move->moveBits = 0;
        move->moveBits |= to;
        move->moveBits |= from << 6;
    }

    return move;
}

struct Move* Move_createPromotionMove( unsigned long from, unsigned long to, unsigned long promotion )
{
    struct Move* move = malloc( sizeof( struct Move ) );

    if ( move != NULL )
    {
        move->moveBits = 0;
        move->moveBits |= to;
        move->moveBits |= from << 6;
        move->moveBits |= promotion << 12;
    }

    return move;
}

void Move_destroy( struct Move* self )
{
    free( self );
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

unsigned long Move_from( struct Move* self ) 
{
    return (self->moveBits & 0b00000000000000000000111111000000) >> 6;
}

unsigned long Move_fromRank( struct Move* self ) 
{
    return ( Move_from( self ) >> 3 ) & 0b00000000000000000000000000000111;
}

unsigned long Move_fromFile( struct Move* self ) 
{
    return Move_from( self ) & 0b00000000000000000000000000000111;
}

unsigned long Move_to( struct Move* self ) 
{
    return self->moveBits & 0b00000000000000000000000000111111;
}

unsigned long Move_toRank( struct Move* self ) 
{
    return ( Move_to( self ) >> 3 ) & 0b00000000000000000000000000000111;
}

unsigned long Move_toFile( struct Move* self ) 
{
    return Move_to( self ) & 0b00000000000000000000000000000111;
}

unsigned long Move_promotion( struct Move* self )
{
    return (self->moveBits & 0b00000000000000001111000000000000) >> 12;
}

bool Move_isPromotion( struct Move* self )
{
    return (self->moveBits & 0b00000000000000001111000000000000) != 0;
}

// MoveList

void MoveList_destroy( struct MoveList* self )
{
    for ( unsigned char loop = 0; loop < self->count; loop++ )
    {
        Move_destroy( self->moves[ loop ] );
    }

    free( self );
}

void MoveList_addMove( struct MoveList* self, struct Move* move )
{
    // Effectively, this takes ownership of move's memory - if this won't work for us, we need to take a copy or something
    self->moves[ self->count++ ] = move;
}
