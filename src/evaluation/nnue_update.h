#ifndef NNUE_UPDATE_H
#define NNUE_UPDATE_H

#include "../types.h"
#include "../board/board.h"
#include "nnue_arch.h"
#include "nnue_io.h"

typedef struct {
    int16_t values[NNUE_HIDDEN_SIZE];
    bool computed;
} Accumulator;

typedef struct {
    Accumulator accumulators[COLOR_NB][MAX_PLY];
    int current_ply;
} NNUEState;

void nnue_state_init(NNUEState* state);
void nnue_state_reset(NNUEState* state);
void nnue_state_push(NNUEState* state);
void nnue_state_pop(NNUEState* state);

void nnue_full_refresh(const Board* board, const NNUEWeights* weights,
                       Color perspective, Accumulator* acc);

int nnue_feature_index(Color side, Square king_sq, Square piece_sq, Piece piece);

void nnue_update_incremental(NNUEState* state, const Board* board,
                             const NNUEWeights* weights, int from, int to,
                             Piece captured);

#endif
