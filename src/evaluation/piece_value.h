#ifndef PIECE_VALUE_H
#define PIECE_VALUE_H

#include "../types.h"
#include "../board/board.h"

typedef struct {
    int mg;
    int eg;
} EvalScore;

#define SCORE_ZERO ((EvalScore){0, 0})

static inline EvalScore score_make(int mg, int eg) {
    EvalScore s = {mg, eg};
    return s;
}

static inline EvalScore score_add(EvalScore a, EvalScore b) {
    EvalScore s = {a.mg + b.mg, a.eg + b.eg};
    return s;
}

static inline EvalScore score_sub(EvalScore a, EvalScore b) {
    EvalScore s = {a.mg - b.mg, a.eg - b.eg};
    return s;
}

static inline EvalScore score_scale(EvalScore s, int factor) {
    EvalScore r = {s.mg * factor, s.eg * factor};
    return r;
}

static inline int score_mg(EvalScore s) {
    return s.mg;
}

static inline int score_eg(EvalScore s) {
    return s.eg;
}

int piece_value_mg(PieceType pt);
int piece_value_eg(PieceType pt);

EvalScore eval_psqt(const Board* board, Color color);

EvalScore eval_knight_outposts(const Board* board, Color color);
EvalScore eval_bishop_pair(const Board* board, Color color);
EvalScore eval_rook_activity(const Board* board, Color color);
EvalScore eval_king_safety_psqt(const Board* board, Color color);

#endif
