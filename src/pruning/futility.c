#include "futility.h"

void futility_init(FutilityMargins* fm) {
    fm->baseMargin = FUTILITY_BASE_MARGIN;
    fm->depthMargin = FUTILITY_DEPTH_MARGIN;
}

bool futility_can_prune(FutilityMargins* fm, int depth, int beta, int eval) {
    if (depth <= 0) return false;
    if (eval >= beta) return false;
    
    int margin = futility_margin(fm, depth);
    return eval + margin < beta;
}

int futility_margin(FutilityMargins* fm, int depth) {
    return fm->baseMargin + fm->depthMargin * depth;
}

int futility_prune_score(int eval, int margin) {
    return eval + margin;
}
