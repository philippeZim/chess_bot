#ifndef LMR_H
#define LMR_H

#include "../types.h"

#define LMR_DEPTH_TABLE_SIZE 64
#define LMR_BASE_REDUCTION 1
#define LMR_MIN_DEPTH 1

typedef struct {
    int reductionTable[LMR_DEPTH_TABLE_SIZE][LMR_DEPTH_TABLE_SIZE];
} LmrTable;

typedef struct {
    int reduction;
    int historyBonus;
    bool isCapture;
} LmrConfig;

void lmr_init(LmrTable* lmr);
int lmr_reduction(LmrTable* lmr, int depth, int moveNum, bool isCapture, int history);
int lmr_base_reduction(int depth, int moveNum);
int lmr_adjust_reduction(int reduction, int history, bool isCapture);
int lmr_min_depth(int depth, int reduction);

#endif
