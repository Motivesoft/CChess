#include <io.h>
#include <ctype.h>
#include <errno.h>
#include <intrin.h>
#include <memory.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "Board.h"
#include "Utility.h"

#define OFF_BOARD UCHAR_MAX

static const char pieceNames[] = " PNBRQK  pnbrqk ";

// This is expected to give us 0b00001000 and 0b00000111
static const char COLOR_BIT = BLACK_PAWN - WHITE_PAWN;
static const char COLOR_MASK = BLACK_PAWN - WHITE_PAWN - 1;

static unsigned long knightDirections[ 64 ][ 8 ];
static unsigned long kingDirections[ 64 ][ 8 ];

void Board_create( Board* board, const char* fen )
{
    Board_initialize( board );
    Board_clearBoard( board );

    // FEN string is multiple space-separated sections as:
    // - piece layout
    // - active color
    // - castling rights
    // - en passant square
    // - halfmove clock (will sometimes be absent - e.g. in epd strings)
    // - fullmove number (will sometimes be absent - e.g. in epd strings)
    //
    // As UCI is a programmatic API, it is safe to assume the FEN string is valid

    // Set these to defaults in case they are not specified in the string
    FenProcessor fenProcessors[] =
    {
        Board_processBoardLayout,
        Board_processActiveColor,
        Board_processCastlingRights,
        Board_processEnPassantSquare,
        Board_processHalfmoveClock,
        Board_processFullmoveNumber,
    };
    unsigned short processor = 0;

    char* fenCopy = _strdup( fen );
    char* section;
    char* nextToken = NULL;
    section = strtok_s( fenCopy, " ", &nextToken );
        
    while ( section != NULL )
    {
        fenProcessors[ processor++ ]( board, section );

        section = strtok_s( NULL, " ", &nextToken );
    }

    free( fenCopy );

    Board_printBoard( board );
    char x[ 256 ];
    Board_exportBoard( board, x );
    printf( "%s\n", x );
}

static void Board_initialize()
{
    // Create an array of knight moves and record which are possible from each starting square
    short knightMoves[8][2] =
    { 
        { -2, -1 },
        { -2, +1 },
        { -1, -2 },
        { -1, +2 },
        { +1, -2 },
        { +1, +2 },
        { +2, -1 },
        { +2, +1 },
    };

    for ( short rank = 0; rank < 8; rank++ )
    {
        for ( short file = 0; file < 8; file++ )
        {
            short index = ( rank * 8 ) + file;

            for ( short loop = 0; loop < 8; loop++ )
            {
                // 0 means "not valid from here"
                knightDirections[ index ][ loop ] = 0;

                // Filter the off-board moves
                if ( file + knightMoves[ loop ][ 0 ] < 0 || file + knightMoves[ loop ][ 0 ] > 7 )
                {
                    continue;
                }
                if ( rank + knightMoves[ loop ][ 1 ] < 0 || rank + knightMoves[ loop ][ 1 ] > 7 )
                {
                    continue;
                }

                // What offset can we move to from here?
                knightDirections[ index ][ loop ] = (knightMoves[ loop ][ 1 ] << 3) + knightMoves[ loop ][ 0 ];
            }
        }
    }

    // Create an array of knight moves and record which are possible from each starting square
    short kingMoves[ 8 ][ 2 ] =
    {
        { -1, -1 },
        { -1,  0 },
        { -1, +1 },
        {  0, -1 },
        {  0, +1 },
        { +1, -1 },
        { +1,  0 },
        { +1, +1 },
    };

    for ( short rank = 0; rank < 8; rank++ )
    {
        for ( short file = 0; file < 8; file++ )
        {
            short index = ( rank * 8 ) + file;

            for ( short loop = 0; loop < 8; loop++ )
            {
                // 0 means "not valid from here"
                kingDirections[ index ][ loop ] = 0;

                // Filter the off-board moves
                if ( file + kingMoves[ loop ][ 0 ] < 0 || file + kingMoves[ loop ][ 0 ] > 7 )
                {
                    continue;
                }
                if ( rank + kingMoves[ loop ][ 1 ] < 0 || rank + kingMoves[ loop ][ 1 ] > 7 )
                {
                    continue;
                }

                // What offset can we move to from here?
                kingDirections[ index ][ loop ] = ( kingMoves[ loop ][ 1 ] << 3 ) + kingMoves[ loop ][ 0 ];
            }
        }
    }
}

