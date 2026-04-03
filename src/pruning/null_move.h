#ifndef NULL_MOVE_H
#define NULL_MOVE_H

#include "../types.h"
#include "../board/board.h"

#define NULL_DEPTH_REDUCTION 3
#define NULL_MIN_DEPTH 2
#define NULL_BETA_MARGIN 100

typedef struct {
    int nullDepthReduction;
    int nullMinDepth;
    int nullBetaMargin;
} NullMoveConfig;

void null_move_init(NullMoveConfig* nm);
bool null_move_can_prune(NullMoveConfig* nm, int depth, int beta, int eval);
int null_move_reduction(NullMoveConfig* nm, int depth);
int null_move_verify(NullMoveConfig* nm, int depth, int beta);
void null_move_apply(Board* board);
void null_move_unapply(Board* board);

#endif
