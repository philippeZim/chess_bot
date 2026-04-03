#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "types.h"
#include "heuristics/history.h"
#include "move/move.h"

int run_history_tests(void) {
    int passed = 0, failed = 0;
    HistoryTable ht;
    HistoryTable ht2;
    
    /* Test 1: history_init */
    {
        history_init(&ht);
        int all_zero = 1;
        for (int c = 0; c < COLOR_NB && all_zero; c++) {
            for (int f = 0; f < SQUARE_NB && all_zero; f++) {
                for (int p = 0; p < PIECE_NB && all_zero; p++) {
                    for (int t = 0; t < SQUARE_NB && all_zero; t++) {
                        if (ht.history[c][f][p][t] != 0) all_zero = 0;
                        if (ht.continuation[c][f][p][t] != 0) all_zero = 0;
                    }
                }
            }
        }
        if (all_zero) {
            printf("  test_history_init... PASS\n"); passed++;
        } else {
            printf("  test_history_init... FAIL\n"); failed++;
        }
    }
    
    /* Test 2: history_get_default */
    {
        history_init(&ht);
        int16_t val = history_get(&ht, ColorWhite, SQUARE_A1, W_PAWN, SQUARE_A2);
        if (val == 0) {
            printf("  test_history_get_default... PASS\n"); passed++;
        } else {
            printf("  test_history_get_default... FAIL\n"); failed++;
        }
    }
    
    /* Test 3: history_update_single */
    {
        history_init(&ht);
        history_update(&ht, ColorWhite, SQUARE_A1, W_PAWN, SQUARE_A2, 256);
        int16_t val = history_get(&ht, ColorWhite, SQUARE_A1, W_PAWN, SQUARE_A2);
        if (val > 0) {
            printf("  test_history_update_single... PASS\n"); passed++;
        } else {
            printf("  test_history_update_single... FAIL\n"); failed++;
        }
    }
    
    /* Test 4: history_update_multiple */
    {
        history_init(&ht);
        int16_t val1 = history_get(&ht, ColorWhite, SQUARE_A1, W_PAWN, SQUARE_A2);
        history_update(&ht, ColorWhite, SQUARE_A1, W_PAWN, SQUARE_A2, 256);
        int16_t val2 = history_get(&ht, ColorWhite, SQUARE_A1, W_PAWN, SQUARE_A2);
        history_update(&ht, ColorWhite, SQUARE_A1, W_PAWN, SQUARE_A2, 256);
        int16_t val3 = history_get(&ht, ColorWhite, SQUARE_A1, W_PAWN, SQUARE_A2);
        if (val2 > val1 && val3 > val2) {
            printf("  test_history_update_multiple... PASS\n"); passed++;
        } else {
            printf("  test_history_update_multiple... FAIL\n"); failed++;
        }
    }
    
    /* Test 5: history_quadratic_bias */
    {
        history_init(&ht);
        history_update(&ht, ColorWhite, SQUARE_A1, W_PAWN, SQUARE_A2, 1000);
        int16_t val = history_get(&ht, ColorWhite, SQUARE_A1, W_PAWN, SQUARE_A2);
        if (val < 1000 && val > 0) {
            printf("  test_history_quadratic_bias... PASS\n"); passed++;
        } else {
            printf("  test_history_quadratic_bias... FAIL\n"); failed++;
        }
    }
    
    /* Test 6: history_clear */
    {
        history_init(&ht);
        history_update(&ht, ColorWhite, SQUARE_A1, W_PAWN, SQUARE_A2, 256);
        history_clear(&ht);
        int16_t val = history_get(&ht, ColorWhite, SQUARE_A1, W_PAWN, SQUARE_A2);
        if (val == 0) {
            printf("  test_history_clear... PASS\n"); passed++;
        } else {
            printf("  test_history_clear... FAIL\n"); failed++;
        }
    }
    
    /* Test 7: history_clear_depth */
    {
        history_init(&ht);
        history_update(&ht, ColorWhite, SQUARE_A1, W_PAWN, SQUARE_A2, 256);
        history_clear_depth(&ht, ColorWhite, 5);
        int16_t val = history_get(&ht, ColorWhite, SQUARE_A1, W_PAWN, SQUARE_A2);
        if (val == 0) {
            printf("  test_history_clear_depth... PASS\n"); passed++;
        } else {
            printf("  test_history_clear_depth... FAIL\n"); failed++;
        }
    }
    
    /* Test 8: history_best_move */
    {
        history_init(&ht);
        Move moves[3];
        moves[0] = make_move(SQUARE_A1, SQUARE_A2);
        moves[1] = make_move(SQUARE_B1, SQUARE_B2);
        moves[2] = make_move(SQUARE_C1, SQUARE_C2);
        
        history_update(&ht, ColorWhite, SQUARE_A1, W_PAWN, SQUARE_A2, 100);
        history_update(&ht, ColorWhite, SQUARE_B1, W_KNIGHT, SQUARE_B2, 300);
        history_update(&ht, ColorWhite, SQUARE_C1, W_BISHOP, SQUARE_C2, 200);
        
        Move best = history_best_move(&ht, ColorWhite, moves, 3);
        if (best == moves[1]) {
            printf("  test_history_best_move... PASS\n"); passed++;
        } else {
            printf("  test_history_best_move... FAIL\n"); failed++;
        }
    }
    
    /* Test 9: history_continuation */
    {
        history_init(&ht);
        history_update_continuation(&ht, ColorWhite, SQUARE_A1, W_PAWN, SQUARE_A2, 256);
        int16_t val = history_get_continuation(&ht, ColorWhite, SQUARE_A1, W_PAWN, SQUARE_A2);
        if (val > 0) {
            printf("  test_history_continuation... PASS\n"); passed++;
        } else {
            printf("  test_history_continuation... FAIL\n"); failed++;
        }
    }
    
    /* Test 10: history_edge_cases */
    {
        history_init(&ht);
        history_update(&ht, ColorWhite, SQUARE_H8, W_PAWN, SQUARE_H8, HISTORY_BONUS_MAX);
        history_update(&ht, ColorWhite, SQUARE_H8, W_PAWN, SQUARE_H8, HISTORY_BONUS_MAX);
        int16_t val = history_get(&ht, ColorWhite, SQUARE_H8, W_PAWN, SQUARE_H8);
        if (val <= HISTORY_BONUS_MAX) {
            printf("  test_history_edge_cases... PASS\n"); passed++;
        } else {
            printf("  test_history_edge_cases... FAIL\n"); failed++;
        }
    }
    
    printf("\nHistory: %d/%d passed\n", passed, passed + failed);
    return failed;
}