void Board_clearBoard( Board* self )
{
    self->whitePieces.bbPawn = 0;
    self->whitePieces.bbKnight = 0;
    self->whitePieces.bbBishop = 0;
    self->whitePieces.bbRook = 0;
    self->whitePieces.bbQueen = 0;
    self->whitePieces.bbKing = 0;
    self->whitePieces.bbAll = 0;
    self->whitePieces.king = OFF_BOARD;

    self->blackPieces.bbPawn = 0;
    self->blackPieces.bbKnight = 0;
    self->blackPieces.bbBishop = 0;
    self->blackPieces.bbRook = 0;
    self->blackPieces.bbQueen = 0;
    self->blackPieces.bbKing = 0;
    self->blackPieces.bbAll = 0;
    self->blackPieces.king = OFF_BOARD;

    self->whiteToMove = true;

    self->whitePieces.kingsideCastling = false;
    self->whitePieces.queensideCastling = false;
    self->blackPieces.kingsideCastling = false;
    self->blackPieces.queensideCastling = false;

    self->enPassantSquare = OFF_BOARD;

    self->halfmoveClock = 0;
    self->fullmoveNumber = 0;

    for ( unsigned short index = 0; index < 64; index++ )
    {
        self->squares[ index ] = EMPTY;
    }
}

void Board_processBoardLayout( Board* self, const char* fenSection ) 
{
    int rank = 7;
    int file = 0;

    char* section = _strdup( fenSection );
    char* nextToken = NULL;
    char* rankSpecification = strtok_s( section, "/", &nextToken );

    while ( rankSpecification != NULL )
    {
        while ( *rankSpecification != '\0' )
        {
            unsigned char index = ( rank << 3 ) + file++;

            char item = *rankSpecification++;
            switch ( item )
            {
                case 'P':
                    self->whitePieces.bbPawn |= (1ull << index);
                    self->squares[ index ] = WHITE_PAWN;
                    break;

                case 'N':
                    self->whitePieces.bbKnight |= (1ull << index);
                    self->squares[ index ] = WHITE_KNIGHT;
                    break;

                case 'B':
                    self->whitePieces.bbBishop |= (1ull << index);
                    self->squares[ index ] = WHITE_BISHOP;
                    break;

                case 'R':
                    self->whitePieces.bbRook |= (1ull << index);
                    self->squares[ index ] = WHITE_ROOK;
                    break;

                case 'Q':
                    self->whitePieces.bbQueen |= (1ull << index);
                    self->squares[ index ] = WHITE_QUEEN;
                    break;

                case 'K':
                    self->whitePieces.bbKing |= (1ull << index);
                    self->squares[ index ] = WHITE_KING;
                    self->whitePieces.king = index;
                    break;

                case 'p':
                    self->blackPieces.bbPawn |= ( 1ull << index );
                    self->squares[ index ] = BLACK_PAWN;
                    break;

                case 'n':
                    self->blackPieces.bbKnight |= ( 1ull << index );
                    self->squares[ index ] = BLACK_KNIGHT;
                    break;

                case 'b':
                    self->blackPieces.bbBishop |= ( 1ull << index );
                    self->squares[ index ] = BLACK_BISHOP;
                    break;

                case 'r':
                    self->blackPieces.bbRook |= ( 1ull << index );
                    self->squares[ index ] = BLACK_ROOK;
                    break;

                case 'q':
                    self->blackPieces.bbQueen |= ( 1ull << index );
                    self->squares[ index ] = BLACK_QUEEN;
                    break;

                case 'k':
                    self->blackPieces.bbKing |= ( 1ull << index );
                    self->squares[ index ] = BLACK_KING;
                    self->blackPieces.king = index;
                    break;

                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                    // Subtract '1', not '0' here as we are already incrementing file by one
                    file += (item - '1');
                    break;
            }
        }

        rankSpecification = strtok_s( NULL, "/", &nextToken );

        rank--;
        file = 0;
    }

    self->whitePieces.bbAll = self->whitePieces.bbPawn | self->whitePieces.bbKnight | self->whitePieces.bbBishop | self->whitePieces.bbRook | self->whitePieces.bbQueen | self->whitePieces.bbKing;
    self->blackPieces.bbAll = self->blackPieces.bbPawn | self->blackPieces.bbKnight | self->blackPieces.bbBishop | self->blackPieces.bbRook | self->blackPieces.bbQueen | self->blackPieces.bbKing;

    free( section );
}

void Board_processActiveColor( Board* self, const char* fenSection ) 
{
    self->whiteToMove = ( fenSection[ 0 ] == 'w' );
}

