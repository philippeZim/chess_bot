#ifndef EVALUATE_H
#define EVALUATE_H

#include "../types.h"
#include "../board/board.h"
#include "piece_value.h"
#include "nnue.h"

typedef struct {
    int mg_score;
    int eg_score;
    int game_phase;
} EvalResult;

int evaluate(const Board* board);
int evaluate_with_nnue(const Board* board, NNUEContext* ctx);
EvalResult evaluate_full(const Board* board);
int compute_game_phase(const Board* board);
int blend_scores(int mg, int eg, int phase);

#endif
