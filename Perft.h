#pragma once

#include "Board.h"
#include "RuntimeSetup.h"

// Public methods 

unsigned long long Perft_depth( struct RuntimeSetup* runtimeSetup, int depth, const char* fen, bool divide );
void Perft_fen( struct RuntimeSetup* runtimeSetup, char* fenWithResults );
void Perft_file( struct RuntimeSetup* runtimeSetup, const char* filename );

// Internal methods

unsigned long long Perft_run( struct RuntimeSetup* runtimeSetup, Board* board, int depth, bool divide );
unsigned long long Perft_loop( struct RuntimeSetup* runtimeSetup, Board* board, int depth );
unsigned long long Perft_divide( struct RuntimeSetup* runtimeSetup, Board* board, int depth );