void Board_processCastlingRights( Board* self, const char* fenSection ) 
{
    if ( fenSection[ 0 ] != '-' )
    {
        for ( unsigned short loop = 0; fenSection[ loop ] != '\0'; loop++ )
        {
            char castlingRight = fenSection[ loop ];

            switch ( castlingRight )
            {
                case 'K':
                    self->whitePieces.kingsideCastling = true;
                    break;

                case 'Q':
                    self->whitePieces.queensideCastling = true;
                    break;

                case 'k':
                    self->blackPieces.kingsideCastling = true;
                    break;

                case 'q':
                    self->blackPieces.queensideCastling = true;
                    break;
            }
        }
    }
}

void Board_processEnPassantSquare( Board* self, const char* fenSection ) 
{
    if ( fenSection[ 0 ] != '-' )
    {
        self->enPassantSquare = squareToIndex( fenSection );
    }
}

void Board_processHalfmoveClock( Board* self, const char* fenSection ) 
{
    if ( fenSection != NULL )
    {
        self->halfmoveClock = atoi( fenSection );
    }
}

void Board_processFullmoveNumber( Board* self, const char* fenSection ) 
{
    if ( fenSection != NULL )
    {
        self->fullmoveNumber = atoi( fenSection );
    }
}

void Board_printBoard( Board* self )
{
    printf( "     A   B   C   D   E   F   G   H \n" );
    printf( "   +---+---+---+---+---+---+---+---+\n" );
    for ( unsigned char rank = 8; rank > 0; rank-- )
    {
        printf( " %d |", rank );
        for ( unsigned char file = 1; file <= 8; file++ )
        {
            unsigned char item = self->squares[ ((rank-1)<<3)+(file-1) ];
            unsigned char piece = pieceNames[ item ];
            if ( ( rank + file ) & 1 )
            {
                printf( " %c |", pieceNames[ item ] );
            }
            else
            {
                if ( item == EMPTY )
                {
                    printf( ":::|" );
                }
                else
                {
                    printf( ":%c:|", pieceNames[ item ] );
                }
            }
        }
        printf( " %d\n", rank );
        printf( "   +---+---+---+---+---+---+---+---+\n" );
    }
    printf( "     A   B   C   D   E   F   G   H \n" );
}

void Board_exportBoard( Board* self, char* fen )
{
    int fenIndex = 0;
    for ( unsigned char rank = 8; rank > 0; rank-- )
    {
        if ( fenIndex > 0 )
        {
            fen[ fenIndex++ ] = '/';
        }

        int Board_isEmptySquare = 0;
        for ( unsigned char file = 1; file <= 8; file++ )
        {
            unsigned char item = self->squares[ ( ( rank - 1 ) << 3 ) + ( file - 1 ) ];

            if ( item == EMPTY )
            {
                Board_isEmptySquare++;
            }
            else
            {
                if ( Board_isEmptySquare > 0 )
                {
                    fen[ fenIndex++ ] = '0' + Board_isEmptySquare;
                    Board_isEmptySquare = 0;
                }

                fen[ fenIndex++ ] = pieceNames[ item ];
            }
        }

        if ( Board_isEmptySquare > 0 )
        {
            fen[ fenIndex++ ] = '0' + Board_isEmptySquare;
            Board_isEmptySquare = 0;
        }
    }

    // Active color
    fen[ fenIndex++ ] = ' ';
    fen[ fenIndex++ ] = self->whiteToMove ? 'w' : 'b';

    // Castling rights
    fen[ fenIndex++ ] = ' ';
    if ( self->whitePieces.kingsideCastling || self->whitePieces.queensideCastling || self->blackPieces.kingsideCastling || self->blackPieces.queensideCastling )
    {
        if ( self->whitePieces.kingsideCastling )
        {
            fen[ fenIndex++ ] = 'K';
        }
        if ( self->whitePieces.queensideCastling )
        {
            fen[ fenIndex++ ] = 'Q';
        }
        if ( self->blackPieces.kingsideCastling )
        {
            fen[ fenIndex++ ] = 'k';
        }
        if ( self->blackPieces.queensideCastling )
        {
            fen[ fenIndex++ ] = 'q';
        }
    }
    else
    {
        fen[ fenIndex++ ] = '-';
    }

    // En passant
    fen[ fenIndex++ ] = ' ';
    if ( self->enPassantSquare == OFF_BOARD )
    {
        fen[ fenIndex++ ] = '-';
    }
    else
    {
        fen[ fenIndex++ ] = 'a' + ( self->enPassantSquare & 0b00000111 );
        fen[ fenIndex++ ] = '1' + ( ( self->enPassantSquare >> 3 ) & 0b00000111 );
    }

    // Terminate the string here so we can sprintf the remainder
    fen[ fenIndex ] = '\0';

    // Halfmove clock - printed in a slightly hinky way
#define BUFFERSIZE 10
    char buffer[ BUFFERSIZE ];
    sprintf_s( buffer, BUFFERSIZE, " %d", self->halfmoveClock );
    strcat_s( fen, strlen( fen ) + BUFFERSIZE, buffer );

    // Fullmove number - printed in a slightly hinky way
    sprintf_s( buffer, BUFFERSIZE, " %d", self->fullmoveNumber );
    strcat_s( fen, strlen( fen ) + BUFFERSIZE, buffer );
}

