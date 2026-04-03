#include <stdio.h>
#include <stdint.h>
#include "types.h"
#include "search/progressive.h"
#include "board/board.h"

int run_progressive_tests(void) {
    int passed = 0, failed = 0;
    ProgressiveSearch ps;
    Board board;

    /* Test 1: progressive_init */
    {
        progressive_init(&ps, 10);
        if (ps.bestMove == 0 && ps.bestScore == 0 &&
            ps.currentDepth == 0 && ps.maxDepth == 10 &&
            ps.stopReason == STOP_NONE) {
            printf("  test_progressive_init... PASS\n"); passed++;
        } else {
            printf("  test_progressive_init... FAIL\n"); failed++;
        }
    }

    /* Test 2: progressive_search_depth1 */
    {
        board_init(&board);
        board_set_fen(&board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        progressive_init(&ps, 1);
        progressive_search(&ps, &board, 1);
        if (ps.currentDepth == 1) {
            printf("  test_progressive_search_depth1... PASS\n"); passed++;
        } else {
            printf("  test_progressive_search_depth1... FAIL\n"); failed++;
        }
    }

    /* Test 3: progressive_search_complete */
    {
        board_init(&board);
        board_set_fen(&board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        progressive_init(&ps, 5);
        progressive_search(&ps, &board, 5);
        if (ps.currentDepth == 5) {
            printf("  test_progressive_search_complete... PASS\n"); passed++;
        } else {
            printf("  test_progressive_search_complete... FAIL\n"); failed++;
        }
    }

    /* Test 4: progressive_next_depth */
    {
        progressive_init(&ps, 10);
        ps.currentDepth = 3;
        int next = progressive_next_depth(&ps);
        if (next == 4) {
            printf("  test_progressive_next_depth... PASS\n"); passed++;
        } else {
            printf("  test_progressive_next_depth... FAIL\n"); failed++;
        }
    }

    /* Test 5: progressive_should_stop_time */
    {
        progressive_init(&ps, 10);
        ps.stopReason = STOP_TIME;
        if (progressive_should_stop(&ps, 5)) {
            printf("  test_progressive_should_stop_time... PASS\n"); passed++;
        } else {
            printf("  test_progressive_should_stop_time... FAIL\n"); failed++;
        }
    }

    /* Test 6: progressive_should_stop_moves */
    {
        progressive_init(&ps, 10);
        ps.stopReason = STOP_MOVES;
        if (progressive_should_stop(&ps, 3)) {
            printf("  test_progressive_should_stop_moves... PASS\n"); passed++;
        } else {
            printf("  test_progressive_should_stop_moves... FAIL\n"); failed++;
        }
    }

    /* Test 7: progressive_aspiration_window */
    {
        int window = progressive_aspiration_window(100);
        if (window == PROGRESSIVE_ASPRATION_WINDOW) {
            printf("  test_progressive_aspiration_window... PASS\n"); passed++;
        } else {
            printf("  test_progressive_aspiration_window... FAIL\n"); failed++;
        }
    }

    /* Test 8: progressive_aspiration_expand */
    {
        int alpha = 0;
        int beta = 100;
        progressive_expand_window(&alpha, &beta);
        if (alpha == -PROGRESSIVE_ASPRATION_EXPANSION &&
            beta == 100 + PROGRESSIVE_ASPRATION_EXPANSION) {
            printf("  test_progressive_aspiration_expand... PASS\n"); passed++;
        } else {
            printf("  test_progressive_aspiration_expand... FAIL\n"); failed++;
        }
    }

    /* Test 9: progressive_aspiration_fail_high */
    {
        int alpha = 0;
        int beta = 100;
        progressive_expand_window(&alpha, &beta);
        progressive_expand_window(&alpha, &beta);
        if (beta > 100) {
            printf("  test_progressive_aspiration_fail_high... PASS\n"); passed++;
        } else {
            printf("  test_progressive_aspiration_fail_high... FAIL\n"); failed++;
        }
    }

    /* Test 10: progressive_aspiration_fail_low */
    {
        int alpha = 0;
        int beta = 100;
        progressive_expand_window(&alpha, &beta);
        progressive_expand_window(&alpha, &beta);
        if (alpha < 0) {
            printf("  test_progressive_aspiration_fail_low... PASS\n"); passed++;
        } else {
            printf("  test_progressive_aspiration_fail_low... FAIL\n"); failed++;
        }
    }

    /* Test 11: progressive_early_exit */
    {
        progressive_init(&ps, 10);
        ps.currentDepth = 5;
        ps.stopReason = STOP_TIME;
        int next = progressive_next_depth(&ps);
        if (next == 6) {
            printf("  test_progressive_early_exit... PASS\n"); passed++;
        } else {
            printf("  test_progressive_early_exit... FAIL\n"); failed++;
        }
    }

    printf("\nProgressive: %d/%d passed\n", passed, passed + failed);
    return failed;
}
