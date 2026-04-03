#ifndef PAWN_TABLE_H
#define PAWN_TABLE_H

#include "../types.h"
#include "../board/board.h"
#include "piece_value.h"

#define PAWN_TABLE_SIZE 16384

typedef struct {
    uint64_t key;
    int16_t score_mg;
    int16_t score_eg;
    Bitboard passed_pawns[COLOR_NB];
    Bitboard pawn_chains[COLOR_NB];
    uint8_t king_shelter[COLOR_NB];
    uint8_t passed_pawn_count[COLOR_NB];
} PawnEntry;

void pawn_table_init(void);
EvalScore eval_pawns(const Board* board);

#endif
