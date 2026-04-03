#include <stdio.h>
#include <stdint.h>
#include "types.h"
#include "pruning/lmp.h"

int run_lmp_tests(void) {
    int passed = 0, failed = 0;
    LmpConfig cfg;
    LmpTable lmp;

    /* Test 1: lmp_init */
    {
        lmp_init(&cfg, &lmp);
        if (cfg.baseLmpMoves == LMP_BASE_MOVES && lmp.lmpTable[0] == LMP_BASE_MOVES) {
            printf("  test_lmp_init... PASS\n"); passed++;
        } else {
            printf("  test_lmp_init... FAIL\n"); failed++;
        }
    }

    /* Test 2: lmp_max_moves_depth0 */
    {
        lmp_init(&cfg, &lmp);
        int moves = lmp_max_moves(&lmp, 0);
        if (moves == LMP_BASE_MOVES) {
            printf("  test_lmp_max_moves_depth0... PASS\n"); passed++;
        } else {
            printf("  test_lmp_max_moves_depth0... FAIL\n"); failed++;
        }
    }

    /* Test 3: lmp_max_moves_depth8 */
    {
        lmp_init(&cfg, &lmp);
        int moves = lmp_max_moves(&lmp, 8);
        int expected = LMP_BASE_MOVES + 8 * 8;
        if (moves == expected) {
            printf("  test_lmp_max_moves_depth8... PASS\n"); passed++;
        } else {
            printf("  test_lmp_max_moves_depth8... FAIL\n"); failed++;
        }
    }

    /* Test 4: lmp_should_prune_false */
    {
        lmp_init(&cfg, &lmp);
        if (!lmp_should_prune(&lmp, 3, 5)) {
            printf("  test_lmp_should_prune_false... PASS\n"); passed++;
        } else {
            printf("  test_lmp_should_prune_false... FAIL\n"); failed++;
        }
    }

    /* Test 5: lmp_should_prune_true */
    {
        lmp_init(&cfg, &lmp);
        int maxMoves = lmp_max_moves(&lmp, 3);
        if (lmp_should_prune(&lmp, 3, maxMoves)) {
            printf("  test_lmp_should_prune_true... PASS\n"); passed++;
        } else {
            printf("  test_lmp_should_prune_true... FAIL\n"); failed++;
        }
    }

    /* Test 6: lmp_quiet_only */
    {
        lmp_init(&cfg, &lmp);
        int moves_d1 = lmp_max_moves(&lmp, 1);
        int moves_d2 = lmp_max_moves(&lmp, 2);
        if (moves_d2 > moves_d1) {
            printf("  test_lmp_quiet_only... PASS\n"); passed++;
        } else {
            printf("  test_lmp_quiet_only... FAIL\n"); failed++;
        }
    }

    /* Test 7: lmp_edge_cases */
    {
        lmp_init(&cfg, &lmp);
        int neg = lmp_max_moves(&lmp, -1);
        int overflow = lmp_max_moves(&lmp, 100);
        if (neg == 0 && overflow == 0) {
            printf("  test_lmp_edge_cases... PASS\n"); passed++;
        } else {
            printf("  test_lmp_edge_cases... FAIL\n"); failed++;
        }
    }

    printf("\nLMP: %d/%d passed\n", passed, passed + failed);
    return failed;
}
