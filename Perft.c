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

unsigned long long Perft_depth( struct RuntimeSetup* runtimeSetup, int depth, const char* fen, bool divide )
{
    LOG_DEBUG( "perft with depth %d and FEN: %s", depth, fen );

    Board board;
    Board_create( &board, fen );
    
    clock_t start = clock();

    unsigned long long count = Perft_run( runtimeSetup, &board, depth, divide );

    clock_t end = clock();

    float totalTime = (float) ( end - start ) / CLOCKS_PER_SEC;
    float nps = count / totalTime;

    LOG_INFO( "Move count: %llu in %0.3fs (%0.0f nps)", count, totalTime, nps );

    return count;
}

void Perft_fen( struct RuntimeSetup* runtimeSetup, char* fenWithResults )
{
    LOG_DEBUG( "perft with FEN: %s", fenWithResults );

    if ( strlen( fenWithResults ) == 0 )
    {
        LOG_ERROR( "Missing FEN string" );
        return;
    }

    // TODO
    // Split expected results from fen
    char* separator = strchr( fenWithResults, ';' );
    if ( separator != NULL )
    {
        // Split the string into FEN and results
        *separator++ = '\0';

        while ( separator != NULL && *separator != '\0' )
        {
            if ( *separator++ != 'D' )
            {
                LOG_ERROR( "Malformed expected result with: %s", fenWithResults );
                return;
            }

            int depth = atoi( separator );

            separator = strchr( separator, ' ' );
            if ( separator == NULL )
            {
                LOG_ERROR( "Malformed expected result with: %s", fenWithResults );
                return;
            }

            unsigned long long expectedResult = atoll( ++separator );

            if ( Perft_depth( runtimeSetup, depth, fenWithResults, false ) != expectedResult )
            {
                LOG_ERROR( "Failed: expected result was %llu", expectedResult );
            }
            else
            {
                LOG_INFO( "Success" );
            }

            separator = strchr( fenWithResults, ';' );
        }

        return;
    }
    
    separator = strchr( fenWithResults, ',' );
    if ( separator == NULL )
    {
        LOG_ERROR( "FEN string missing expected results: %s", fenWithResults );
        return;
    }


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

unsigned long long Perft_run( struct RuntimeSetup* runtimeSetup, Board* board, int depth, bool divide )
{
    return divide ? Perft_divide( runtimeSetup, board, depth ) : Perft_loop( runtimeSetup, board, depth );
}

unsigned long long Perft_loop( struct RuntimeSetup* runtimeSetup, Board* board, int depth )
{
    if ( depth == 0 )
    {
        return 1;
    }

    unsigned long long nodes = 0;

    MoveList moveList;
    moveList.count = 0;
    Board_generateMoves( board, &moveList );

    // This is a cheating optimisation
    if ( depth == 1 )
    {
        return moveList.count;
    }

    Board copy;
    Board_copy( board, &copy );

    for ( unsigned char loop = 0; loop < moveList.count; loop++ )
    {
        if ( Board_makeMove( board, moveList.moves[ loop ] ) )
        {
            nodes += Perft_loop( runtimeSetup, board, depth - 1 );
        }

        Board_apply( board, &copy );
    }

    return nodes;
}

unsigned long long Perft_divide( struct RuntimeSetup* runtimeSetup, Board* board, int depth )
{
    if ( depth == 0 )
    {
        return 1;
    }

    unsigned long long nodes = 0;
    unsigned long long divideNodes = 0;

    char moveString[ 10 ];
    char fenString[ 256 ];

    MoveList moveList;
    moveList.count = 0;
    Board_generateMoves( board, &moveList );

    Board copy;
    Board_copy( board, &copy );

    for ( unsigned char loop = 0; loop < moveList.count; loop++ )
    {
        if ( Board_makeMove( board, moveList.moves[ loop ] ) )
        {
            divideNodes = Perft_loop( runtimeSetup, board, depth - 1 );
            nodes += divideNodes;

            Board_exportMove( moveList.moves[ loop ], moveString );
            Board_exportBoard( board, fenString );

            LOG_INFO( "  %s : %llu - %s", moveString, divideNodes, fenString );
        }

        Board_apply( board, &copy );
    }

    return nodes;
}
