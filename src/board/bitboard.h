#ifndef BITBOARD_H
#define BITBOARD_H

#include "../types.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

extern Bitboard BB_A_FILE;
extern Bitboard BB_B_FILE;
extern Bitboard BB_C_FILE;
extern Bitboard BB_D_FILE;
extern Bitboard BB_E_FILE;
extern Bitboard BB_F_FILE;
extern Bitboard BB_G_FILE;
extern Bitboard BB_H_FILE;

extern Bitboard BB_1ST_RANK;
extern Bitboard BB_2ND_RANK;
extern Bitboard BB_3RD_RANK;
extern Bitboard BB_4TH_RANK;
extern Bitboard BB_5TH_RANK;
extern Bitboard BB_6TH_RANK;
extern Bitboard BB_7TH_RANK;
extern Bitboard BB_8TH_RANK;

extern Bitboard BB_CENTER;

#define BB_FILE(file) (0x0101010101010101ULL << (file))
#define BB_RANK(rank) (0xFFULL << ((rank) * 8))

Bitboard bitboard_from_square(Square square);
Square bitboard_next_square(Bitboard* bb);
int bitboard_popcount(Bitboard bb);
Bitboard bitboard_pop_lsb(Bitboard bb);
Bitboard bitboard_lsb(Bitboard bb);

Bitboard square_attacks_knight(Square square);
Bitboard square_attacks_king(Square square);
Bitboard pawn_attacks(Color c, Square square);
Bitboard bishop_attacks(Square square, Bitboard block);
Bitboard rook_attacks(Square square, Bitboard block);
Bitboard queen_attacks(Square square, Bitboard block);

const char* square_to_string(Square square);
Square string_to_square(const char* str);
void bitboard_to_string(Bitboard bb, char* buf, size_t bufsize);

#endif
