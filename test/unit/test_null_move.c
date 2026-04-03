#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "types.h"
#include "pruning/null_move.h"
#include "board/board.h"

int run_null_move_tests(void) {
    int passed = 0, failed = 0;
    NullMoveConfig nm;
    Board board;

    /* Test 1: null_move_init */
    {
        null_move_init(&nm);
        if (nm.nullDepthReduction == NULL_DEPTH_REDUCTION &&
            nm.nullMinDepth == NULL_MIN_DEPTH &&
            nm.nullBetaMargin == NULL_BETA_MARGIN) {
            printf("  test_null_move_init... PASS\n"); passed++;
        } else {
            printf("  test_null_move_init... FAIL\n"); failed++;
        }
    }

    /* Test 2: null_move_can_prune_true */
    {
        null_move_init(&nm);
        if (null_move_can_prune(&nm, 5, 100, 200)) {
            printf("  test_null_move_can_prune_true... PASS\n"); passed++;
        } else {
            printf("  test_null_move_can_prune_true... FAIL\n"); failed++;
        }
    }

    /* Test 3: null_move_can_prune_false_depth */
    {
        null_move_init(&nm);
        if (!null_move_can_prune(&nm, 1, 100, 200)) {
            printf("  test_null_move_can_prune_false_depth... PASS\n"); passed++;
        } else {
            printf("  test_null_move_can_prune_false_depth... FAIL\n"); failed++;
        }
    }

    /* Test 4: null_move_can_prune_false_eval */
    {
        null_move_init(&nm);
        if (!null_move_can_prune(&nm, 5, 100, -200)) {
            printf("  test_null_move_can_prune_false_eval... PASS\n"); passed++;
        } else {
            printf("  test_null_move_can_prune_false_eval... FAIL\n"); failed++;
        }
    }

    /* Test 5: null_move_reduction_depth3 */
    {
        null_move_init(&nm);
        int r = null_move_reduction(&nm, 3);
        if (r >= 2) {
            printf("  test_null_move_reduction_depth3... PASS\n"); passed++;
        } else {
            printf("  test_null_move_reduction_depth3... FAIL\n"); failed++;
        }
    }

    /* Test 6: null_move_reduction_depth8 */
    {
        null_move_init(&nm);
        int r = null_move_reduction(&nm, 8);
        if (r >= 2 && r > null_move_reduction(&nm, 3)) {
            printf("  test_null_move_reduction_depth8... PASS\n"); passed++;
        } else {
            printf("  test_null_move_reduction_depth8... FAIL\n"); failed++;
        }
    }

    /* Test 7: null_move_verify_depth */
    {
        null_move_init(&nm);
        int v = null_move_verify(&nm, 6, 100);
        if (v >= 1 && v < 6) {
            printf("  test_null_move_verify_depth... PASS\n"); passed++;
        } else {
            printf("  test_null_move_verify_depth... FAIL\n"); failed++;
        }
    }

    /* Test 8: null_move_apply */
    {
        board_init(&board);
        board_set_fen(&board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        Color orig = board.turn;
        uint8_t origHalf = board.halfMoves;
        null_move_apply(&board);
        if (board.turn == (Color)(orig ^ 1) && board.halfMoves == (int)(origHalf + 1)) {
            printf("  test_null_move_apply... PASS\n"); passed++;
        } else {
            printf("  test_null_move_apply... FAIL\n"); failed++;
        }
    }

    /* Test 9: null_move_unapply */
    {
        board_init(&board);
        board_set_fen(&board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        Color orig = board.turn;
        uint8_t origHalf = board.halfMoves;
        null_move_apply(&board);
        null_move_unapply(&board);
        if (board.turn == orig && board.halfMoves == origHalf) {
            printf("  test_null_move_unapply... PASS\n"); passed++;
        } else {
            printf("  test_null_move_unapply... FAIL\n"); failed++;
        }
    }

    /* Test 10: null_move_zugzwang_check */
    {
        null_move_init(&nm);
        int depth = NULL_MIN_DEPTH;
        int beta = 50;
        int eval = -60;
        if (!null_move_can_prune(&nm, depth, beta, eval)) {
            printf("  test_null_move_zugzwang_check... PASS\n"); passed++;
        } else {
            printf("  test_null_move_zugzwang_check... FAIL\n"); failed++;
        }
    }

    printf("\nNull Move: %d/%d passed\n", passed, passed + failed);
    return failed;
}
