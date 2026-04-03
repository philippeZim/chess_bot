#ifndef TRANSPOSITION_H
#define TRANSPOSITION_H

#include "../types.h"
#include "../board/board.h"

typedef struct {
    uint64_t key;
    uint16_t depth;
    int16_t score;
    uint16_t flag;
    Move best_move;
} TranspositionEntry;

#define TT_FLAG_EXACT 1
#define TT_FLAG_ALPHA 2
#define TT_FLAG_BETA 3

#define TT_SIZE (1024 * 1024)

typedef struct {
    TranspositionEntry* table;
    int size;
} TranspositionTable;

bool tt_alloc(TranspositionTable* tt);
void tt_free(TranspositionTable* tt);
void tt_init(TranspositionTable* tt);
void tt_clear(TranspositionTable* tt);
TranspositionEntry* tt_probe(TranspositionTable* tt, uint64_t key);
void tt_store(TranspositionTable* tt, uint64_t key, uint16_t depth, int16_t score, uint16_t flag, Move best_move);
Move tt_get_move(TranspositionTable* tt, uint64_t key);
uint64_t tt_get_usage(TranspositionTable* tt);

#endif
