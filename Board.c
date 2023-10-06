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

// This is expected to be 0b00001000 but we should check
static const char COLOR_BIT = BLACK_PAWN - WHITE_PAWN;

static unsigned long knightDirections[ 64 ][ 8 ];

struct Board* Board_create( const char* fen )
{
    struct Board* board = malloc( sizeof( struct Board ) );

    if ( board != NULL )
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
    }

    Board_printBoard( board );
    char x[ 256 ];
    Board_exportBoard( board, x );
    printf( "%s\n", x );

    return board;
}

void Board_destroy( struct Board* self )
{
    if ( self != NULL )
    {
        free( self );
    }
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
}

void Board_clearBoard( struct Board* self )
{
    self->whitePieces.bbPawn = 0;
    self->whitePieces.bbKnight = 0;
    self->whitePieces.bbBishop = 0;
    self->whitePieces.bbRook = 0;
    self->whitePieces.bbQueen = 0;
    self->whitePieces.bbKing = 0;
    self->whitePieces.king = OFF_BOARD;

    self->blackPieces.bbPawn = 0;
    self->blackPieces.bbKnight = 0;
    self->blackPieces.bbBishop = 0;
    self->blackPieces.bbRook = 0;
    self->blackPieces.bbQueen = 0;
    self->blackPieces.bbKing = 0;
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

void Board_processBoardLayout( struct Board* self, const char* fenSection ) 
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

    free( section );
}

void Board_processActiveColor( struct Board* self, const char* fenSection ) 
{
    self->whiteToMove = ( fenSection[ 0 ] == 'w' );
}

void Board_processCastlingRights( struct Board* self, const char* fenSection ) 
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

void Board_processEnPassantSquare( struct Board* self, const char* fenSection ) 
{
    if ( fenSection[ 0 ] != '-' )
    {
        self->enPassantSquare = squareToIndex( fenSection );
    }
}

void Board_processHalfmoveClock( struct Board* self, const char* fenSection ) 
{
    if ( fenSection != NULL )
    {
        self->halfmoveClock = atoi( fenSection );
    }
}

void Board_processFullmoveNumber( struct Board* self, const char* fenSection ) 
{
    if ( fenSection != NULL )
    {
        self->fullmoveNumber = atoi( fenSection );
    }
}

void Board_printBoard( struct Board* self )
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

