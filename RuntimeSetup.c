#include "stdafx.h"

#include "RuntimeSetup.h"

struct RuntimeSetup RuntimeSetup_create()
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
