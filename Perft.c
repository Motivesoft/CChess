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
#include <time.h>

#include "Perft.h"
#include "Utility.h"

#define BUFFER_SIZE 256

#define LOG_DEBUG( ... ) { RuntimeSetup_log( runtimeSetup, DEBUG, __VA_ARGS__ ); }
#define LOG_INFO( ... ) { RuntimeSetup_log( runtimeSetup, INFO, __VA_ARGS__ ); }
#define LOG_WARN( ... ) { RuntimeSetup_log( runtimeSetup, WARN, __VA_ARGS__ ); }
#define LOG_ERROR( ... ) { RuntimeSetup_log( runtimeSetup, ERROR, __VA_ARGS__ ); }

void Perft_depth( struct RuntimeSetup* runtimeSetup, int depth, const char* fen )
{
    LOG_DEBUG( "perft with depth %d and FEN: %s", depth, fen );

    struct Board* board = Board_create( fen );

    if ( board == NULL )
    {
        LOG_ERROR( "Cannot allocate memory for perft test" );
        return;
    }
    
    clock_t start = clock();

    unsigned long count = Perft_run( board, depth, runtimeSetup->debug );

    clock_t end = clock();

    float totalTime = (float) ( end - start ) / CLOCKS_PER_SEC;
    float nps = count / totalTime;
    fprintf( runtimeSetup->logger, "Move count: %u in %0.3fs (%0.0f nps)\n", count, totalTime, nps );
}

void Perft_fen( struct RuntimeSetup* runtimeSetup, const char* fenWithResults )
{
    LOG_DEBUG( "perft with FEN: %s", fenWithResults );

    // TODO
    // Split expected results from fen
    // Board* = Board_create( fen );
    // for each expected result
    //   unsigned long count = Perft_loop( board, depth, true );
    //   if( count != expected ) panic 
    //   else print count
}

void Perft_file( struct RuntimeSetup* runtimeSetup, const char* filename )
{
    LOG_DEBUG( "perft with file: %s", filename );

    FILE* file;
    errno_t err = fopen_s( &file, filename, "r" );
    if ( err == 0 )
    {
        char buffer[ BUFFER_SIZE ];
        memset( buffer, 0, BUFFER_SIZE );
        while ( fgets( buffer, BUFFER_SIZE, file ) )
        {
            sanitize( buffer );

            // Skip empty or comment lines
            if ( strlen( buffer ) == 0 || buffer[ 0 ] == '#' )
            {
                continue;
            }

            Perft_fen( runtimeSetup, buffer );
        }

        fclose( file );
    }
    else
    {
        LOG_ERROR( "Failed to open file: %s (reason %d)", filename, err );
    }
}

unsigned long Perft_run( struct Board* board, int depth, bool divide )
{
    return divide ? Perft_divide( board, depth ) : Perft_loop( board, depth );
}

unsigned long Perft_loop( struct Board* board, int depth )
{
    if ( depth == 0 )
    {
        return 1;
    }

    unsigned int nodes = 0;

    struct MoveList* moveList = Board_generateMoves( board );

    // This is a cheating optimisation
    if ( depth == 1 )
    {
        return moveList->count;
    }

    struct Board* copy = Board_copy( board );

    for ( unsigned char loop = 0; loop < moveList->count; loop++ )
    {
        if ( Board_makeMove( board, moveList->moves[ loop ] ) )
        {
            nodes += Perft_loop( board, depth - 1 );
        }

        Board_apply( board, copy );
    }
    Board_destroy( copy );

    MoveList_destroy( moveList );

    return nodes;
}

unsigned long Perft_divide( struct Board* board, int depth )
{
    if ( depth == 0 )
    {
        return 1;
    }

    unsigned int nodes = 0;
    unsigned int divideNodes = 0;

    char moveString[ 10 ];
    char fenString[ 256 ];

    struct MoveList* moveList = Board_generateMoves( board );

    struct Board* copy = Board_copy( board );

    for ( unsigned char loop = 0; loop < moveList->count; loop++ )
    {
        if ( Board_makeMove( board, moveList->moves[ loop ] ) )
        {
            divideNodes = Perft_loop( board, depth - 1 );
            nodes += divideNodes;

            Board_exportMove( moveList->moves[ loop ], moveString );
            Board_exportBoard( board, fenString );

            printf( "  %s : %d - %s\n", moveString, divideNodes, fenString );
        }

        Board_apply( board, copy );
    }
    Board_destroy( copy );

    MoveList_destroy( moveList );

    return nodes;
}
