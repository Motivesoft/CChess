#pragma once

#include "RuntimeSetup.h"

void Perft_depth( struct RuntimeSetup* runtimeSetup, int depth, const char* fen );
void Perft_fen( struct RuntimeSetup* runtimeSetup, const char* fenWithResults );
void Perft_file( struct RuntimeSetup* runtimeSetup, const char* filename );
