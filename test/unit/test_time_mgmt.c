#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "types.h"
#include "time/time_mgmt.h"
#include "board/board.h"

int run_time_mgmt_tests(void) {
    int passed = 0, failed = 0;
    TimeManagement tm;
    TimeControl tc;
    Board board;

    /* Test 1: time_mgmt_init */
    {
        memset(&tc, 0, sizeof(TimeControl));
        tc.increments = 100;
        tc.movesToGo = 40;
        time_mgmt_init(&tm, &tc);
        if (tm.tc.increments == 100 && tm.tc.movesToGo == 40 && tm.timeUsed == 0) {
            printf("  test_time_mgmt_init... PASS\n"); passed++;
        } else {
            printf("  test_time_mgmt_init... FAIL\n"); failed++;
        }
    }

    /* Test 2: time_mgmt_allocate_simple */
    {
        memset(&tc, 0, sizeof(TimeControl));
        tc.increments = 0;
        tc.movesToGo = 40;
        time_mgmt_init(&tm, &tc);
        board_init(&board);
        time_mgmt_allocate(&tm, &board, 60000);
        if (tm.tc.optimalTime > 0 && tm.tc.maxTime > 0) {
            printf("  test_time_mgmt_allocate_simple... PASS\n"); passed++;
        } else {
            printf("  test_time_mgmt_allocate_simple... FAIL\n"); failed++;
        }
    }

    /* Test 3: time_mgmt_allocate_increment */
    {
        memset(&tc, 0, sizeof(TimeControl));
        tc.increments = 500;
        tc.movesToGo = 40;
        time_mgmt_init(&tm, &tc);
        board_init(&board);
        time_mgmt_allocate(&tm, &board, 60000);
        uint64_t opt = time_mgmt_optimal_time(&tm);
        if (opt > 500) {
            printf("  test_time_mgmt_allocate_increment... PASS\n"); passed++;
        } else {
            printf("  test_time_mgmt_allocate_increment... FAIL\n"); failed++;
        }
    }

    /* Test 4: time_mgmt_allocate_moves_go */
    {
        memset(&tc, 0, sizeof(TimeControl));
        tc.increments = 0;
        tc.movesToGo = 10;
        time_mgmt_init(&tm, &tc);
        board_init(&board);
        time_mgmt_allocate(&tm, &board, 60000);
        uint64_t opt1 = tm.tc.optimalTime;

        tc.movesToGo = 40;
        time_mgmt_init(&tm, &tc);
        time_mgmt_allocate(&tm, &board, 60000);
        uint64_t opt2 = tm.tc.optimalTime;

        if (opt1 > opt2) {
            printf("  test_time_mgmt_allocate_moves_go... PASS\n"); passed++;
        } else {
            printf("  test_time_mgmt_allocate_moves_go... FAIL\n"); failed++;
        }
    }

    /* Test 5: time_mgmt_optimal_time_calc */
    {
        memset(&tc, 0, sizeof(TimeControl));
        tc.optimalTime = 5000;
        time_mgmt_init(&tm, &tc);
        if (time_mgmt_optimal_time(&tm) == 5000) {
            printf("  test_time_mgmt_optimal_time_calc... PASS\n"); passed++;
        } else {
            printf("  test_time_mgmt_optimal_time_calc... FAIL\n"); failed++;
        }
    }

    /* Test 6: time_mgmt_max_time_calc */
    {
        memset(&tc, 0, sizeof(TimeControl));
        tc.maxTime = 10000;
        time_mgmt_init(&tm, &tc);
        if (time_mgmt_max_time(&tm) == 10000) {
            printf("  test_time_mgmt_max_time_calc... PASS\n"); passed++;
        } else {
            printf("  test_time_mgmt_max_time_calc... FAIL\n"); failed++;
        }
    }

    /* Test 7: time_mgmt_elapsed */
    {
        memset(&tc, 0, sizeof(TimeControl));
        time_mgmt_init(&tm, &tc);
        tm.tc.startTime = 1000;
        uint64_t elapsed = time_mgmt_elapsed(&tm);
        if (elapsed > 0) {
            printf("  test_time_mgmt_elapsed... PASS\n"); passed++;
        } else {
            printf("  test_time_mgmt_elapsed... FAIL\n"); failed++;
        }
    }

    /* Test 8: time_mgmt_remaining */
    {
        memset(&tc, 0, sizeof(TimeControl));
        tc.optimalTime = 60000;
        time_mgmt_init(&tm, &tc);
        tm.tc.startTime = time_mgmt_elapsed(&tm);
        uint64_t remaining = time_mgmt_remaining(&tm);
        if (remaining <= 60000) {
            printf("  test_time_mgmt_remaining... PASS\n"); passed++;
        } else {
            printf("  test_time_mgmt_remaining... FAIL\n"); failed++;
        }
    }

    /* Test 9: time_mgmt_should_stop_elapsed */
    {
        memset(&tc, 0, sizeof(TimeControl));
        tc.optimalTime = 1;
        time_mgmt_init(&tm, &tc);
        /* Wait a bit for elapsed time */
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 5000000;
        nanosleep(&ts, NULL);
        if (time_mgmt_should_stop(&tm, 1, 0)) {
            printf("  test_time_mgmt_should_stop_elapsed... PASS\n"); passed++;
        } else {
            printf("  test_time_mgmt_should_stop_elapsed... FAIL\n"); failed++;
        }
    }

    /* Test 10: time_mgmt_should_stop_safe */
    {
        memset(&tc, 0, sizeof(TimeControl));
        tc.optimalTime = 999999999;
        time_mgmt_init(&tm, &tc);
        if (!time_mgmt_should_stop(&tm, 1, 0)) {
            printf("  test_time_mgmt_should_stop_safe... PASS\n"); passed++;
        } else {
            printf("  test_time_mgmt_should_stop_safe... FAIL\n"); failed++;
        }
    }

    /* Test 11: time_mgmt_scale_depth */
    {
        int scale = time_mgmt_scale(5, 40);
        if (scale >= 50 && scale <= 200) {
            printf("  test_time_mgmt_scale_depth... PASS\n"); passed++;
        } else {
            printf("  test_time_mgmt_scale_depth... FAIL\n"); failed++;
        }
    }

    /* Test 12: time_mgmt_scale_movesGo */
    {
        int scale1 = time_mgmt_scale(5, 10);
        int scale2 = time_mgmt_scale(5, 40);
        if (scale1 > scale2) {
            printf("  test_time_mgmt_scale_movesGo... PASS\n"); passed++;
        } else {
            printf("  test_time_mgmt_scale_movesGo... FAIL\n"); failed++;
        }
    }

    printf("\nTime Mgmt: %d/%d passed\n", passed, passed + failed);
    return failed;
}
