#include "stdafx.h"

#include "Perft.h"
#include "Utility.h"

#define BUFFER_SIZE 256

#define LOG_DEBUG( ... ) { log( runtimeSetup, DEBUG, __VA_ARGS__ ); }
#define LOG_INFO( ... ) { log( runtimeSetup, INFO, __VA_ARGS__ ); }
#define LOG_WARN( ... ) { log( runtimeSetup, WARN, __VA_ARGS__ ); }
#define LOG_ERROR( ... ) { log( runtimeSetup, ERROR, __VA_ARGS__ ); }

void Perft_depth( struct RuntimeSetup* runtimeSetup, int depth, const char* fen )
{
    LOG_DEBUG( "perft with depth %d and FEN: %s", depth, fen );

    // TODO
    struct Board* board = Board_create( fen );

    if ( board == NULL )
    {
        LOG_ERROR( "Cannot allocate memory for perft test" );
        return;
    }
    
    unsigned long count = Perft_run( board, depth, true );
    fprintf( runtimeSetup->logger, "Move count: %u\n", count );
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
    return divide ? Perft_loop( board, depth ) : Perft_divide( board, depth );
}

unsigned long Perft_loop( struct Board* board, int depth )
{
    return 1;
}

unsigned long Perft_divide( struct Board* board, int depth )
{
    return 1;
}
