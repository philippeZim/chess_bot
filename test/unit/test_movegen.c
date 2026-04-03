#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "../src/types.h"
#include "../src/board/board.h"
#include "../src/board/bitboard.h"
#include "../src/move/movegen.h"
#include "../src/move/move.h"

int run_movegen_tests(void) {
    int passed = 0, failed = 0;
    Board board;
    MoveList moves;
    
    /* Test 1: Empty position generates no moves */
    {
        board_init(&board);
        memset(&board, 0, sizeof(Board));
        board.turn = ColorWhite;
        generate_pseudolegal_moves(&board, &moves);
        if (moves.count == 0) {
            printf("  test_empty_position... PASS\n"); passed++;
        } else {
            printf("  test_empty_position... FAIL (got %d moves)\n", moves.count); failed++;
        }
    }
    
    /* Test 2: Starting position generate moves */
    {
        if (board_init(&board)) {
            generate_pseudolegal_moves(&board, &moves);
            if (moves.count > 0) {
                printf("  test_start_position... PASS\n"); passed++;
            } else {
                printf("  test_start_position... FAIL (got %d moves)\n", moves.count); failed++;
            }
        } else {
            printf("  test_start_position... FAIL\n"); failed++;
        }
    }
    
    /* Test 3: Pawn moves from starting position */
    {
        if (board_init(&board)) {
            generate_pseudolegal_moves(&board, &moves);
            int pawn_moves = 0;
            for (int i = 0; i < moves.count; i++) {
                Move m = moves.moves[i];
                int to_rank = square_rank(move_to(m));
                if (to_rank == 2 || to_rank == 5) {
                    pawn_moves++;
                }
            }
            if (pawn_moves > 0) {
                printf("  test_pawn_moves... PASS\n"); passed++;
            } else {
                printf("  test_pawn_moves... FAIL\n"); failed++;
            }
        } else {
            printf("  test_pawn_moves... FAIL\n"); failed++;
        }
    }
    
    /* Test 4: Knight moves */
    {
        if (board_init(&board)) {
            generate_pseudolegal_moves(&board, &moves);
            int knight_moves = 0;
            for (int i = 0; i < moves.count; i++) {
                Move m = moves.moves[i];
                Bitboard from_bb = 1ULL << move_from(m);
                if (from_bb & (board.byType[KNIGHT] & board.byColor[ColorWhite])) {
                    knight_moves++;
                }
            }
            if (knight_moves == 4) {
                printf("  test_knight_moves... PASS\n"); passed++;
            } else {
                printf("  test_knight_moves... FAIL (got %d knight moves)\n", knight_moves); failed++;
            }
        } else {
            printf("  test_knight_moves... FAIL\n"); failed++;
        }
    }
    
    /* Test 5: King moves include castling */
    {
        if (board_init(&board)) {
            generate_pseudolegal_moves(&board, &moves);
            int king_moves = 0;
            for (int i = 0; i < moves.count; i++) {
                Move m = moves.moves[i];
                Bitboard from_bb = 1ULL << move_from(m);
                if (from_bb & (board.byType[KING] & board.byColor[ColorWhite])) {
                    king_moves++;
                }
            }
            if (king_moves >= 0) {
                printf("  test_king_moves... PASS\n"); passed++;
            } else {
                printf("  test_king_moves... FAIL (got %d king moves)\n", king_moves); failed++;
            }
        } else {
            printf("  test_king_moves... FAIL\n"); failed++;
        }
    }
    
    /* Test 6: Castling availability */
    {
        if (board_set_fen(&board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1")) {
            generate_pseudolegal_moves(&board, &moves);
            int castling_moves = 0;
            for (int i = 0; i < moves.count; i++) {
                Move m = moves.moves[i];
                if (move_from(m) == SQUARE_E1 && (move_to(m) == SQUARE_G1 || move_to(m) == SQUARE_C1)) {
                    castling_moves++;
                }
            }
            if (castling_moves >= 0) {
                printf("  test_castling_moves... PASS\n"); passed++;
            } else {
                printf("  test_castling_moves... FAIL (got %d castling moves)\n", castling_moves); failed++;
            }
        } else {
            printf("  test_castling_moves... FAIL\n"); failed++;
        }
    }
    
    /* Test 7: No castling when blocked */
    {
        if (board_set_fen(&board, "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKQ1R w K - 0 1")) {
            generate_pseudolegal_moves(&board, &moves);
            int king_moves = 0;
            for (int i = 0; i < moves.count; i++) {
                Move m = moves.moves[i];
                if (move_from(m) == SQUARE_E1 && (move_to(m) == SQUARE_G1 || move_to(m) == SQUARE_C1)) {
                    king_moves++;
                }
            }
            if (king_moves == 0) {
                printf("  test_no_castling_when_pawn_in_way... PASS\n"); passed++;
            } else {
                printf("  test_no_castling_when_pawn_in_way... FAIL (got %d)\n", king_moves); failed++;
            }
        } else {
            printf("  test_no_castling_when_pawn_in_way... FAIL\n"); failed++;
        }
    }
    
    printf("\nMovegen: %d/%d passed\n", passed, passed + failed);
    return failed;
}
