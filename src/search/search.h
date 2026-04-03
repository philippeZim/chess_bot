#ifndef SEARCH_H
#define SEARCH_H

#include "../types.h"
#include "../board/board.h"
#include "../move/movegen.h"
#include "../transposition/transposition.h"

typedef struct {
    Move moves[MAX_MOVES_PER_POSITION];
    int count;
} PVLine;

typedef struct {
    TranspositionTable tt;
    uint64_t nodes;
    uint64_t time_start;
    int depth;
    PVLine pv;
} SearchContext;

#define PV_INIT(pv) do { (pv)->count = 0; } while(0)
#define PV_ADD(pv, move) do { \
    if ((pv)->count < MAX_MOVES_PER_POSITION) { \
        (pv)->moves[(pv)->count++] = (move); \
    } \
} while(0)

void search_init(SearchContext* ctx);
void search_clear(SearchContext* ctx);
int search_negamax(SearchContext* ctx, Board* board, int depth, int alpha, int beta);
int search_iterative_deepening(SearchContext* ctx, Board* board, int max_depth, int* best_move);
int evaluate_position(SearchContext* ctx, const Board* board);
int qsearch(SearchContext* ctx, Board* board, int alpha, int beta);

#endif
