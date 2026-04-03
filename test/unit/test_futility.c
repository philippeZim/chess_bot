#include <stdio.h>
#include <stdint.h>
#include "types.h"
#include "pruning/futility.h"

int run_futility_tests(void) {
    int passed = 0, failed = 0;
    FutilityMargins fm;

    /* Test 1: futility_init */
    {
        futility_init(&fm);
        if (fm.baseMargin == FUTILITY_BASE_MARGIN && fm.depthMargin == FUTILITY_DEPTH_MARGIN) {
            printf("  test_futility_init... PASS\n"); passed++;
        } else {
            printf("  test_futility_init... FAIL\n"); failed++;
        }
    }

    /* Test 2: futility_margin_depth1 */
    {
        futility_init(&fm);
        int margin = futility_margin(&fm, 1);
        int expected = FUTILITY_BASE_MARGIN + FUTILITY_DEPTH_MARGIN * 1;
        if (margin == expected) {
            printf("  test_futility_margin_depth1... PASS\n"); passed++;
        } else {
            printf("  test_futility_margin_depth1... FAIL\n"); failed++;
        }
    }

    /* Test 3: futility_margin_depth5 */
    {
        futility_init(&fm);
        int margin = futility_margin(&fm, 5);
        int expected = FUTILITY_BASE_MARGIN + FUTILITY_DEPTH_MARGIN * 5;
        if (margin == expected) {
            printf("  test_futility_margin_depth5... PASS\n"); passed++;
        } else {
            printf("  test_futility_margin_depth5... FAIL\n"); failed++;
        }
    }

    /* Test 4: futility_can_prune_true */
    {
        futility_init(&fm);
        int depth = 2;
        int beta = 200;
        int eval = -300;
        if (futility_can_prune(&fm, depth, beta, eval)) {
            printf("  test_futility_can_prune_true... PASS\n"); passed++;
        } else {
            printf("  test_futility_can_prune_true... FAIL\n"); failed++;
        }
    }

    /* Test 5: futility_can_prune_false */
    {
        futility_init(&fm);
        int depth = 2;
        int beta = 200;
        int eval = 300;
        if (!futility_can_prune(&fm, depth, beta, eval)) {
            printf("  test_futility_can_prune_false... PASS\n"); passed++;
        } else {
            printf("  test_futility_can_prune_false... FAIL\n"); failed++;
        }
    }

    /* Test 6: futility_prune_score */
    {
        int eval = 100;
        int margin = 150;
        int score = futility_prune_score(eval, margin);
        if (score == 250) {
            printf("  test_futility_prune_score... PASS\n"); passed++;
        } else {
            printf("  test_futility_prune_score... FAIL\n"); failed++;
        }
    }

    /* Test 7: futility_depth_zero */
    {
        futility_init(&fm);
        if (!futility_can_prune(&fm, 0, 100, 50)) {
            printf("  test_futility_depth_zero... PASS\n"); passed++;
        } else {
            printf("  test_futility_depth_zero... FAIL\n"); failed++;
        }
    }

    /* Test 8: futility_edge_case_beta */
    {
        futility_init(&fm);
        int depth = 1;
        int beta = -30000;
        int eval = -30000;
        if (!futility_can_prune(&fm, depth, beta, eval)) {
            printf("  test_futility_edge_case_beta... PASS\n"); passed++;
        } else {
            printf("  test_futility_edge_case_beta... FAIL\n"); failed++;
        }
    }

    /* Test 9: futility_edge_case_eval */
    {
        futility_init(&fm);
        int depth = 3;
        int beta = 100;
        int eval = 99;
        int margin = futility_margin(&fm, depth);
        if (eval + margin < beta || !futility_can_prune(&fm, depth, beta, eval)) {
            printf("  test_futility_edge_case_eval... PASS\n"); passed++;
        } else {
            printf("  test_futility_edge_case_eval... FAIL\n"); failed++;
        }
    }

    printf("\nFutility: %d/%d passed\n", passed, passed + failed);
    return failed;
}
