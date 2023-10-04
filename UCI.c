#include "stdafx.h"

#include "UCI.h"

// Control methods

struct UCIConfiguration UCI_createUCIConfiguration()
{
    struct UCIConfiguration uci;

    // Starting position
    uci.fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    return uci;
}

void UCI_shutdown( struct UCIConfiguration* self )
{
    // TODO release any resources
}

// UCI interface

void UCI_uci( struct UCIConfiguration* self, FILE* output )
{
    fprintf( output, "id name %s\n", "CChess" );
    fprintf( output, "id author %s\n", "Motivesoft" );
    fprintf( output, "uciok\n" );
}

void UCI_quit( struct UCIConfiguration* self )
{
    // TODO stop any current activity and threads as quickly as possible
}