#include "stdafx.h"

#include "Board.h"
#include "Utility.h"

#define OFF_BOARD -1

struct Board* Board_create( const char* fen )
{
    struct Board* board = malloc( sizeof( struct Board ) );

    if ( board != NULL )
    {
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
    return board;
}

void Board_shutdown( struct Board* self )
{
    if ( self != NULL )
    {
        free( self );
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
    static const char pieceNames[] = " PNBRQKpnbrqk";
    printf( "     A   B   C   D   E   F   G   H \n" );
    printf( "   +---+---+---+---+---+---+---+---+\n" );
    for ( char rank = 8; rank > 0; rank-- )
    {
        printf( " %d |", rank );
        for ( char file = 1; file <= 8; file++ )
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