void Board_exportMove( Move move, char* moveString )
{
    unsigned char index = 0;
    moveString[ index++ ] = (char)('a' + Move_fromFile( move ) );
    moveString[ index++ ] = (char)('1' + Move_fromRank( move ) );
    moveString[ index++ ] = (char)('a' + Move_toFile( move ) );
    moveString[ index++ ] = (char)('1' + Move_toRank( move ) );
    if ( Move_isPromotion( move ) )
    {
        moveString[ index++ ] = pieceNames[ Move_promotion( move ) ];
    }
    moveString[ index ] = '\0';
}

MoveList* Board_generateMoves( Board* self, MoveList* moveList )
{
    Board_generatePawnMoves( self, moveList );
    Board_generateKnightMoves( self, moveList );
    Board_generateBishopMoves( self, moveList );
    Board_generateRookMoves( self, moveList );
    Board_generateQueenMoves( self, moveList );
    Board_generateKingMoves( self, moveList );

    return moveList;
}

void Board_generatePawnMoves( Board* self, MoveList* moveList )
{
    static const unsigned long whitePromotionPieces[] = { WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN };
    static const unsigned long blackPromotionPieces[] = { BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN };

    const PieceList* friendlyPieces = self->whiteToMove ? &self->whitePieces : &self->blackPieces;

    const unsigned long oneStep = self->whiteToMove ? +8 : -8;
    const unsigned long twoStep = self->whiteToMove ? +16 : -16;
    const unsigned long captureL = self->whiteToMove ? +7 : -9;
    const unsigned long captureR = self->whiteToMove ? +9 : -7;

    // From where can they move two squares forward
    const unsigned long homeRank = self->whiteToMove ? 1 : 6;
    const unsigned long promotionRank = self->whiteToMove ? 7 : 0;
    const unsigned long *promotionPieces = self->whiteToMove ? whitePromotionPieces : blackPromotionPieces;

    unsigned long long pieces = friendlyPieces->bbPawn;

    unsigned long index;
    unsigned long destination;
    while ( _BitScanForward64( &index, pieces ) )
    {
        pieces ^= 1ull << index;

        // Single move forward, no capture
        // Pawns cannot be on 8th/1st rank (depending on color) due to promotion, so no need for edge detection for this
        destination = index + oneStep;
        if ( self->squares[ destination ] == EMPTY )
        {
            if ( Board_rankFromIndex( destination ) == promotionRank )
            {
                for ( unsigned short loop = 0; loop < 4; loop++ )
                {
                    MoveList_addMove( moveList, Move_createPromotionMove( index, destination, promotionPieces[ loop] ) );
                }
            }
            else
            {
                MoveList_addMove( moveList, Move_createMove( index, destination ) );

                // Those elible for a single step forward can maybe also do two steps
     
                destination = index + twoStep;
                if ( Board_rankFromIndex( index ) == homeRank && self->squares[ destination ] == EMPTY )
                {
                    MoveList_addMove( moveList, Move_createMove( index, destination ) );
                }
            }
        }

        if ( Board_fileFromIndex( index ) > 0 )
        {
            destination = index + captureL;
            if ( destination == self->enPassantSquare )
            {
                MoveList_addMove( moveList, Move_createMove( index, destination ) );
            }
            else if ( Board_containsAttacker( self, destination ) )
            {
                if ( Board_rankFromIndex( destination ) == promotionRank )
                {
                    for ( unsigned short loop = 0; loop < 4; loop++ )
                    {
                        MoveList_addMove( moveList, Move_createPromotionMove( index, destination, promotionPieces[ loop ] ) );
                    }
                }
                else
                {
                    MoveList_addMove( moveList, Move_createMove( index, destination ) );
                }
            }
        }

        if ( Board_fileFromIndex( index ) < 7 )
        {
            destination = index + captureR;
            if ( destination == self->enPassantSquare )
            {
                MoveList_addMove( moveList, Move_createMove( index, destination ) );
            }
            else if ( Board_containsAttacker( self, destination ) )
            {
                if ( Board_rankFromIndex( destination ) == promotionRank )
                {
                    for ( unsigned short loop = 0; loop < 4; loop++ )
                    {
                        MoveList_addMove( moveList, Move_createPromotionMove( index, destination, promotionPieces[ loop ] ) );
                    }
                }
                else
                {
                    MoveList_addMove( moveList, Move_createMove( index, destination ) );
                }
            }
        }
    }
}

