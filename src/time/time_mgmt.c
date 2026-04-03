#include "time_mgmt.h"
#include <time.h>
#include <string.h>

static uint64_t get_current_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

void time_mgmt_init(TimeManagement* tm, TimeControl* tc) {
    memcpy(&tm->tc, tc, sizeof(TimeControl));
    tm->timeUsed = 0;
}

void time_mgmt_allocate(TimeManagement* tm, Board* board, uint64_t totalTime) {
    (void)board;
    uint64_t increment = tm->tc.increments;
    int movesToGo = tm->tc.movesToGo > 0 ? tm->tc.movesToGo : TIME_DEFAULT_MOVES_TO_GO;
    
    uint64_t timeBase = totalTime / movesToGo;
    uint64_t optimal = (timeBase * TIME_OPTIMAL_TIME_FRACTION) / 100 + increment;
    uint64_t max = (timeBase * TIME_MAX_TIME_FRACTION) / 100 + increment * 2;
    
    tm->tc.optimalTime = optimal < totalTime ? optimal : totalTime / 4;
    tm->tc.maxTime = max < totalTime ? max : totalTime / 2;
    tm->tc.startTime = get_current_time_ms();
}

uint64_t time_mgmt_optimal_time(TimeManagement* tm) {
    return tm->tc.optimalTime;
}

uint64_t time_mgmt_max_time(TimeManagement* tm) {
    return tm->tc.maxTime;
}

uint64_t time_mgmt_elapsed(TimeManagement* tm) {
    return get_current_time_ms() - tm->tc.startTime;
}

uint64_t time_mgmt_remaining(TimeManagement* tm) {
    return tm->tc.optimalTime - time_mgmt_elapsed(tm);
}

bool time_mgmt_should_stop(TimeManagement* tm, int depth, int score) {
    (void)depth;
    (void)score;
    uint64_t elapsed = time_mgmt_elapsed(tm);
    return elapsed >= tm->tc.optimalTime;
}

int time_mgmt_scale(int depth, int movesToGo) {
    int scale = 100;
    if (movesToGo > 0) {
        scale = 100 + (40 - movesToGo) * 2;
        if (scale < 50) scale = 50;
        if (scale > 200) scale = 200;
    }
    return scale;
}
