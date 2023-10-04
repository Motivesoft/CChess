#include "stdafx.h"

#include "RuntimeSetup.h"

struct RuntimeSetup RuntimeSetup_createRuntimeSetup()
{
    struct RuntimeSetup runtimeSetup;

    RuntimeSetup_resetInput( &runtimeSetup );
    RuntimeSetup_resetOutput( &runtimeSetup );
    RuntimeSetup_resetLogger( &runtimeSetup );

    return runtimeSetup;
}

void RuntimeSetup_close( struct RuntimeSetup* runtimeSetup )
{
    if ( runtimeSetup->input != stdin )
    {
        fclose( runtimeSetup->input );
    }
    if ( runtimeSetup->output != stdout )
    {
        fclose( runtimeSetup->output );
    }
    if ( runtimeSetup->logger != stderr )
    {
        fclose( runtimeSetup->logger );
    }
}

void RuntimeSetup_resetInput( struct RuntimeSetup* runtimeSetup )
{
    runtimeSetup->input = stdin;
}

void RuntimeSetup_resetOutput( struct RuntimeSetup* runtimeSetup )
{
    runtimeSetup->output = stdout;
}

void RuntimeSetup_resetLogger( struct RuntimeSetup* runtimeSetup )
{
    runtimeSetup->logger = stderr;
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

char* RuntimeSetup_getline( struct RuntimeSetup* self, char* buffer, size_t bufferSize )
{
    memset( buffer, 0, bufferSize );
    return fgets( buffer, sizeof( buffer ), self->input );
}

void log( struct RuntimeSetup* self, enum LogLevel level, const char* format, ... )
{
    static const char* label[] = { "DEBUG","ERROR" };
    static const char* color[] = { "\x1B[36m","\x1B[31m" };

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

#define LOG_DEBUG( runtimeSetup, ... )  LOG( runtimeSetup, "\x1B[36m", "DEBUG", __VA_ARGS__ ) 
#define LOG_ERROR( runtimeSetup, ... )  LOG( runtimeSetup, "\x1B[31m", "ERROR", __VA_ARGS__ ) 
