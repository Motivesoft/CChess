#include "stdafx.h"

#include "RuntimeSetup.h"

#define LOG_DEBUG( ... ) log( self, DEBUG, __VA_ARGS__ ) 
#define LOG_INFO( ... ) log( self, INFO, __VA_ARGS__ ) 
#define LOG_WARN( ... ) log( self, WARN, __VA_ARGS__ ) 
#define LOG_ERROR( ... ) log( self, ERROR, __VA_ARGS__ ) 

struct RuntimeSetup* RuntimeSetup_createRuntimeSetup()
{
    struct RuntimeSetup* runtimeSetup = malloc( sizeof( struct RuntimeSetup ) );

    if ( runtimeSetup )
    {
        RuntimeSetup_resetInput( runtimeSetup );
        RuntimeSetup_resetOutput( runtimeSetup );
        RuntimeSetup_resetLogger( runtimeSetup );

        runtimeSetup->debug = false;
    }

    return runtimeSetup;
}

void RuntimeSetup_destroy( struct RuntimeSetup* self )
{
    if ( self != NULL )
    {
        if ( self->input != stdin )
        {
            fclose( self->input );
        }
        if ( self->output != stdout )
        {
            fclose( self->output );
        }
        if ( self->logger != stderr )
        {
            fclose( self->logger );
        }

        free( self );
    }
}

void RuntimeSetup_resetInput( struct RuntimeSetup* self )
{
    self->input = stdin;
}

void RuntimeSetup_resetOutput( struct RuntimeSetup* self )
{
    self->output = stdout;
}

void RuntimeSetup_resetLogger( struct RuntimeSetup* self )
{
    self->logger = stderr;
}

FILE* RuntimeSetup_getInput( struct RuntimeSetup* self )
{
    return self->input;
}

FILE* RuntimeSetup_getOutput( struct RuntimeSetup* self )
{
    return self->output;
}

FILE* RuntimeSetup_getLogger( struct RuntimeSetup* self )
{
    return self->logger;
}

errno_t RuntimeSetup_setInput( struct RuntimeSetup* self, const char* filename )
{
    errno_t err = fopen_s( &self->input, filename, "r" );
    if ( err != 0 )
    {
        // Reset the setting to something meaningful
        RuntimeSetup_resetInput( self );
    }
    return err;
}

errno_t RuntimeSetup_setOutput( struct RuntimeSetup* self, const char* filename )
{
    errno_t err = fopen_s( &self->output, filename, "w" );
    if ( err != 0 )
    {
        // Reset the setting to something meaningful
        RuntimeSetup_resetOutput( self );
    }
    return err;
}

errno_t RuntimeSetup_setLogger( struct RuntimeSetup* self, const char* filename )
{
    errno_t err = fopen_s( &self->logger, filename, "w" );
    if ( err != 0 )
    {
        // Reset the setting to something meaningful
        RuntimeSetup_resetLogger( self );
    }
    return err;
}

void RuntimeSetup_setDebug( struct RuntimeSetup* self, bool debug )
{
    LOG_DEBUG( "Set debug %s", (debug ? "on" : "off") );
    self->debug = debug;
}

char* RuntimeSetup_getline( struct RuntimeSetup* self, char* buffer, int bufferSize )
{
    memset( buffer, 0, bufferSize );
    return fgets( buffer, bufferSize, self->input );
}

void log( struct RuntimeSetup* self, enum LogLevel level, const char* format, ... )
{
    if ( level == DEBUG && !self->debug )
    {
        // Not configured
        return;
    }

    static const char* label[] = { "DEBUG", "INFO ", "WARN ","ERROR" };
    static const char* color[] = { "\x1B[36m", "\x1B[32m", "\x1B[33m", "\x1B[31m" };

    va_list args;
    va_start( args, format );

    if ( self->logger == stderr )
    {
        fprintf( self->logger, "%s%s: ", color[ level ], label[ level ] );

        vfprintf( self->logger, format, args );
        fprintf( self->logger, "\033[0m\n" );
    }
    else
    {
        fprintf( self->logger, "%s: ", label[ level ] );
        vfprintf( self->logger, format, args );
        fprintf( self->logger, "\n" );
    }
    fflush( self->logger );

    va_end( args );
}
