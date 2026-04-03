#ifndef PROGRESSIVE_H
#define PROGRESSIVE_H

#include "types.h"
#include "board/board.h"

#define PROGRESSIVE_ASPRATION_WINDOW 10
#define PROGRESSIVE_ASPRATION_EXPANSION 20

typedef struct {
    Move bestMove;
    int bestScore;
    int currentDepth;
    int maxDepth;
    int stopReason;
} ProgressiveSearch;

typedef enum {
    STOP_NONE = 0,
    STOP_TIME = 1,
    STOP_MOVES = 2,
    STOP_USER = 4
} StopReasonEnum;

void progressive_init(ProgressiveSearch* ps, int maxDepth);
int progressive_search(ProgressiveSearch* ps, Board* board, int maxDepth);
int progressive_next_depth(ProgressiveSearch* ps);
bool progressive_should_stop(ProgressiveSearch* ps, int depth);
int progressive_aspiration_window(int score);
void progressive_expand_window(int* alpha, int* beta);

#endif
