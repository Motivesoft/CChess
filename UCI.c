#include "stdafx.h"

#include "UCI.h"

// Intrnal methods

void UCI_broadcast( struct RuntimeSetup* runtimeSetup, const char* format, ... )
{
    va_list args;
    va_start( args, format );
    vfprintf( runtimeSetup->output, format, args );
    fprintf( runtimeSetup->output, "\n" );
    fflush( runtimeSetup->output );
    va_end( args );
}

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

void UCI_uci( struct UCIConfiguration* self, struct RuntimeSetup* runtimeSetup )
{
    UCI_broadcast( runtimeSetup, "id name %s", "CChess" );
    UCI_broadcast( runtimeSetup, "id author %s", "Motivesoft" );
    UCI_broadcast( runtimeSetup, "uciok" );
}

void UCI_quit( struct UCIConfiguration* self )
{
    // TODO stop any current activity and threads as quickly as possible
}