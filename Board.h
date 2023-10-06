#pragma once

#include "Move.h"

enum Squares
{
    A8 = 0x38, B8 = 0x39, C8 = 0x3a, D8 = 0x3b, E8 = 0x3c, F8 = 0x3d, G8 = 0x3e, H8 = 0x3f,
    A7 = 0x30, B7 = 0x31, C7 = 0x32, D7 = 0x33, E7 = 0x34, F7 = 0x35, G7 = 0x36, H7 = 0x37,
    A6 = 0x28, B6 = 0x29, C6 = 0x2a, D6 = 0x2b, E6 = 0x2c, F6 = 0x2d, G6 = 0x2e, H6 = 0x2f,
    A5 = 0x20, B5 = 0x21, C5 = 0x22, D5 = 0x23, E5 = 0x24, F5 = 0x25, G5 = 0x26, H5 = 0x27,
    A4 = 0x18, B4 = 0x19, C4 = 0x1a, D4 = 0x1b, E4 = 0x1c, F4 = 0x1d, G4 = 0x1e, H4 = 0x1f,
    A3 = 0x10, B3 = 0x11, C3 = 0x12, D3 = 0x13, E3 = 0x14, F3 = 0x15, G3 = 0x16, H3 = 0x17,
    A2 = 0x08, B2 = 0x09, C2 = 0x0a, D2 = 0x0b, E2 = 0x0c, F2 = 0x0d, G2 = 0x0e, H2 = 0x0f,
    A1 = 0x00, B1 = 0x01, C1 = 0x02, D1 = 0x03, E1 = 0x04, F1 = 0x05, G1 = 0x06, H1 = 0x07,
};

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

// To correspond with the Piece enum
enum ColorlessPiece
{
    PAWN = 1,
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN,
    KING
};

struct PieceList
{
    unsigned long long bbPawn;
    unsigned long long bbKnight;
    unsigned long long bbBishop;
    unsigned long long bbRook;
    unsigned long long bbQueen;
    unsigned long long bbKing;
    unsigned long long bbAll;
    unsigned long king;
    bool kingsideCastling;
    bool queensideCastling;
};

struct Board
{
    unsigned char squares[ 64 ];
    struct PieceList whitePieces;
    struct PieceList blackPieces;
    bool whiteToMove;
    unsigned long enPassantSquare;
    unsigned short halfmoveClock;
    unsigned short fullmoveNumber;
};

struct Board* Board_create( const char* fen );
void Board_destroy( struct Board* self );

// Internal methods
struct Board* Board_copy( struct Board* self );
void Board_apply( struct Board* self, struct Board* other );
bool Board_compare( struct Board* self, struct Board* other );

/// <summary>
/// Initialise static structures. Need only be called once but should cope with multiple
/// calls if it can
/// </summary>
static void Board_initialize();

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
bool Board_isEmptySquare( struct Board* self, unsigned long index );

/// <summary>
/// Is the piece at index a friendly piece?
/// </summary>
/// <param name="self">the board</param>
/// <param name="index">the location</param>
bool Board_containsFriendly( struct Board* self, unsigned long index );

/// <summary>
/// Is the piece at index an attacker?
/// </summary>
/// <param name="self">the board</param>
/// <param name="index">the location</param>
bool Board_containsAttacker( struct Board* self, unsigned long index );

bool Board_isPawn( enum Piece piece );
bool Board_isKnight( enum Piece piece );
bool Board_isBishop( enum Piece piece );
bool Board_isRook( enum Piece piece );
bool Board_isQueen( enum Piece piece );
bool Board_isKing( enum Piece piece );

/// <summary>
/// Remove contents of square
/// </summary>
/// <param name="self">the board</param>
/// <param name="pieceList">the current incumbent or NULL if unknown</param>
/// <param name="index">the location</param>
void Board_clearSquare( struct Board* self, struct PieceList* pieceList, unsigned long index );

/// <summary>
/// Remove contents of square
/// </summary>
/// <param name="self">the board</param>
/// <param name="pieceList">the piece list or NULL for unknown</param>
/// <param name="piece">the piece</param>
/// <param name="index">the location</param>
void Board_setSquare( struct Board* self, struct PieceList* pieceList, enum Piece piece, unsigned long index );

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

void Board_generatePawnMoves( struct Board* self, struct MoveList* moveList );
void Board_generateKnightMoves( struct Board* self, struct MoveList* moveList );
