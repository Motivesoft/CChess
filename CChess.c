// CChess.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "stdafx.h"
#include <fcntl.h> 

#include "UCI.h"

#include "Utility.h"

#define BUFFER_SIZE 4096

#define LOG( file, color, level, ... )           \
{                                                \
    if ( file == stderr )                        \
    {                                            \
        fprintf( file, "%s%s: ", color, level ); \
        fprintf( file, __VA_ARGS__ );            \
        fprintf( file, "\033[0m\n" );            \
    }                                            \
    else                                         \
    {                                            \
        fprintf( file, "%s: ", level );          \
        fprintf( file, __VA_ARGS__ );            \
        fprintf( file, "\n" );                   \
    }                                            \
    fflush( file );                              \
}

#define DEBUG( file, ... )  LOG( file, "\x1B[36m", "DEBUG", __VA_ARGS__ ) 
#define ERROR( file, ... )  LOG( file, "\x1B[31m", "ERROR", __VA_ARGS__ ) 

int main( int argc, char** argv )
{
    printf( "CChess\n" );

    // Configure from command line args
    FILE* input = stdin;
    FILE* output = stdout;
    FILE* logfile = stderr;

    errno_t err = 0;
    for ( int loop = 1; loop < argc && !err; loop++ )
    {
        if ( strcmp( argv[ loop ], "-input" ) == 0 )
        {
            if ( loop + 1 < argc )
            {
                err = fopen_s( &input, argv[ ++loop ], "r" );
                if ( err != 0 )
                {
                    // Reset the setting to something meaningful
                    input = stdin;

                    // Report the problem
                    ERROR( logfile, "Failed to open input file: %s (reason %d)", argv[ loop ], err );
                }
            }
            else
            {
                ERROR( logfile, "Missing input filename" );
                err = EINVAL;
            }
        }
        else if ( strcmp( argv[ loop ], "-output" ) == 0 )
        {
            if ( loop + 1 < argc )
            {
                err = fopen_s( &output, argv[ ++loop ], "w" );
                if ( err != 0 )
                {
                    // Reset the setting to something meaningful
                    output = stdout;

                    // Report the problem
                    ERROR( logfile, "Failed to open output file: %s (reason %d)", argv[ loop ], err );
                    err = EINVAL;
                }
            }
            else
            {
                ERROR( logfile, "Missing output filename" );
                err = EINVAL;
            }
        }
        else if ( strcmp( argv[ loop ], "-logfile" ) == 0 )
        {
            if ( loop + 1 < argc )
            {
                err = fopen_s( &logfile, argv[ ++loop ], "w" );
                if ( err != 0 )
                {
                    // Reset the setting to something meaningful
                    logfile = stderr;

                    // Report the problem
                    ERROR( logfile, "Failed to open log file: %s (reason %d)", argv[ loop ], err );
                    err = EINVAL;
                }
            }
            else
            {
                ERROR( logfile, "Missing log filename" );
                err = EINVAL;
            }
        }
        else
        {
            ERROR( logfile, "Unrecognised argument: %s", argv[ loop ] );
            err = EINVAL;
        }
    }

    if ( !err )
    {
        struct UCIConfiguration uci = UCI_createUCIConfiguration();

        // Process input
        char buffer[ BUFFER_SIZE ];
        char* line;
        memset( buffer, 0, BUFFER_SIZE );
        while ( fgets( buffer, sizeof( buffer ), input ) )
        {
            line = sanitize( buffer );

            DEBUG( logfile, "> %s", line );

            // Ignore empty lines and comments
            if ( strlen( line ) == 0 || line[ 0 ] == '#' )
            {
                continue;
            }

            if ( strcmp( line, "uci" ) == 0 )
            {
                UCI_uci( &uci, output );
                break;
            }
            else if ( strcmp( line, "quit" ) == 0 )
            {
                UCI_quit( &uci );
                break;
            }
            else
            {
                // Ignore any unknown commands
                ERROR( logfile, "Unrecognised input: %s", line );
            }
        }

        UCI_shutdown( &uci );
    }

    // Shutdown

    if ( input != stdin )
    {
        fclose( input );
    }

    if ( output != stdin )
    {
        fclose( output );
    }

    if ( logfile != stderr )
    {
        fclose( logfile );
    }

    return err;
}
