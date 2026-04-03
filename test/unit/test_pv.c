#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/search/search.h"
#include "../../src/types.h"
#include "../../src/board/board.h"
#include "../../src/move/movegen.h"

#define ASSERT_EQ(actual, expected, msg) do { \
    if ((actual) != (expected)) { \
        printf("FAIL: %s - Expected %d, got %d\n", msg, (expected), (int)(actual)); \
        failures++; \
    } else { \
        printf("PASS: %s\n", msg); \
        passes++; \
    } \
} while(0)

#define ASSERT_TRUE(cond, msg) do { \
    if (!(cond)) { \
        printf("FAIL: %s\n", msg); \
        failures++; \
    } else { \
        printf("PASS: %s\n", msg); \
        passes++; \
    } \
} while(0)

static int passes = 0;
static int failures = 0;

int test_pv_init(void) {
    printf("\n=== PV Init Tests ===\n");
    
    PVLine pv;
    PV_INIT(&pv);
    
    ASSERT_EQ(pv.count, 0, "PV count initialized to 0");
    
    return failures;
}

int test_pv_add_move(void) {
    printf("\n=== PV Add Move Tests ===\n");
    
    PVLine pv;
    PV_INIT(&pv);
    
    Move move1 = make_move(SQUARE_E2, SQUARE_E4);
    PV_ADD(&pv, move1);
    
    ASSERT_EQ(pv.count, 1, "PV count is 1 after adding first move");
    ASSERT_EQ(pv.moves[0], move1, "First move stored correctly");
    
    Move move2 = make_move(SQUARE_E7, SQUARE_E5);
    PV_ADD(&pv, move2);
    
    ASSERT_EQ(pv.count, 2, "PV count is 2 after adding second move");
    ASSERT_EQ(pv.moves[1], move2, "Second move stored correctly");
    
    return failures;
}

int test_pv_max_moves(void) {
    printf("\n=== PV Max Moves Tests ===\n");
    
    PVLine pv;
    PV_INIT(&pv);
    
    for (int i = 0; i < MAX_MOVES_PER_POSITION; i++) {
        PV_ADD(&pv, make_move(SQUARE_A1, (Square)(SQUARE_A1 + i % 64)));
    }
    
    ASSERT_EQ(pv.count, MAX_MOVES_PER_POSITION, "PV count at max");
    
    Move extra_move = make_move(SQUARE_H8, SQUARE_G8);
    PV_ADD(&pv, extra_move);
    
    ASSERT_EQ(pv.count, MAX_MOVES_PER_POSITION, "PV count doesn't exceed max");
    
    return failures;
}

int test_pv_clear(void) {
    printf("\n=== PV Clear Tests ===\n");
    
    PVLine pv;
    PV_INIT(&pv);
    
    PV_ADD(&pv, make_move(SQUARE_E2, SQUARE_E4));
    PV_ADD(&pv, make_move(SQUARE_E7, SQUARE_E5));
    
    ASSERT_EQ(pv.count, 2, "PV count is 2");
    
    PV_INIT(&pv);
    
    ASSERT_EQ(pv.count, 0, "PV cleared");
    
    return failures;
}

int test_pv_sequence(void) {
    printf("\n=== PV Sequence Tests ===\n");
    
    PVLine pv;
    PV_INIT(&pv);
    
    Move moves[] = {
        make_move(SQUARE_E2, SQUARE_E4),
        make_move(SQUARE_E7, SQUARE_E5),
        make_move(SQUARE_G1, SQUARE_F3),
        make_move(SQUARE_B8, SQUARE_C6),
        make_move(SQUARE_F1, SQUARE_C4)
    };
    
    int move_count = sizeof(moves) / sizeof(moves[0]);
    
    for (int i = 0; i < move_count; i++) {
        PV_ADD(&pv, moves[i]);
    }
    
    ASSERT_EQ(pv.count, move_count, "PV has all moves");
    
    for (int i = 0; i < move_count; i++) {
        ASSERT_EQ(pv.moves[i], moves[i], "Move at index matches");
    }
    
    return failures;
}

int run_pv_tests(void) {
    printf("Principal Variation Tests\n");
    printf("=========================\n");
    
    test_pv_init();
    test_pv_add_move();
    test_pv_max_moves();
    test_pv_clear();
    test_pv_sequence();
    
    printf("\n=========================\n");
    printf("Passed: %d, Failed: %d\n", passes, failures);
    printf("=========================\n");
    
    return failures;
}
