#pragma once

#include "RuntimeSetup.h"
#include "Utility.h"

struct UCIConfiguration
{
    char* fen;
} UCIConfiguration;

// Control methods

struct UCIConfiguration UCI_createUCIConfiguration();
void UCI_shutdown( struct UCIConfiguration* self );

// UCI methods

typedef bool ( *UciCommandHandler )( struct UCIConfiguration* self, struct RuntimeSetup* runtimeSetup, const char* arguments );

struct UciCommandHandler
{
    const char* command;
    UciCommandHandler handler;
};

struct UciCommandHandler uciCommandHandlers[];

bool UCI_processCommand( struct UCIConfiguration* self, struct RuntimeSetup* runtimeSetup, const char* command, const char* arguments );

bool UCI_quit( struct UCIConfiguration* self, struct RuntimeSetup* runtimeSetup, const char* arguments );
bool UCI_uci( struct UCIConfiguration* self, struct RuntimeSetup* runtimeSetup, const char* arguments );