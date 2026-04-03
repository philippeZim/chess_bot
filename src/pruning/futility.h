#ifndef FUTILITY_H
#define FUTILITY_H

#include "types.h"

#define FUTILITY_BASE_MARGIN 125
#define FUTILITY_DEPTH_MARGIN 85

typedef struct {
    int baseMargin;
    int depthMargin;
} FutilityMargins;

void futility_init(FutilityMargins* fm);
bool futility_can_prune(FutilityMargins* fm, int depth, int beta, int eval);
int futility_margin(FutilityMargins* fm, int depth);
int futility_prune_score(int eval, int margin);

#endif
