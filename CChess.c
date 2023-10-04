// CChess.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "stdafx.h"
#include <fcntl.h> 
#include <stdio.h> 
#include <string.h>

#include "UCI.h"

#include "Utility.h"

#define BUFFER_SIZE 4096

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
                    fprintf( logfile, "Failed to open input file: %s (reason %d)", argv[ loop ], err );
                    error = true;

                    // Reset the setting to something meaningful
                    input = stdin;
                }
            }
            else
            {
                fprintf( logfile, "Missing input filename" );
                error = true;
            }
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
