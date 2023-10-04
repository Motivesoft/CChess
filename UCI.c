#include "stdafx.h"

#include "UCI.h"

struct UCIConfiguration UCI_createUCIConfiguration()
{
    struct UCIConfiguration uci;

    // Starting position
    uci.fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    return uci;
}