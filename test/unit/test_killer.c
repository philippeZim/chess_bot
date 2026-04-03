#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "types.h"
#include "heuristics/killer.h"
#include "move/move.h"

int run_killer_tests(void) {
    int passed = 0, failed = 0;
    KillerTable kt;

    /* Test 1: killer_init */
    {
        killer_init(&kt);
        int all_zero = 1;
        for (int d = 0; d < MAX_DEPTH && all_zero; d++) {
            if (kt.killerMoves[d][0] != 0) all_zero = 0;
            if (kt.killerMoves[d][1] != 0) all_zero = 0;
            if (kt.killerCounts[d] != 0) all_zero = 0;
        }
        if (all_zero) {
            printf("  test_killer_init... PASS\n"); passed++;
        } else {
            printf("  test_killer_init... FAIL\n"); failed++;
        }
    }

    /* Test 2: killer_add_single */
    {
        killer_init(&kt);
        Move m = make_move(SQUARE_E2, SQUARE_E4);
        killer_add(&kt, 3, m);
        if (kt.killerMoves[3][0] == m && kt.killerCounts[3] == 1) {
            printf("  test_killer_add_single... PASS\n"); passed++;
        } else {
            printf("  test_killer_add_single... FAIL\n"); failed++;
        }
    }

    /* Test 3: killer_add_two */
    {
        killer_init(&kt);
        Move m1 = make_move(SQUARE_E2, SQUARE_E4);
        Move m2 = make_move(SQUARE_D2, SQUARE_D4);
        killer_add(&kt, 5, m1);
        killer_add(&kt, 5, m2);
        if (kt.killerMoves[5][0] == m1 && kt.killerMoves[5][1] == m2 && kt.killerCounts[5] == 2) {
            printf("  test_killer_add_two... PASS\n"); passed++;
        } else {
            printf("  test_killer_add_two... FAIL\n"); failed++;
        }
    }

    /* Test 4: killer_add_overflow */
    {
        killer_init(&kt);
        Move m1 = make_move(SQUARE_E2, SQUARE_E4);
        Move m2 = make_move(SQUARE_D2, SQUARE_D4);
        Move m3 = make_move(SQUARE_C2, SQUARE_C4);
        killer_add(&kt, 4, m1);
        killer_add(&kt, 4, m2);
        killer_add(&kt, 4, m3);
        if (kt.killerMoves[4][0] == m1 && kt.killerMoves[4][1] == m2 && kt.killerCounts[4] == 2) {
            printf("  test_killer_add_overflow... PASS\n"); passed++;
        } else {
            printf("  test_killer_add_overflow... FAIL\n"); failed++;
        }
    }

    /* Test 5: killer_get_depth */
    {
        killer_init(&kt);
        Move m1 = make_move(SQUARE_E2, SQUARE_E4);
        Move m2 = make_move(SQUARE_D2, SQUARE_D4);
        killer_add(&kt, 6, m1);
        killer_add(&kt, 6, m2);
        Move moves[2];
        killer_get(&kt, 6, moves);
        if (moves[0] == m1 && moves[1] == m2) {
            printf("  test_killer_get_depth... PASS\n"); passed++;
        } else {
            printf("  test_killer_get_depth... FAIL\n"); failed++;
        }
    }

    /* Test 6: killer_is_killer_true */
    {
        killer_init(&kt);
        Move m = make_move(SQUARE_E2, SQUARE_E4);
        killer_add(&kt, 2, m);
        if (killer_is_killer(&kt, 2, m)) {
            printf("  test_killer_is_killer_true... PASS\n"); passed++;
        } else {
            printf("  test_killer_is_killer_true... FAIL\n"); failed++;
        }
    }

    /* Test 7: killer_is_killer_false */
    {
        killer_init(&kt);
        Move m = make_move(SQUARE_E2, SQUARE_E4);
        Move other = make_move(SQUARE_D2, SQUARE_D4);
        killer_add(&kt, 2, m);
        if (!killer_is_killer(&kt, 2, other)) {
            printf("  test_killer_is_killer_false... PASS\n"); passed++;
        } else {
            printf("  test_killer_is_killer_false... FAIL\n"); failed++;
        }
    }

    /* Test 8: killer_clear */
    {
        killer_init(&kt);
        Move m = make_move(SQUARE_E2, SQUARE_E4);
        killer_add(&kt, 3, m);
        killer_clear(&kt);
        if (kt.killerMoves[3][0] == 0 && kt.killerCounts[3] == 0) {
            printf("  test_killer_clear... PASS\n"); passed++;
        } else {
            printf("  test_killer_clear... FAIL\n"); failed++;
        }
    }

    /* Test 9: killer_clear_depth */
    {
        killer_init(&kt);
        Move m1 = make_move(SQUARE_E2, SQUARE_E4);
        Move m2 = make_move(SQUARE_D2, SQUARE_D4);
        killer_add(&kt, 3, m1);
        killer_add(&kt, 5, m2);
        killer_clear_depth(&kt, 3);
        if (kt.killerMoves[3][0] == 0 && kt.killerCounts[3] == 0 &&
            kt.killerMoves[5][0] == m2 && kt.killerCounts[5] == 1) {
            printf("  test_killer_clear_depth... PASS\n"); passed++;
        } else {
            printf("  test_killer_clear_depth... FAIL\n"); failed++;
        }
    }

    /* Test 10: killer_order_moves */
    {
        killer_init(&kt);
        Move k1 = make_move(SQUARE_E2, SQUARE_E4);
        Move k2 = make_move(SQUARE_D2, SQUARE_D4);
        killer_add(&kt, 4, k1);
        killer_add(&kt, 4, k2);

        Move moves[4];
        moves[0] = make_move(SQUARE_C2, SQUARE_C4);
        moves[1] = k2;
        moves[2] = make_move(SQUARE_B2, SQUARE_B4);
        moves[3] = k1;

        killer_order_moves(&kt, 4, moves, 4);

        if (moves[0] == k1 && moves[1] == k2) {
            printf("  test_killer_order_moves... PASS\n"); passed++;
        } else {
            printf("  test_killer_order_moves... FAIL\n"); failed++;
        }
    }

    /* Test 11: killer_cross_depth */
    {
        killer_init(&kt);
        Move m1 = make_move(SQUARE_E2, SQUARE_E4);
        Move m2 = make_move(SQUARE_D2, SQUARE_D4);
        killer_add(&kt, 3, m1);
        killer_add(&kt, 5, m2);

        if (kt.killerMoves[3][0] == m1 && kt.killerMoves[5][0] == m2 &&
            !killer_is_killer(&kt, 3, m2) && !killer_is_killer(&kt, 5, m1)) {
            printf("  test_killer_cross_depth... PASS\n"); passed++;
        } else {
            printf("  test_killer_cross_depth... FAIL\n"); failed++;
        }
    }

    printf("\nKiller: %d/%d passed\n", passed, passed + failed);
    return failed;
}
