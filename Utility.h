#pragma once

#define LOG( logger, color, level, ... )           \
{                                                  \
    if ( logger == stderr )                        \
    {                                              \
        fprintf( logger, "%s%s: ", color, level ); \
        fprintf( logger, __VA_ARGS__ );            \
        fprintf( logger, "\033[0m\n" );            \
    }                                              \
    else                                           \
    {                                              \
        fprintf( logger, "%s: ", level );          \
        fprintf( logger, __VA_ARGS__ );            \
        fprintf( logger, "\n" );                   \
    }                                              \
    fflush( logger );                              \
}

#define LOG_DEBUG( runtimeSetup, ... )  LOG( runtimeSetup, "\x1B[36m", "DEBUG", __VA_ARGS__ ) 
#define LOG_ERROR( runtimeSetup, ... )  LOG( runtimeSetup, "\x1B[31m", "ERROR", __VA_ARGS__ ) 

char* trim( char* string );
char* sanitize( char* string );

typedef void( *WriteToFile )( char* format, ... );