void Board_generateKnightMoves( Board* self, MoveList* moveList )
{
    const PieceList* friendlyPieces = self->whiteToMove ? &self->whitePieces : &self->blackPieces;

    unsigned long long pieces = friendlyPieces->bbKnight;

    unsigned long index;
    unsigned long destination;
    while ( _BitScanForward64( &index, pieces ) )
    {
        pieces ^= 1ull << index;

        unsigned long* directions = knightDirections[ index ];
        for( short loop = 0; loop < 8; loop++ )
        {
            if ( directions[ loop ] != 0 )
            {
                destination = index + directions[ loop ];
                if ( !Board_containsFriendly( self, destination ) )
                {
                    MoveList_addMove( moveList, Move_createMove( index, destination ) );
                }
            }
        }
    }
}

void Board_generateBishopMoves( Board* self, MoveList* moveList ) 
{
    const PieceList* friendlyPieces = self->whiteToMove ? &self->whitePieces : &self->blackPieces;
    const PieceList* attackerPieces = self->whiteToMove ? &self->blackPieces : &self->whitePieces;

    unsigned long long pieces = friendlyPieces->bbBishop;

    unsigned long index;
    unsigned long destination;
    while ( _BitScanForward64( &index, pieces ) )
    {
        pieces ^= 1ull << index;

        long pieceRank = Board_rankFromIndex( index );
        long pieceFile = Board_fileFromIndex( index );

        // Normal one-square moves diagonal
        short vectors[][2] =
        { 
            {-1,-1},
            {-1,+1},
            {+1,-1},
            {+1,+1},
        };

        for ( unsigned short vectorIndex = 0; vectorIndex < 4; vectorIndex++ )
        {
            for ( unsigned short distance = 1; distance < 8; distance++ )
            {
                long destinationRank = pieceRank + ( distance * vectors[ vectorIndex ][ 0 ] );
                if ( destinationRank < 0 || destinationRank > 7 )
                {
                    // Stop travelling in this direction
                    break;
                }
                long destinationFile = pieceFile + ( distance * vectors[ vectorIndex ][ 1 ] );
                if ( destinationFile < 0 || destinationFile > 7 )
                {
                    // Stop travelling in this direction
                    break;
                }

                destination = ( destinationRank << 3 ) + destinationFile;

                if ( Board_containsFriendly( self, destination ) )
                {
                    // Stop travelling in this direction - friendly piece found
                    break;
                }
                else if ( Board_containsAttacker( self, destination ) )
                {
                    MoveList_addMove( moveList, Move_createMove( index, destination ) );

                    // Stop travelling in this direction - capture made
                    break;
                }
                else // Empty square - note the move and keep looking
                {
                    MoveList_addMove( moveList, Move_createMove( index, destination ) );
                }
            }
        }
    }
}

void Board_generateRookMoves( Board* self, MoveList* moveList ) 
{
    const PieceList* friendlyPieces = self->whiteToMove ? &self->whitePieces : &self->blackPieces;
    const PieceList* attackerPieces = self->whiteToMove ? &self->blackPieces : &self->whitePieces;

    unsigned long long pieces = friendlyPieces->bbRook;

    unsigned long index;
    unsigned long destination;
    while ( _BitScanForward64( &index, pieces ) )
    {
        pieces ^= 1ull << index;

        long pieceRank = Board_rankFromIndex( index );
        long pieceFile = Board_fileFromIndex( index );

        // Normal one-square moves horizontal and vertical
        short vectors[][ 2 ] =
        {
            {-1, 0},
            { 0,-1},
            { 0,+1},
            {+1, 0},
        };

        for ( unsigned short vectorIndex = 0; vectorIndex < 4; vectorIndex++ )
        {
            for ( unsigned short distance = 1; distance < 8; distance++ )
            {
                long destinationRank = pieceRank + ( distance * vectors[ vectorIndex ][ 0 ] );
                if ( destinationRank < 0 || destinationRank > 7 )
                {
                    // Stop travelling in this direction
                    break;
                }
                long destinationFile = pieceFile + ( distance * vectors[ vectorIndex ][ 1 ] );
                if ( destinationFile < 0 || destinationFile > 7 )
                {
                    // Stop travelling in this direction
                    break;
                }

                destination = ( destinationRank << 3 ) + destinationFile;

                if ( Board_containsFriendly( self, destination ) )
                {
                    // Stop travelling in this direction - friendly piece found
                    break;
                }
                else if ( Board_containsAttacker( self, destination ) )
                {
                    MoveList_addMove( moveList, Move_createMove( index, destination ) );

                    // Stop travelling in this direction - capture made
                    break;
                }
                else // Empty square - note the move and keep looking
                {
                    MoveList_addMove( moveList, Move_createMove( index, destination ) );
                }
            }
        }
    }
}

