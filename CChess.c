// CChess.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "stdafx.h"
#include <fcntl.h> 
#include <stdio.h> 
#include <string.h>

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

    bool error = false;
    for ( int loop = 1; loop < argc && !error; loop++ )
    {
        if ( strcmp( argv[ loop ], "-input" ) == 0 )
        {
            if ( loop + 1 < argc )
            {
                errno_t err = fopen_s( &input, argv[ ++loop ], "r" );
                if ( err != 0 )
                {
                    // Reset the setting to something meaningful
                    input = stdin;

                    // Report the problem
                    ERROR( logfile, "Failed to open input file: %s (reason %d)", argv[ loop ], err );
                    error = true;
                }
            }
            else
            {
                ERROR( logfile, "Missing input filename" );
                error = true;
            }
        }
        else if ( strcmp( argv[ loop ], "-logfile" ) == 0 )
        {
            if ( loop + 1 < argc )
            {
                errno_t err = fopen_s( &logfile, argv[ ++loop ], "w" );
                if ( err != 0 )
                {
                    // Reset the setting to something meaningful
                    logfile = stderr;

                    // Report the problem
                    ERROR( logfile, "Failed to open log file: %s (reason %d)", argv[ loop ], err );
                    error = true;
                }
            }
            else
            {
                ERROR( logfile, "Missing log filename" );
                error = true;
            }
        }
        else if ( strcmp( argv[ loop ], "-output" ) == 0 )
        {
            if ( loop + 1 < argc )
            {
                errno_t err = fopen_s( &output, argv[ ++loop ], "w" );
                if ( err != 0 )
                {
                    // Reset the setting to something meaningful
                    output = stdout;

                    // Report the problem
                    ERROR( logfile, "Failed to open output file: %s (reason %d)", argv[ loop ], err );
                    error = true;
                }
            }
            else
            {
                ERROR( logfile, "Missing output filename" );
                error = true;
            }
        }
        else
        {
            ERROR( logfile, "Unrecognised argument: %s", argv[ loop ] );
            error = true;
        }
    }

    if ( !error )
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

            if ( strcmp( line, "quit" ) == 0 )
            {
                break;
            }
        }
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

    return error ? -1 : 0;
}
