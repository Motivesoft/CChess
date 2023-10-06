#pragma once

#include "Move.h"

/// <summary>
/// Represents each piece, with some padding so that WHITE_x and BLACK_x differ only in the msb
/// </summary>
enum Piece
{
    EMPTY = 0,
    WHITE_PAWN,
    WHITE_KNIGHT,
    WHITE_BISHOP,
    WHITE_ROOK,
    WHITE_QUEEN,
    WHITE_KING,
    UNUSED_1,
    UNUSED_2,
    BLACK_PAWN,
    BLACK_KNIGHT,
    BLACK_BISHOP,
    BLACK_ROOK,
    BLACK_QUEEN,
    BLACK_KING,
    UNUSED_3,
};

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

unsigned long knightDirections[ 64 ][ 8 ];

struct Board* Board_create( const char* fen );
void Board_destroy( struct Board* self );

// Internal methods

void Board_initialize( struct Board* self );

/// <summary>
/// Reset the content of the board to nothing so it can be populated from a FEN string
/// </summary>
/// <param name="self">the board</param>
void Board_clearBoard( struct Board* self );

/// <summary>
/// Print the content of the board
/// </summary>
/// <param name="self">the board</param>
void Board_printBoard( struct Board* self );

/// <summary>
/// Export the content of the board to a FEN string.
/// This assumes the provided character buffer is of sufficient length
/// </summary>
/// <param name="self">the board</param>
/// <param name="fen">the buffer to receive the FEN output</param>
void Board_exportBoard( struct Board* self, char* fen );

void Board_exportMove( struct Move* move, char* moveString );

typedef void ( *FenProcessor )( struct Board* self, const char* fenSection );

void Board_processBoardLayout( struct Board* self, const char* fenSection );
void Board_processActiveColor( struct Board* self, const char* fenSection );
void Board_processCastlingRights( struct Board* self, const char* fenSection );
void Board_processEnPassantSquare( struct Board* self, const char* fenSection );
void Board_processHalfmoveClock( struct Board* self, const char* fenSection );
void Board_processFullmoveNumber( struct Board* self, const char* fenSection );

unsigned long rankFromIndex( unsigned long index );
unsigned long fileFromIndex( unsigned long index );

/// <summary>
/// Is the square at index empty?
/// </summary>
/// <param name="self">the board</param>
/// <param name="index">the location</param>
bool empty( struct Board* self, unsigned long index );

/// <summary>
/// Is the piece at index a friendly piece?
/// </summary>
/// <param name="self">the board</param>
/// <param name="index">the location</param>
bool friendly( struct Board* self, unsigned long index );

/// <summary>
/// Is the piece at index an attacker?
/// </summary>
/// <param name="self">the board</param>
/// <param name="index">the location</param>
bool attacker( struct Board* self, unsigned long index );

/// <summary>
/// Generate the pseudolegal moves for the current position
/// </summary>
/// <param name="self">the board</param>
struct MoveList* Board_generateMoves( struct Board* self );

/// <summary>
/// Makes a move but returns false if the move is illegal
/// </summary>
/// <param name="self">the board</param>
/// <param name="move">the pseudolegal move</param>
bool Board_makeMove( struct Board* self, struct Move* move );

/// <summary>
/// Reverts the board state
/// </summary>
/// <param name="self">the board</param>
void Board_unmakeMove( struct Board* self );

void Board_generatePawnMoves( struct Board* self, struct MoveList* moveList );
void Board_generateKnightMoves( struct Board* self, struct MoveList* moveList );
