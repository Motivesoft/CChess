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

typedef bool ( *UciCommandHandler )( struct UCIConfiguration* self, struct RuntimeSetup* runtimeSetup, char* arguments );

struct UciCommandHandler
{
    const char* command;
    UciCommandHandler handler;
};

struct UciCommandHandler uciCommandHandlers[];

bool UCI_processCommand( struct UCIConfiguration* self, struct RuntimeSetup* runtimeSetup, const char* command, char* arguments );

bool UCI_uci( struct UCIConfiguration* self, struct RuntimeSetup* runtimeSetup, char* arguments );
bool UCI_debug( struct UCIConfiguration* self, struct RuntimeSetup* runtimeSetup, char* arguments );
bool UCI_isready( struct UCIConfiguration* self, struct RuntimeSetup* runtimeSetup, char* arguments );
bool UCI_setoption( struct UCIConfiguration* self, struct RuntimeSetup* runtimeSetup, char* arguments );
bool UCI_register( struct UCIConfiguration* self, struct RuntimeSetup* runtimeSetup, char* arguments );
bool UCI_ucinewgame( struct UCIConfiguration* self, struct RuntimeSetup* runtimeSetup, char* arguments );
bool UCI_position( struct UCIConfiguration* self, struct RuntimeSetup* runtimeSetup, char* arguments );
bool UCI_go( struct UCIConfiguration* self, struct RuntimeSetup* runtimeSetup, char* arguments );
bool UCI_stop( struct UCIConfiguration* self, struct RuntimeSetup* runtimeSetup, char* arguments );
bool UCI_ponderhit( struct UCIConfiguration* self, struct RuntimeSetup* runtimeSetup, char* arguments );
bool UCI_quit( struct UCIConfiguration* self, struct RuntimeSetup* runtimeSetup, char* arguments );

bool UCI_perft( struct UCIConfiguration* self, struct RuntimeSetup* runtimeSetup, char* arguments );
bool UCI_test( struct UCIConfiguration* self, struct RuntimeSetup* runtimeSetup, char* arguments );