void Board_generateQueenMoves( Board* self, MoveList* moveList ) 
{
    const PieceList* friendlyPieces = self->whiteToMove ? &self->whitePieces : &self->blackPieces;
    const PieceList* attackerPieces = self->whiteToMove ? &self->blackPieces : &self->whitePieces;

    unsigned long long pieces = friendlyPieces->bbQueen;

    unsigned long index;
    unsigned long destination;
    while ( _BitScanForward64( &index, pieces ) )
    {
        pieces ^= 1ull << index;

        long pieceRank = Board_rankFromIndex( index );
        long pieceFile = Board_fileFromIndex( index );

        // Normal one-square moves in any direction
        short vectors[][ 2 ] =
        {
            {-1,-1},
            {-1,+1},
            {+1,-1},
            {+1,+1},
            {-1, 0},
            { 0,-1},
            { 0,+1},
            {+1, 0},
        };

        for ( unsigned short vectorIndex = 0; vectorIndex < 8; vectorIndex++ )
        {
            for ( unsigned short distance = 1; distance < 8; distance++ )
            {
                long destinationRank = pieceRank + ( distance * vectors[ vectorIndex ][ 0 ] );
                if ( destinationRank < 0 || destinationRank > 7 )
                {
                    // Stop travelling in this direction
                    break;
                }
                long destinationFile = pieceFile + ( distance * vectors[ vectorIndex ][ 1 ] );
                if ( destinationFile < 0 || destinationFile > 7 )
                {
                    // Stop travelling in this direction
                    break;
                }

                destination = ( destinationRank << 3 ) + destinationFile;

                if ( Board_containsFriendly( self, destination ) )
                {
                    // Stop travelling in this direction - friendly piece found
                    break;
                }
                else if ( Board_containsAttacker( self, destination ) )
                {
                    MoveList_addMove( moveList, Move_createMove( index, destination ) );

                    // Stop travelling in this direction - capture made
                    break;
                }
                else // Empty square - note the move and keep looking
                {
                    MoveList_addMove( moveList, Move_createMove( index, destination ) );
                }
            }
        }
    }
}

void Board_generateKingMoves( Board* self, MoveList* moveList )
{
    const PieceList* friendlyPieces = self->whiteToMove ? &self->whitePieces : &self->blackPieces;
    const PieceList* attackerPieces = self->whiteToMove ? &self->blackPieces : &self->whitePieces;

    unsigned long index = friendlyPieces->king;
    unsigned long destination;

    long kingRank = Board_rankFromIndex( index );
    long kingFile = Board_fileFromIndex( index );

    // Normal one-square moves
    for ( short rankOffset = -1; rankOffset <= +1; rankOffset++ )
    {
        if ( kingRank + rankOffset < 0 || kingRank + rankOffset > 7 )
        {
            continue;
        }

        for ( short fileOffset = -1; fileOffset <= +1; fileOffset++ )
        {
            if ( kingFile + fileOffset < 0 || kingFile + fileOffset > 7 )
            {
                continue;
            }

            destination = index + ( rankOffset * 8 ) + fileOffset;

            if ( !Board_containsFriendly( self, destination ) )
            {
                MoveList_addMove( moveList, Move_createMove( index, destination ) );
            }

        }
    }

    // Castling? 
    // Assume the flags are accurate but still need to make sure there is nothing in the way and we're not moving through check
    unsigned long long allPieces = friendlyPieces->bbAll & attackerPieces->bbAll;

    if ( friendlyPieces->kingsideCastling )
    {
        // It is simply easier to do this with bespoke code for both colors
        if ( self->whiteToMove )
        {
            if ( Board_isEmptySquare( self, F1 ) && Board_isEmptySquare( self, G1 ) )
            {
                MoveList_addMove( moveList, Move_createMove( index, G1 ) );
            }
        }
        else
        {
            if ( Board_isEmptySquare( self, F8 ) && Board_isEmptySquare( self, G8 ) )
            {
                MoveList_addMove( moveList, Move_createMove( index, G8 ) );
            }
        }
    }
    if ( friendlyPieces->queensideCastling )
    {
        // It is simply easier to do this with bespoke code for both colors
        if ( self->whiteToMove )
        {
            if ( Board_isEmptySquare( self, B1 ) && Board_isEmptySquare( self, C1 ) && Board_isEmptySquare( self, D1 ) )
            {
                MoveList_addMove( moveList, Move_createMove( index, C1 ) );
            }
        }
        else
        {
            if ( Board_isEmptySquare( self, B8 ) && Board_isEmptySquare( self, C8 ) && Board_isEmptySquare( self, D8 ) )
            {
                MoveList_addMove( moveList, Move_createMove( index, C8 ) );
            }
        }
    }
}

