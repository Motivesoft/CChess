#pragma once

struct Board
{
    bool whiteToMove;
};

struct Board* Board_create( const char* fen );
void Board_shutdown( struct Board* self );
