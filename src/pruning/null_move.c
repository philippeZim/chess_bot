#include "null_move.h"
#include "evaluation/evaluate.h"

void null_move_init(NullMoveConfig* nm) {
    nm->nullDepthReduction = NULL_DEPTH_REDUCTION;
    nm->nullMinDepth = NULL_MIN_DEPTH;
    nm->nullBetaMargin = NULL_BETA_MARGIN;
}

bool null_move_can_prune(NullMoveConfig* nm, int depth, int beta, int eval) {
    if (depth < nm->nullMinDepth) return false;
    if (eval < beta - nm->nullBetaMargin) return false;
    return true;
}

int null_move_reduction(NullMoveConfig* nm, int depth) {
    int r = nm->nullDepthReduction + depth / 4;
    return r < 2 ? 2 : r;
}

int null_move_verify(NullMoveConfig* nm, int depth, int beta) {
    int verifyDepth = depth - null_move_reduction(nm, depth);
    return verifyDepth < 1 ? 1 : verifyDepth;
}

void null_move_apply(Board* board) {
    board->turn = (Color)(board->turn ^ 1);
    board->halfMoves++;
}

void null_move_unapply(Board* board) {
    board->turn = (Color)(board->turn ^ 1);
    board->halfMoves--;
}