unsigned long Board_rankFromIndex( unsigned long index )
{
    return (index >> 3) & 0b00000111;
}

unsigned long Board_fileFromIndex( unsigned long index )
{
    return index & 0b00000111;
}

bool Board_isEmptySquare( Board* self, unsigned long index )
{
    return self->squares[ index ] == EMPTY;
}

bool Board_containsFriendly( Board* self, unsigned long index )
{
    // Empty can look like a white piece if we don't explicitly check
    if ( Board_isEmptySquare( self, index ) )
    {
        return false;
    }

    if ( self->whiteToMove )
    {
        return ( self->squares[ index ] & COLOR_BIT ) == 0;
    }
    else
    {
        return ( self->squares[ index ] & COLOR_BIT ) == COLOR_BIT;
    }
}

bool Board_containsAttacker( Board* self, unsigned long index )
{
    // Empty can look like a white piece if we don't explicitly check
    if ( Board_isEmptySquare( self, index ) )
    {
        return false;
    }

    if ( self->whiteToMove )
    {
        return ( self->squares[ index ] & COLOR_BIT ) == COLOR_BIT;
    }
    else
    {
        return ( self->squares[ index ] & COLOR_BIT ) == 0;
    }
}

bool Board_isPawn( enum Piece piece )
{
    return ( piece & COLOR_MASK ) == PAWN;
}

bool Board_isKnight( enum Piece piece )
{
    return ( piece & COLOR_MASK ) == KNIGHT;
}

bool Board_isBishop( enum Piece piece )
{
    return ( piece & COLOR_MASK ) == BISHOP;
}

bool Board_isRook( enum Piece piece )
{
    return (piece & COLOR_MASK) == ROOK;
}

bool Board_isQueen( enum Piece piece )
{
    return ( piece & COLOR_MASK ) == QUEEN;
}

bool Board_isKing( enum Piece piece )
{
    return ( piece & COLOR_MASK ) == KING;
}

bool Board_isWhitePiece( enum Piece piece )
{
    return piece >= WHITE_PAWN && piece <= WHITE_KING;
}

bool Board_isBlackPiece( enum Piece piece )
{
    return piece >= BLACK_PAWN && piece <= BLACK_KING;
}

void Board_clearSquare( Board* self, PieceList* pieceList, unsigned long index )
{
    if ( Board_isEmptySquare( self, index ) )
    {
        return;
    }

    if ( pieceList == NULL )
    {
        // We already know the square isn't empty, so one of these is right
        if ( Board_isWhitePiece( self->squares[ index ] ) )
        {
            Board_clearSquare( self, &self->whitePieces, index );
        }
        else
        {
            Board_clearSquare( self, &self->blackPieces, index );
        }
    }
    else
    {
        const unsigned long long mask = 1ull << index;
        const unsigned long long notmask = ~mask;

        self->squares[ index ] = EMPTY;

        // Special case
        if ( pieceList->bbKing & mask )
        {
            pieceList->bbKing &= notmask;
            pieceList->king = OFF_BOARD;
        }
        else
        {

            pieceList->bbPawn &= notmask;
            pieceList->bbKnight &= notmask;
            pieceList->bbBishop &= notmask;
            pieceList->bbRook &= notmask;
            pieceList->bbQueen &= notmask;
        }

        pieceList->bbAll &= notmask;
    }
}

void Board_setSquare( Board* self, PieceList* pieceList, enum Piece piece, unsigned long index )
{
    // Precaution, hopefully not too costly
    if ( !Board_isEmptySquare( self, index ) )
    {
        Board_clearSquare( self, NULL, index );
    }

    if ( pieceList == NULL )
    {
        if ( Board_isWhitePiece( piece ) )
        {
            Board_setSquare( self, &self->whitePieces, piece, index );
        }
        else
        {
            Board_setSquare( self, &self->blackPieces, piece, index );
        }
    }
    else
    {
        const unsigned long long mask = 1ull << index;

        self->squares[ index ] = piece;

        switch ( piece & COLOR_MASK )
        {
            case PAWN:
                pieceList->bbPawn |= mask;
                break;
            case KNIGHT:
                pieceList->bbKnight |= mask;
                break;
            case BISHOP:
                pieceList->bbBishop |= mask;
                break;
            case ROOK:
                pieceList->bbRook |= mask;
                break;
            case QUEEN:
                pieceList->bbQueen |= mask;
                break;
            case KING:
                pieceList->bbKing |= mask;
                pieceList->king = index;
                break;
        }

        pieceList->bbAll |= mask;
    }
}

