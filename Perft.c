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
}

void Perft_fen( struct RuntimeSetup* runtimeSetup, const char* fenWithResults )
{
    LOG_DEBUG( "perft with FEN: %s", fenWithResults );
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
