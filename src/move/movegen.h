#ifndef MOVEGEN_H
#define MOVEGEN_H

#include "../types.h"
#include "../board/board.h"

typedef struct {
    Move moves[MAX_MOVES_PER_POSITION];
    int count;
} MoveList;

#define make_move(from, to) (((from) | ((to) << 6)))
#define make_promotion_move(from, to, promo) (((from) | ((to) << 6) | ((promo) << 12)))

void move_list_init(MoveList* list);
bool move_list_add(MoveList* list, Move move);

void generate_pseudolegal_moves(const Board* board, MoveList* moves);
void generate_legal_moves(const Board* board, MoveList* moves);
bool move_is_legal(const Board* board, Move move);

#endif
