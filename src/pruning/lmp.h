#ifndef LMP_H
#define LMP_H

#include "types.h"

#define LMP_BASE_MOVES 2
#define LMP_MAX_DEPTH 64

typedef struct {
    int baseLmpMoves;
} LmpConfig;

typedef struct {
    int lmpTable[LMP_MAX_DEPTH];
} LmpTable;

void lmp_init(LmpConfig* cfg, LmpTable* lmp);
int lmp_max_moves(LmpTable* lmp, int depth);
bool lmp_should_prune(LmpTable* lmp, int depth, int moveCount);

#endif
