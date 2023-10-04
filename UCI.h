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

void UCI_quit( struct UCIConfiguration* self );
void UCI_uci( struct UCIConfiguration* self, struct RuntimeSetup* runtimeSetup );