#pragma once

#include "Board.h"
#include "RuntimeSetup.h"

// Public methods 

void Perft_depth( struct RuntimeSetup* runtimeSetup, int depth, const char* fen );
void Perft_fen( struct RuntimeSetup* runtimeSetup, const char* fenWithResults );
void Perft_file( struct RuntimeSetup* runtimeSetup, const char* filename );

// Internal methods

unsigned long Perft_run( struct Board* board, int depth, bool divide );
unsigned long Perft_loop( struct Board* board, int depth );
unsigned long Perft_divide( struct Board* board, int depth );