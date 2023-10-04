// CChess.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "stdafx.h"
#include <fcntl.h> 

#include "UCI.h"

#include "RuntimeSetup.h"
#include "Utility.h"

#define BUFFER_SIZE 4096

int main( int argc, char** argv )
{
    printf( "CChess\n" );

    // Configure from command line args
    struct RuntimeSetup runtimeSetup = RuntimeSetup_create();

    errno_t err = 0;
    for ( int loop = 1; loop < argc && !err; loop++ )
    {
        if ( strcmp( argv[ loop ], "-input" ) == 0 )
        {
            if ( loop + 1 < argc )
            {
                err = RuntimeSetup_setInput( &runtimeSetup, argv[ ++loop ] );
                if ( err != 0 )
                {
                    // Report the problem
                    LOG_ERROR( runtimeSetup.logger, "Failed to open input file: %s (reason %d)", argv[ loop ], err );
                }
            }
            else
            {
                LOG_ERROR( runtimeSetup.logger, "Missing input filename" );
                err = EINVAL;
            }
        }
        else if ( strcmp( argv[ loop ], "-output" ) == 0 )
        {
            if ( loop + 1 < argc )
            {
                err = RuntimeSetup_setOutput( &runtimeSetup, argv[ ++loop ] );
                if ( err != 0 )
                {
                    // Report the problem
                    LOG_ERROR( runtimeSetup.logger, "Failed to open output file: %s (reason %d)", argv[ loop ], err );
                    err = EINVAL;
                }
            }
            else
            {
                LOG_ERROR( runtimeSetup.logger, "Missing output filename" );
                err = EINVAL;
            }
        }
        else if ( strcmp( argv[ loop ], "-logfile" ) == 0 )
        {
            if ( loop + 1 < argc )
            {
                err = RuntimeSetup_setLogger( &runtimeSetup, argv[ ++loop ] );
                if ( err != 0 )
                {
                    // Report the problem
                    LOG_ERROR( runtimeSetup.logger, "Failed to open log file: %s (reason %d)", argv[ loop ], err );
                    err = EINVAL;
                }
            }
            else
            {
                LOG_ERROR( runtimeSetup.logger, "Missing log filename" );
                err = EINVAL;
            }
        }
        else
        {
            LOG_ERROR( runtimeSetup.logger, "Unrecognised argument: %s", argv[ loop ] );
            err = EINVAL;
        }
    }

    if ( !err )
    {
        struct UCIConfiguration uci = UCI_createUCIConfiguration();

        // Process input
        char buffer[ BUFFER_SIZE ];
        char* line;
        while ( RuntimeSetup_getline( &runtimeSetup, buffer, sizeof( buffer ) ) )
        {
            line = sanitize( buffer );

            LOG_DEBUG( runtimeSetup.logger, "> %s", line );

            // Ignore empty lines and comments
            if ( strlen( line ) == 0 || line[ 0 ] == '#' )
            {
                continue;
            }

            if ( strcmp( line, "uci" ) == 0 )
            {
                UCI_uci( &uci, &runtimeSetup );
            }
            else if ( strcmp( line, "quit" ) == 0 )
            {
                UCI_quit( &uci );
                break;
            }
            else
            {
                // Ignore any unknown commands
                LOG_ERROR( runtimeSetup.logger, "Unrecognised input: %s", line );
            }
        }

        UCI_shutdown( &uci );
    }

    // Shutdown
    RuntimeSetup_close( &runtimeSetup );

    return err;
}
