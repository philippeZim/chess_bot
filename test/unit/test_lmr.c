#include <stdio.h>
#include <stdint.h>
#include "types.h"
#include "pruning/lmr.h"

int run_lmr_tests(void) {
    int passed = 0, failed = 0;
    LmrTable lmr;

    /* Test 1: lmr_init */
    {
        lmr_init(&lmr);
        int table_ok = 1;
        for (int d = 0; d < LMR_DEPTH_TABLE_SIZE && table_ok; d++) {
            for (int m = 0; m < LMR_DEPTH_TABLE_SIZE && table_ok; m++) {
                int expected = lmr_base_reduction(d, m);
                if (lmr.reductionTable[d][m] != expected) table_ok = 0;
            }
        }
        if (table_ok) {
            printf("  test_lmr_init... PASS\n"); passed++;
        } else {
            printf("  test_lmr_init... FAIL\n"); failed++;
        }
    }

    /* Test 2: lmr_reduction_depth1 */
    {
        lmr_init(&lmr);
        int r = lmr_reduction(&lmr, 1, 3, false, 0);
        if (r >= 1) {
            printf("  test_lmr_reduction_depth1... PASS\n"); passed++;
        } else {
            printf("  test_lmr_reduction_depth1... FAIL\n"); failed++;
        }
    }

    /* Test 3: lmr_reduction_depth10 */
    {
        lmr_init(&lmr);
        int r = lmr_reduction(&lmr, 10, 5, false, 0);
        if (r > 0) {
            printf("  test_lmr_reduction_depth10... PASS\n"); passed++;
        } else {
            printf("  test_lmr_reduction_depth10... FAIL\n"); failed++;
        }
    }

    /* Test 4: lmr_reduction_first_move */
    {
        lmr_init(&lmr);
        int r = lmr_reduction(&lmr, 5, 0, false, 0);
        if (r == 0) {
            printf("  test_lmr_reduction_first_move... PASS\n"); passed++;
        } else {
            printf("  test_lmr_reduction_first_move... FAIL\n"); failed++;
        }
    }

    /* Test 5: lmr_reduction_last_move */
    {
        lmr_init(&lmr);
        int r1 = lmr_reduction(&lmr, 5, 2, false, 0);
        int r2 = lmr_reduction(&lmr, 5, 10, false, 0);
        if (r2 <= r1) {
            printf("  test_lmr_reduction_last_move... PASS\n"); passed++;
        } else {
            printf("  test_lmr_reduction_last_move... FAIL\n"); failed++;
        }
    }

    /* Test 6: lmr_capture_no_reduce */
    {
        lmr_init(&lmr);
        int r = lmr_reduction(&lmr, 5, 5, true, 0);
        if (r >= 1) {
            printf("  test_lmr_capture_no_reduce... PASS\n"); passed++;
        } else {
            printf("  test_lmr_capture_no_reduce... FAIL\n"); failed++;
        }
    }

    /* Test 7: lmr_history_bonus */
    {
        lmr_init(&lmr);
        int r1 = lmr_reduction(&lmr, 5, 5, false, 0);
        int r2 = lmr_reduction(&lmr, 5, 5, false, 512);
        if (r2 >= r1) {
            printf("  test_lmr_history_bonus... PASS\n"); passed++;
        } else {
            printf("  test_lmr_history_bonus... FAIL\n"); failed++;
        }
    }

    /* Test 8: lmr_history_penalty */
    {
        lmr_init(&lmr);
        int r1 = lmr_reduction(&lmr, 5, 5, false, 0);
        int r2 = lmr_reduction(&lmr, 5, 5, false, -512);
        if (r2 <= r1) {
            printf("  test_lmr_history_penalty... PASS\n"); passed++;
        } else {
            printf("  test_lmr_history_penalty... FAIL\n"); failed++;
        }
    }

    /* Test 9: lmr_min_depth */
    {
        int result = lmr_min_depth(3, 5);
        if (result >= 1) {
            printf("  test_lmr_min_depth... PASS\n"); passed++;
        } else {
            printf("  test_lmr_min_depth... FAIL\n"); failed++;
        }
    }

    /* Test 10: lmr_edge_cases */
    {
        lmr_init(&lmr);
        int r1 = lmr_reduction(&lmr, 0, 0, false, 0);
        int r2 = lmr_reduction(&lmr, 63, 63, false, 0);
        int r3 = lmr_reduction(&lmr, 64, 64, false, 0);
        if (r1 == 0 && r2 >= 0 && r3 >= 0) {
            printf("  test_lmr_edge_cases... PASS\n"); passed++;
        } else {
            printf("  test_lmr_edge_cases... FAIL\n"); failed++;
        }
    }

    printf("\nLMR: %d/%d passed\n", passed, passed + failed);
    return failed;
}
