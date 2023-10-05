#pragma once

/// <summary>
/// moveBits is special|promotion|from|to
/// 0b11111111111111110000000000000000 - special bits
/// 0b00000000000000001111000000000000 - promotion piece
/// 0b00000000000000000000111111000000 - from bits
/// 0b00000000000000000000111000000000 - from rank
/// 0b00000000000000000000000111000000 - from file
/// 0b00000000000000000000000000111111 - to bits
/// 0b00000000000000000000000000111000 - to rank
/// 0b00000000000000000000000000000111 - to file
/// </summary>
struct Move
{
    unsigned int moveBits;
};

struct MoveList
{
    struct Move* moves[ 256 ];
    unsigned char count;
};

// Move methods

struct Move* Move_createMove( unsigned long from, unsigned long to );
struct Move* Move_createPromotionMove( unsigned long from, unsigned long to, unsigned long promotion );
void Move_destroy( struct Move* self );

unsigned long Move_from( struct Move* self );
unsigned long Move_fromRank( struct Move* self );
unsigned long Move_fromFile( struct Move* self );
unsigned long Move_to( struct Move* self );
unsigned long Move_toRank( struct Move* self );
unsigned long Move_toFile( struct Move* self );
unsigned long Move_promotion( struct Move* self );

bool Move_isPromotion( struct Move* self );

// MoveList methods

struct MoveList* MoveList_createMoveList();
void MoveList_destroy( struct MoveList* self );

void MoveList_addMove( struct MoveList* self, struct Move* move );
