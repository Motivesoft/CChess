// CChess.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "stdafx.h"
#include <fcntl.h> 

#include "UCI.h"

#include "RuntimeSetup.h"
#include "Utility.h"

#define BUFFER_SIZE 4096

#define LOG_DEBUG( ... ) { log( runtimeSetup, DEBUG, __VA_ARGS__ ); }
#define LOG_INFO( ... ) { log( runtimeSetup, INFO, __VA_ARGS__ ); }
#define LOG_WARN( ... ) { log( runtimeSetup, WARN, __VA_ARGS__ ); }
#define LOG_ERROR( ... ) { log( runtimeSetup, ERROR, __VA_ARGS__ ); }

int main( int argc, char** argv )
{
    printf( "CChess\n" );

    errno_t err = 0;

    // Configure from command line args
    struct RuntimeSetup* runtimeSetup = RuntimeSetup_createRuntimeSetup();
    if( runtimeSetup == NULL )
    {
        fprintf( stderr, "Failed to allocate memory for the runtime setup\n" );
        return ENOMEM;
    }

    for ( int loop = 1; loop < argc && !err; loop++ )
    {
        if ( strcmp( argv[ loop ], "-input" ) == 0 )
        {
            if ( loop + 1 < argc )
            {
                err = RuntimeSetup_setInput( runtimeSetup, argv[ ++loop ] );
                if ( err != 0 )
                {
                    // Report the problem
                    LOG_ERROR( "Failed to open input file: %s (reason %d)", argv[ loop ], err );
                }
            }
            else
            {
                LOG_ERROR( "Missing input filename" );
                err = EINVAL;
            }
        }
        else if ( strcmp( argv[ loop ], "-output" ) == 0 )
        {
            if ( loop + 1 < argc )
            {
                err = RuntimeSetup_setOutput( runtimeSetup, argv[ ++loop ] );
                if ( err != 0 )
                {
                    // Report the problem
                    LOG_ERROR( "Failed to open output file: %s (reason %d)", argv[ loop ], err );
                    err = EINVAL;
                }
            }
            else
            {
                LOG_ERROR( "Missing output filename" );
                err = EINVAL;
            }
        }
        else if ( strcmp( argv[ loop ], "-logfile" ) == 0 )
        {
            if ( loop + 1 < argc )
            {
                err = RuntimeSetup_setLogger( runtimeSetup, argv[ ++loop ] );
                if ( err != 0 )
                {
                    // Report the problem
                    LOG_ERROR( "Failed to open log file: %s (reason %d)", argv[ loop ], err );
                    err = EINVAL;
                }
            }
            else
            {
                LOG_ERROR( "Missing log filename" );
                err = EINVAL;
            }
        }
        else if ( strcmp( argv[ loop ], "-debug" ) == 0 )
        {
            RuntimeSetup_setDebug( runtimeSetup, true );
        }
        else
        {
            LOG_ERROR( "Unrecognised argument: %s", argv[ loop ] );
            err = EINVAL;
        }
    }

    if ( !err )
    {
        struct UCIConfiguration* uci = UCI_createUCIConfiguration();
        if ( uci == NULL )
        {
            fprintf( stderr, "Failed to allocate memory for the UCI interface\n" );
            return ENOMEM;
        }

        // Process input
        char buffer[ BUFFER_SIZE ];
        while ( RuntimeSetup_getline( runtimeSetup, buffer, BUFFER_SIZE ) )
        {
            char* command;
            char* arguments;
            spliterate( buffer, &command, &arguments );

            // Ignore empty lines and comments
            if ( strlen( command ) == 0 || command[ 0 ] == '#' )
            {
                continue;
            }

            LOG_DEBUG( "Processing input: [%s][%s]", command, arguments );

            if ( !UCI_processCommand( uci, runtimeSetup, command, arguments ) )
            {
                // Break out of main loop and shut down
                break;
            }
        }

        UCI_shutdown( uci );
    }

    // Shutdown
    RuntimeSetup_shutdown( runtimeSetup );

    return err;
}
