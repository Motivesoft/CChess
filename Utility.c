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

#include "Utility.h"

char* trim( char* string )
{
    char* result = string;

    if ( strlen( string ) )
    {
        // Remove leading spaces
        while ( strlen( result ) && isspace( result[ 0 ] ) )
        {
            result++;
        }

        // Shift the data back to the start of the string
        memcpy( string, result, strlen( result ) + 1 );

        // Remove trailing spaces
        while ( strlen( result ) && isspace( result[ strlen( result ) - 1 ] ) )
        {
            result[ strlen( result ) - 1 ] = '\0';
        }
    }

    return string;
}

char* sanitize( char* string )
{
    char* result = string;

    // Get rid of leading and trailing spaces
    string = trim( string );

    for ( result = string; *result != '\0'; )
    {
        if ( isspace( *result ) )
        {
            // If we find multiple spaces, collapse them down to one
            while ( isspace( *( result + 1 ) ) )
            {
                memcpy( result, result + 1, strlen( result ) );
            }

            // Make sure all isspace characters are actual spaces
            *result++ = ' ';
        }
        else
        {
            // Non-space, step over
            result++;
        }
    }

    return string;
}

bool spliterate( char* line, char** command, char** arguments )
{
    bool result = false;

    // Make sure the line is trimmed and contains no double spaces, tabs etc
    line = sanitize( line );

    // By default, expect command to be all of line, and arguments to be zero-length (pointing to the null at the end)
    *command = line;
    *arguments = line + strlen( line );

    for ( int loop = 0; loop < strlen( line ); loop++ )
    {
        if ( isspace( line[ loop ] ) )
        {
            line[ loop ] = '\0';
            *arguments = &line[ loop + 1 ];

            result = true;
            break;
        }
    }

    return result;
}

unsigned char squareToIndex( const char* square )
{
    return ( ( square[ 1 ] - '1' ) << 3 ) + ( square[ 0 ] - 'a' );
}

void indexToSquare( unsigned char index, char** square )
{
    *square[ 0 ] = 'a' + ( index & 0b00000111 );
    *square[ 1 ] = '1' + ( ( index >> 3 ) & 0b00000111 );
    *square[ 2 ] = '\0';
}
