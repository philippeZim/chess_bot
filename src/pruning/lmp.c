#include "lmp.h"

void lmp_init(LmpConfig* cfg, LmpTable* lmp) {
    cfg->baseLmpMoves = LMP_BASE_MOVES;
    
    for (int depth = 0; depth < LMP_MAX_DEPTH; depth++) {
        lmp->lmpTable[depth] = cfg->baseLmpMoves + depth * depth;
    }
}

int lmp_max_moves(LmpTable* lmp, int depth) {
    if (depth < 0 || depth >= LMP_MAX_DEPTH) return 0;
    return lmp->lmpTable[depth];
}

bool lmp_should_prune(LmpTable* lmp, int depth, int moveCount) {
    if (depth < 0 || depth >= LMP_MAX_DEPTH) return false;
    return moveCount >= lmp->lmpTable[depth];
}
