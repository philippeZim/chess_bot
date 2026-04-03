#ifndef PERFT_H
#define PERFT_H

#include "../types.h"
#include "../move/movegen.h"

uint64_t perft_depth(const Board* board, int depth);
uint64_t perft_divide(const Board* board, int depth);
uint64_t perft(const Board* board, int depth, int print_moves);

#endif
