#include "lmr.h"
#include <math.h>
#include <stdlib.h>

void lmr_init(LmrTable* lmr) {
    for (int depth = 0; depth < LMR_DEPTH_TABLE_SIZE; depth++) {
        for (int moveNum = 0; moveNum < LMR_DEPTH_TABLE_SIZE; moveNum++) {
            lmr->reductionTable[depth][moveNum] = lmr_base_reduction(depth, moveNum);
        }
    }
}

int lmr_reduction(LmrTable* lmr, int depth, int moveNum, bool isCapture, int history) {
    if (depth < LMR_MIN_DEPTH || moveNum == 0) return 0;
    
    int reduction = lmr->reductionTable[depth < LMR_DEPTH_TABLE_SIZE ? depth : LMR_DEPTH_TABLE_SIZE - 1]
                                       [moveNum < LMR_DEPTH_TABLE_SIZE ? moveNum : LMR_DEPTH_TABLE_SIZE - 1];
    
    reduction = lmr_adjust_reduction(reduction, history, isCapture);
    reduction = lmr_min_depth(depth, reduction);
    
    return reduction;
}

int lmr_base_reduction(int depth, int moveNum) {
    if (depth <= 1 || moveNum <= 1) return 0;
    
    int reduction = (int)(log(depth) * log(moveNum) / 2.0);
    return reduction;
}

int lmr_adjust_reduction(int reduction, int history, bool isCapture) {
    if (isCapture) return 0;
    
    if (history > 0) {
        reduction -= history / 256;
    } else if (history < 0) {
        reduction += (-history) / 256;
    }
    
    if (reduction < 0) reduction = 0;
    return reduction;
}

int lmr_min_depth(int depth, int reduction) {
    int newDepth = depth - reduction;
    return newDepth < 1 ? 1 : newDepth;
}
