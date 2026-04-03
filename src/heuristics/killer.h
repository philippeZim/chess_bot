#ifndef KILLER_H
#define KILLER_H

#include "types.h"

#define KILLER_MAX_MOVES 2

typedef struct {
    Move killerMoves[MAX_DEPTH][KILLER_MAX_MOVES];
    int killerCounts[MAX_DEPTH];
} KillerTable;

void killer_init(KillerTable* kt);
void killer_add(KillerTable* kt, int depth, Move move);
void killer_get(KillerTable* kt, int depth, Move* moves);
void killer_clear(KillerTable* kt);
void killer_clear_depth(KillerTable* kt, int depth);
bool killer_is_killer(KillerTable* kt, int depth, Move move);
void killer_order_moves(KillerTable* kt, int depth, Move* moves, int count);

#endif
