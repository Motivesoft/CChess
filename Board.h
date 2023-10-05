#pragma once

struct PieceList
{
    unsigned long long bbPawn;
    unsigned long long bbKnight;
    unsigned long long bbBishop;
    unsigned long long bbRook;
    unsigned long long bbQueen;
    unsigned long long bbKing;
    unsigned char king;
    bool kingsideCastling;
    bool queensideCastling;
};

struct Board
{
    unsigned char squares[ 64 ];
    struct PieceList whitePieces;
    struct PieceList blackPieces;
    bool whiteToMove;
    unsigned char enPassantSquare;
    unsigned short halfmoveClock;
    unsigned short fullmoveNumber;
};

struct Board* Board_create( const char* fen );
void Board_shutdown( struct Board* self );

// Internal methods

void Board_clearBoard( struct Board* self );

typedef void ( *FenProcessor )( struct Board* self, const char* fenSection );

void Board_processBoardLayout( struct Board* self, const char* fenSection );
void Board_processActiveColor( struct Board* self, const char* fenSection );
void Board_processCastlingRights( struct Board* self, const char* fenSection );
void Board_processEnPassantSquare( struct Board* self, const char* fenSection );
void Board_processHalfmoveClock( struct Board* self, const char* fenSection );
void Board_processFullmoveNumber( struct Board* self, const char* fenSection );