bool Board_makeMove( Board* self, Move move )
{
    // Return false if it becomes apparent that the move is not legal

    PieceList* friendlyPieces = self->whiteToMove ? &self->whitePieces : &self->blackPieces;
    PieceList* attackerPieces = self->whiteToMove ? &self->blackPieces : &self->whitePieces;

    unsigned long from = Move_from( move );
    unsigned long to = Move_to( move );

    unsigned char fromPiece = self->squares[ from ];
    unsigned char toPiece = self->squares[ to ];

    // Steps:
    // - remove any piece being captured from bb and from squares
    //   - include pawn captured by enPassant (which won't be on "to" square)
    if ( !Board_isEmptySquare( self, to ) )
    {
        // Capture - remove enemy piece from the board
        // We presumably don't need to care about the king in the attackerPieces structure as we will not actually be capturing it
        Board_clearSquare( self, attackerPieces, to );
    }
    else if ( to == self->enPassantSquare && Board_isPawn( fromPiece ) )
    {
        // enPassant capture - remove enemy piece (pawn) from the board
        const enPassantAttackerIndex = self->whiteToMove ? self->enPassantSquare - 8 : self->enPassantSquare + 8;

        Board_clearSquare( self, attackerPieces, enPassantAttackerIndex );
    }

    // - lift piece from "from" and put it on "to" in both bb and squares (and king if appropriate)
    //   - replace "to" piece in the case of promotion
    Board_clearSquare( self, friendlyPieces, from );

    if ( Move_isPromotion( move ) )
    {
        Board_setSquare( self, friendlyPieces, Move_promotion( move ), to);
    }
    else
    {
        Board_setSquare( self, friendlyPieces, fromPiece, to );
    }

    // - clear enPassant and set to new value if required
    if ( Board_isPawn( fromPiece ) && abs( from - to ) == 16 )
    {
        // Double pawn move - set en passant sqaure
        self->enPassantSquare = ( from + to ) / 2;
    }
    else
    {
        self->enPassantSquare = OFF_BOARD;
    }

    // - move rook if castling in both bb and squares
    if ( Board_isKing( fromPiece ) )
    {
        // Castling? Move the associated rook
        if ( abs( from - to ) == 2 )
        {
            // Kingside or Queenside
            if ( from < to )
            {
                enum Piece rook = self->squares[ from + 3 ];
                Board_clearSquare( self, friendlyPieces, from + 3 );
                Board_setSquare( self, friendlyPieces, rook, from + 1 );
            }
            else
            {
                enum Piece rook = self->squares[ from - 4 ];
                Board_clearSquare( self, friendlyPieces, from - 4 );
                Board_setSquare( self, friendlyPieces, rook, from - 1 );
            }
        }
    }
    
    // - update castling rights if affected by this move
    if ( Board_isKing( fromPiece ) )
    {
        friendlyPieces->kingsideCastling = false;
        friendlyPieces->queensideCastling = false;
    }

    if ( Board_isRook( fromPiece ) )
    {
        if ( Board_fileFromIndex( from ) == 0 )
        {
            friendlyPieces->queensideCastling = false;

        }
        else if ( Board_fileFromIndex( from ) == 7 )
        {
            friendlyPieces->kingsideCastling = false;
        }
    }

    if ( Board_isRook( toPiece ) )
    {
        if ( Board_fileFromIndex( to ) == 0 )
        {
            attackerPieces->queensideCastling = false;

        }
        else if ( Board_fileFromIndex( to ) == 7 )
        {
            attackerPieces->kingsideCastling = false;
        }
    }

    // - swap active color
    self->whiteToMove = !self->whiteToMove;

    // - increment or reset halfmove clock
    if ( Board_isPawn( fromPiece ) || toPiece != EMPTY )
    {
        // Pawn move or capture - reset halfmove clock
        self->halfmoveClock = 0;
    }
    else
    {
        self->halfmoveClock++;
    }

    // - increment fullmove number if next move will be for white
    if ( self->whiteToMove )
    {
        self->fullmoveNumber++;
    }

    return true;
}

void Board_copy( Board* self, Board* copy ) 
{
    memcpy( copy, self, sizeof( Board ) );
}

void Board_apply( Board* self, Board* other ) 
{
    memcpy( self, other, sizeof( Board ) );
}

bool Board_compare( Board* self, Board* other ) 
{
    return memcmp( self, other, sizeof( Board ) ) == 0;
}

