#include "stdafx.h"

#include "Perft.h"
#include "UCI.h"

// Internal methods

#define LOG_DEBUG( ... ) { log( runtimeSetup, DEBUG, __VA_ARGS__ ); }
#define LOG_INFO( ... ) { log( runtimeSetup, INFO, __VA_ARGS__ ); }
#define LOG_WARN( ... ) { log( runtimeSetup, WARN, __VA_ARGS__ ); }
#define LOG_ERROR( ... ) { log( runtimeSetup, ERROR, __VA_ARGS__ ); }

static const char* STARTPOS = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

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
    uci.fen = STARTPOS;

    return uci;
}

void UCI_shutdown( struct UCIConfiguration* self )
{
    // TODO release any resources
}

// UCI interface

struct UciCommandHandler uciCommandHandlers[] =
{
    { "uci", UCI_uci },
    { "debug", UCI_debug },
    { "isready", UCI_isready },
    { "setoption", UCI_setoption },
    { "register", UCI_register },
    { "ucinewgame", UCI_ucinewgame },
    { "position", UCI_position },
    { "go", UCI_go },
    { "stop", UCI_stop },
    { "ponderhit", UCI_ponderhit },
    { "quit", UCI_quit },
    { "perft", UCI_perft },
    { "test", UCI_test },
    { NULL, NULL }
};

bool UCI_processCommand( struct UCIConfiguration* self, struct RuntimeSetup* runtimeSetup, const char* command, char* arguments )
{
    for ( int loop = 0; uciCommandHandlers[ loop ].command != NULL; loop++ )
    {
        if ( strcmp( command, uciCommandHandlers[ loop ].command ) == 0 )
        {
            return uciCommandHandlers[ loop ].handler( self, runtimeSetup, arguments );
        }
    }

    LOG_ERROR( "Unrecognised command: %s", command );

    return true;
}

bool UCI_uci( struct UCIConfiguration* self, struct RuntimeSetup* runtimeSetup, char* arguments )
{
    LOG_DEBUG( "Processing uci command" );

    UCI_broadcast( runtimeSetup, "id name %s", "CChess" );
    UCI_broadcast( runtimeSetup, "id author %s", "Motivesoft" );
    UCI_broadcast( runtimeSetup, "uciok" );

    return true;
}

bool UCI_debug( struct UCIConfiguration* self, struct RuntimeSetup* runtimeSetup, char* arguments )
{
    LOG_DEBUG( "Processing debug command" );

    char* first;
    char* remainder;
    spliterate( arguments, &first, &remainder );

    if ( strcmp( arguments, "on" ) == 0 )
    {
        RuntimeSetup_setDebug( runtimeSetup, true );
    }
    else if ( strcmp( arguments, "off" ) == 0 )
    {
        RuntimeSetup_setDebug( runtimeSetup, false );
    }
    else
    {
        LOG_ERROR( "Illegal debug argument: %s", arguments );
        return true;
    }

    if ( strlen( remainder ) )
    {
        LOG_WARN( "Unexpected additional information passed to debug ignored: %s", remainder );
    }

    return true;
}

bool UCI_isready( struct UCIConfiguration* self, struct RuntimeSetup* runtimeSetup, char* arguments )
{
    LOG_DEBUG( "Processing isready command" );

    return true;
}

bool UCI_setoption( struct UCIConfiguration* self, struct RuntimeSetup* runtimeSetup, char* arguments )
{
    LOG_DEBUG( "Processing setoption command" );

    return true;
}

bool UCI_register( struct UCIConfiguration* self, struct RuntimeSetup* runtimeSetup, char* arguments )
{
    LOG_DEBUG( "Processing register command" );

    return true;
}

bool UCI_ucinewgame( struct UCIConfiguration* self, struct RuntimeSetup* runtimeSetup, char* arguments )
{
    LOG_DEBUG( "Processing ucinewgame command" );

    return true;
}

bool UCI_position( struct UCIConfiguration* self, struct RuntimeSetup* runtimeSetup, char* arguments )
{
    LOG_DEBUG( "Processing position command" );

    return true;
}

bool UCI_go( struct UCIConfiguration* self, struct RuntimeSetup* runtimeSetup, char* arguments )
{
    LOG_DEBUG( "Processing go command" );

    return true;
}

bool UCI_stop( struct UCIConfiguration* self, struct RuntimeSetup* runtimeSetup, char* arguments )
{
    LOG_DEBUG( "Processing stop command" );

    return true;
}

bool UCI_ponderhit( struct UCIConfiguration* self, struct RuntimeSetup* runtimeSetup, char* arguments )
{
    LOG_DEBUG( "Processing ponderhit command" );

    return true;
}

bool UCI_quit( struct UCIConfiguration* self, struct RuntimeSetup* runtimeSetup, char* arguments )
{
    LOG_DEBUG( "Processing quit command" );

    // TODO stop any current activity and threads as quickly as possible

    return false;
}

bool UCI_perft( struct UCIConfiguration* self, struct RuntimeSetup* runtimeSetup, char* arguments )
{
    LOG_DEBUG( "Processing perft command" );

    // Syntax:
    //  perft [n] <fen>              - moves to depth [n] from <fen>, if supplied, or startpos otherwise
    //  perft fen [fen-with-results] - moves based on expected results provided at the end of the [fen-with-results] string
    //  perft file [filename]        - read mutliple [fen-with-results] lines from a text file and process one by one

    // Which are we dealing with?
    char* keyword;
    char* remainder;
    spliterate( arguments, &keyword, &remainder );

    if ( strcmp( keyword, "file" ) == 0 )
    {
        Perft_file( runtimeSetup, remainder );
    }
    else if ( strcmp( keyword, "fen" ) == 0 )
    {
        Perft_fen( runtimeSetup, remainder );
    }
    else // Assume depth and optional fen
    {
        int depth = atoi( keyword );
        Perft_depth( runtimeSetup, depth, strlen( remainder ) > 0 ? remainder : STARTPOS );
    }

    return true;
}

bool UCI_test( struct UCIConfiguration* self, struct RuntimeSetup* runtimeSetup, char* arguments )
{
    LOG_DEBUG( "Processing test command" );

    return true;
}
