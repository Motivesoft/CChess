#pragma once

#include "stdafx.h"

struct RuntimeSetup
{
    FILE* input;
    FILE* output;
    FILE* logger;
};

// Control methods

struct RuntimeSetup RuntimeSetup_createRuntimeSetup();
void RuntimeSetup_close( struct RuntimeSetup* self );

// Internal methods

void RuntimeSetup_resetInput( struct RuntimeSetup* self );
void RuntimeSetup_resetOutput( struct RuntimeSetup* self );
void RuntimeSetup_resetLogger( struct RuntimeSetup* self );

// Access methods

FILE* RuntimeSetup_getInput( struct RuntimeSetup* self );
FILE* RuntimeSetup_getOutput( struct RuntimeSetup* self );
FILE* RuntimeSetup_getLogger( struct RuntimeSetup* self );

errno_t RuntimeSetup_setInput( struct RuntimeSetup* self, const char* filename );
errno_t RuntimeSetup_setOutput( struct RuntimeSetup* self, const char* filename );
errno_t RuntimeSetup_setLogger( struct RuntimeSetup* self, const char* filename );

// Helper methods

char* RuntimeSetup_getline( struct RuntimeSetup* self, char* buffer, int bufferSize );

enum LogLevel
{
    DEBUG,
    ERROR
};

void log( struct RuntimeSetup* self, enum LogLevel level, const char* format, ... );