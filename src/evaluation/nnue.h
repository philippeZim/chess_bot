#ifndef NNUE_H
#define NNUE_H

#include "../types.h"
#include "../board/board.h"
#include "nnue_arch.h"
#include "nnue_io.h"
#include "nnue_update.h"

typedef struct {
    NNUEWeights weights;
    NNUEState state;
    bool loaded;
} NNUEContext;

void nnue_context_init(NNUEContext* ctx);
bool nnue_context_load(NNUEContext* ctx, const char* path);
void nnue_context_refresh(NNUEContext* ctx, const Board* board);
int nnue_evaluate(const Board* board, NNUEContext* ctx);
int nnue_evaluate_scalar(const Board* board, const NNUEWeights* weights);

#endif
