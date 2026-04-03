#ifndef TIME_MGMT_H
#define TIME_MGMT_H

#include "../types.h"
#include "../board/board.h"

#define TIME_DEFAULT_MOVES_TO_GO 40
#define TIME_MAX_TIME_FRACTION 75
#define TIME_OPTIMAL_TIME_FRACTION 50

typedef struct {
    uint64_t startTime;
    uint64_t optimalTime;
    uint64_t maxTime;
    uint64_t moveTime;
    int increments;
    int movesToGo;
} TimeControl;

typedef struct {
    TimeControl tc;
    uint64_t timeUsed;
} TimeManagement;

void time_mgmt_init(TimeManagement* tm, TimeControl* tc);
void time_mgmt_allocate(TimeManagement* tm, Board* board, uint64_t totalTime);
uint64_t time_mgmt_optimal_time(TimeManagement* tm);
uint64_t time_mgmt_max_time(TimeManagement* tm);
uint64_t time_mgmt_elapsed(TimeManagement* tm);
uint64_t time_mgmt_remaining(TimeManagement* tm);
bool time_mgmt_should_stop(TimeManagement* tm, int depth, int score);
int time_mgmt_scale(int depth, int movesToGo);

#endif
