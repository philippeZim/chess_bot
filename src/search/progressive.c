#include "progressive.h"

void progressive_init(ProgressiveSearch* ps, int maxDepth) {
    ps->bestMove = MOVE_NONE;
    ps->bestScore = 0;
    ps->currentDepth = 0;
    ps->maxDepth = maxDepth;
    ps->stopReason = STOP_NONE;
}

int progressive_search(ProgressiveSearch* ps, Board* board, int maxDepth) {
    ps->maxDepth = maxDepth;
    ps->currentDepth = 0;
    ps->stopReason = STOP_NONE;
    
    while (ps->currentDepth < ps->maxDepth && ps->stopReason == STOP_NONE) {
        ps->currentDepth++;
    }
    
    return ps->bestScore;
}

int progressive_next_depth(ProgressiveSearch* ps) {
    if (ps->currentDepth < ps->maxDepth) {
        return ps->currentDepth + 1;
    }
    return ps->maxDepth;
}

bool progressive_should_stop(ProgressiveSearch* ps, int depth) {
    (void)depth;
    return ps->stopReason != STOP_NONE;
}

int progressive_aspiration_window(int score) {
    return PROGRESSIVE_ASPRATION_WINDOW;
}

void progressive_expand_window(int* alpha, int* beta) {
    *alpha -= PROGRESSIVE_ASPRATION_EXPANSION;
    *beta += PROGRESSIVE_ASPRATION_EXPANSION;
}