void Board_exportBoard( struct Board* self, char* fen )
{
    int fenIndex = 0;
    for ( unsigned char rank = 8; rank > 0; rank-- )
    {
        if ( fenIndex > 0 )
        {
            fen[ fenIndex++ ] = '/';
        }

        int empty = 0;
        for ( unsigned char file = 1; file <= 8; file++ )
        {
            unsigned char item = self->squares[ ( ( rank - 1 ) << 3 ) + ( file - 1 ) ];

            if ( item == EMPTY )
            {
                empty++;
            }
            else
            {
                if ( empty > 0 )
                {
                    fen[ fenIndex++ ] = '0' + empty;
                    empty = 0;
                }

                fen[ fenIndex++ ] = pieceNames[ item ];
            }
        }

        if ( empty > 0 )
        {
            fen[ fenIndex++ ] = '0' + empty;
            empty = 0;
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

void Board_exportMove( struct Move* move, char* moveString )
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

struct MoveList* Board_generateMoves( struct Board* self )
{
    struct MoveList* moveList = MoveList_createMoveList();

    Board_generatePawnMoves( self, moveList );
    Board_generateKnightMoves( self, moveList );

    return moveList;
}

void Board_generatePawnMoves( struct Board* self, struct MoveList* moveList )
{
    static const unsigned long whitePromotionPieces[] = { WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN };
    static const unsigned long blackPromotionPieces[] = { BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN };

    const struct PieceList* playerPieces = self->whiteToMove ? &self->whitePieces : &self->blackPieces;

    const unsigned long oneStep = self->whiteToMove ? +8 : -8;
    const unsigned long twoStep = self->whiteToMove ? +16 : -16;
    const unsigned long captureL = self->whiteToMove ? +7 : -9;
    const unsigned long captureR = self->whiteToMove ? +9 : -7;

    // From where can they move two squares forward
    const unsigned long homeRank = self->whiteToMove ? 1 : 6;
    const unsigned long promotionRank = self->whiteToMove ? 7 : 0;
    const unsigned long *promotionPieces = self->whiteToMove ? whitePromotionPieces : blackPromotionPieces;

    unsigned long long pieces = playerPieces->bbPawn;

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
            if ( rankFromIndex( destination ) == promotionRank )
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
                if ( rankFromIndex( index ) == homeRank && self->squares[ destination ] == EMPTY )
                {
                    MoveList_addMove( moveList, Move_createMove( index, destination ) );
                }
            }
        }

        if ( fileFromIndex( index ) > 0 )
        {
            destination = index + captureL;
            if ( destination == self->enPassantSquare )
            {
                MoveList_addMove( moveList, Move_createMove( index, destination ) );
            }
            else if ( attacker( self, destination ) )
            {
                if ( rankFromIndex( destination ) == promotionRank )
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

        if ( fileFromIndex( index ) < 7 )
        {
            destination = index + captureR;
            if ( destination == self->enPassantSquare )
            {
                MoveList_addMove( moveList, Move_createMove( index, destination ) );
            }
            else if ( attacker( self, destination ) )
            {
                if ( rankFromIndex( destination ) == promotionRank )
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

void Board_generateKnightMoves( struct Board* self, struct MoveList* moveList )
{
    const struct PieceList* playerPieces = self->whiteToMove ? &self->whitePieces : &self->blackPieces;

    unsigned long long pieces = playerPieces->bbKnight;

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
                if ( !friendly( self, destination ) )
                {
                    MoveList_addMove( moveList, Move_createMove( index, destination ) );
                }
            }
        }
    }
}

unsigned long rankFromIndex( unsigned long index )
{
    return (index >> 3) & 0b00000111;
}

unsigned long fileFromIndex( unsigned long index )
{
    return index & 0b00000111;
}

bool empty( struct Board* self, unsigned long index )
{
    return self->squares[ index ] == EMPTY;
}

bool friendly( struct Board* self, unsigned long index )
{
    // Empty can look like a white piece if we don't explicitly check
    if ( empty( self, index ) )
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

bool attacker( struct Board* self, unsigned long index )
{
    // Empty can look like a white piece if we don't explicitly check
    if ( empty( self, index ) )
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

bool Board_makeMove( struct Board* self, struct Move* move )
{
    // Return false if it becomes apparent that the move is not legal

    // Steps
    // - remove any piece being captured from bb and from squares
    //   - include pawn captured by enPassant (which won't be on "to" square)
    // - lift piece from "from" and put it on "to" in both bb and squares
    //   - replace "to" piece in the case of promotion
    // - clear enPassant and set to new value if required
    // - move rook if castling in both bb and squares
    // - update castling rights if affected by this move
    // - increment or reset halfmove clock
    // - increment fullmove number if next move will be for white
    // - swap active color

    return true;
}

struct Board* Board_copy( struct Board* self ) 
{
    struct Board* other = malloc( sizeof( struct Board ) );

    if ( other != NULL )
    {
        memcpy( self, other, sizeof( struct Board ) );
    }

    return other;
}

void Board_apply( struct Board* self, struct Board* other ) 
{
    memcpy( self, other, sizeof( struct Board ) );
}

bool Board_compare( struct Board* self, struct Board* other ) 
{
    //struct Board
    //{
    //    unsigned char squares[ 64 ];
    //    struct PieceList whitePieces;
    //    struct PieceList blackPieces;
    //    bool whiteToMove;
    //    unsigned char enPassantSquare;
    //    unsigned short halfmoveClock;
    //    unsigned short fullmoveNumber;
    //};
    for ( short loop = 0; loop < 64; loop++ )
    {
        if ( self->squares[ loop ] != other->squares[ loop ] )
        {
            return false;
        }
    }
    if ( self->whitePieces.bbPawn != other->whitePieces.bbPawn )
    {
        return false;
    }
    if ( self->whitePieces.bbKnight != other->whitePieces.bbKnight )
    {
        return false;
    }
    if ( self->whitePieces.bbBishop != other->whitePieces.bbBishop )
    {
        return false;
    }
    if ( self->whitePieces.bbRook != other->whitePieces.bbRook )
    {
        return false;
    }
    if ( self->whitePieces.bbQueen != other->whitePieces.bbQueen )
    {
        return false;
    }
    if ( self->whitePieces.bbKing != other->whitePieces.bbKing )
    {
        return false;
    }
    if ( self->whitePieces.king != other->whitePieces.king )
    {
        return false;
    }
    if ( self->whitePieces.kingsideCastling != other->whitePieces.kingsideCastling )
    {
        return false;
    }
    if ( self->whitePieces.queensideCastling != other->whitePieces.queensideCastling )
    {
        return false;
    }
    if ( self->blackPieces.bbPawn != other->blackPieces.bbPawn )
    {
        return false;
    }
    if ( self->blackPieces.bbKnight != other->blackPieces.bbKnight )
    {
        return false;
    }
    if ( self->blackPieces.bbBishop != other->blackPieces.bbBishop )
    {
        return false;
    }
    if ( self->blackPieces.bbRook != other->blackPieces.bbRook )
    {
        return false;
    }
    if ( self->blackPieces.bbQueen != other->blackPieces.bbQueen )
    {
        return false;
    }
    if ( self->blackPieces.bbKing != other->blackPieces.bbKing )
    {
        return false;
    }
    if ( self->blackPieces.king != other->blackPieces.king )
    {
        return false;
    }
    if ( self->blackPieces.kingsideCastling != other->blackPieces.kingsideCastling )
    {
        return false;
    }
    if ( self->blackPieces.queensideCastling != other->blackPieces.queensideCastling )
    {
        return false;
    }
    if ( self->whiteToMove != other->whiteToMove )
    {
        return false;
    }
    if ( self->enPassantSquare != other->enPassantSquare )
    {
        return false;
    }
    if ( self->halfmoveClock != other->halfmoveClock )
    {
        return false;
    }
    if ( self->fullmoveNumber != other->fullmoveNumber )
    {
        return false;
    }
    return true;
    //return memcmp( self, other, sizeof( struct Board ) ) == 0;
}

