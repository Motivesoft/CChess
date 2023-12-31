#pragma once

/// <summary>
/// Move is special|promotion|from|to
/// 0b11111111111111110000000000000000 - special bits
/// 0b00000000000000001111000000000000 - promotion piece
/// 0b00000000000000000000111111000000 - from bits
/// 0b00000000000000000000111000000000 - from rank
/// 0b00000000000000000000000111000000 - from file
/// 0b00000000000000000000000000111111 - to bits
/// 0b00000000000000000000000000111000 - to rank
/// 0b00000000000000000000000000000111 - to file
/// </summary>

typedef unsigned int Move;
typedef struct
{
    Move moves[ 256 ]; 
    unsigned char count;
} MoveList;

// Move methods

Move Move_createMove( unsigned long from, unsigned long to );
Move Move_createPromotionMove( unsigned long from, unsigned long to, unsigned long promotion );

unsigned long Move_from( Move self );
unsigned long Move_fromRank( Move self );
unsigned long Move_fromFile( Move self );
unsigned long Move_to( Move self );
unsigned long Move_toRank( Move self );
unsigned long Move_toFile( Move self );
unsigned long Move_promotion( Move self );

bool Move_isPromotion( Move self );

// MoveList methods

void MoveList_addMove( MoveList* self, Move move );
