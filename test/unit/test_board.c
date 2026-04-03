#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "../src/types.h"
#include "../src/board/board.h"
#include "../src/board/bitboard.h"

int run_board_tests(void) {
    int passed = 0, failed = 0;
    Board board;
    char fen[256];
    
    /* Test 1: board_init */
    {
        if (board_init(&board) && board_is_valid(&board) && board.turn == ColorWhite) {
            printf("  test_board_init... PASS\n"); passed++;
        } else {
            printf("  test_board_init... FAIL\n"); failed++;
        }
    }
    
    /* Test 2: board_set_fen_startpos */
    {
        if (board_set_fen(&board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1") &&
            board_is_valid(&board) && board.turn == ColorWhite) {
            printf("  test_board_set_fen_startpos... PASS\n"); passed++;
        } else {
            printf("  test_board_set_fen_startpos... FAIL\n"); failed++;
        }
    }
    
    /* Test 3: board_set_fen_black_to_move */
    {
        if (board_set_fen(&board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1") &&
            board.turn == ColorBlack) {
            printf("  test_board_set_fen_black_to_move... PASS\n"); passed++;
        } else {
            printf("  test_board_set_fen_black_to_move... FAIL\n"); failed++;
        }
    }
    
    /* Test 4: board_set_fen_no_castling */
    {
        if (board_set_fen(&board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w - - 0 1") &&
            board.castling == CASTLING_NONE) {
            printf("  test_board_set_fen_no_castling... PASS\n"); passed++;
        } else {
            printf("  test_board_set_fen_no_castling... FAIL\n"); failed++;
        }
    }
    
    /* Test 5: board_set_fen_with_en_passant */
    {
        if (board_set_fen(&board, "rnbqkbnr/ppppppp1/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1") &&
            board.enPassant == SQUARE_E3) {
            printf("  test_board_set_fen_with_en_passant... PASS\n"); passed++;
        } else {
            printf("  test_board_set_fen_with_en_passant... FAIL\n"); failed++;
        }
    }
    
    /* Test 6: board_get_fen */
    {
        board_init(&board);
        if (board_get_fen(&board, fen, sizeof(fen)) && strncmp(fen, "rnbqkbnr/pppppppp", 17) == 0) {
            printf("  test_board_get_fen... PASS\n"); passed++;
        } else {
            printf("  test_board_get_fen... FAIL\n"); failed++;
        }
    }
    
    /* Test 7: board_clone */
    {
        Board board2;
        if (board_init(&board)) {
            uint8_t castling = board.castling;
            board_clone(&board, &board2);
            if (castling == board2.castling && board.turn == board2.turn) {
                printf("  test_board_clone... PASS\n"); passed++;
            } else {
                printf("  test_board_clone... FAIL\n"); failed++;
            }
        } else {
            printf("  test_board_clone... FAIL\n"); failed++;
        }
    }
    
    /* Test 8: board_is_invalid */
    {
        memset(&board, 0, sizeof(Board));
        if (!board_is_valid(&board)) {
            printf("  test_board_is_invalid... PASS\n"); passed++;
        } else {
            printf("  test_board_is_invalid... FAIL\n"); failed++;
        }
    }
    
    /* Test 9: board_piece_counts */
    {
        if (board_init(&board) &&
            bitboard_popcount(board.byType[PAWN]) == 16 &&
            bitboard_popcount(board.byType[KNIGHT]) == 4 &&
            bitboard_popcount(board.byType[BISHOP]) == 4 &&
            bitboard_popcount(board.byType[ROOK]) == 4 &&
            bitboard_popcount(board.byType[QUEEN]) == 2 &&
            bitboard_popcount(board.byType[KING]) == 2) {
            printf("  test_board_piece_counts... PASS\n"); passed++;
        } else {
            printf("  test_board_piece_counts... FAIL\n"); failed++;
        }
    }
    
    /* Test 10: board_color_bitboards */
    {
        if (board_init(&board) &&
            bitboard_popcount(board.byColor[ColorWhite]) == 16 &&
            bitboard_popcount(board.byColor[ColorBlack]) == 16) {
            printf("  test_board_color_bitboards... PASS\n"); passed++;
        } else {
            printf("  test_board_color_bitboards... FAIL\n"); failed++;
        }
    }
    
    /* Test 11: board_is_check */
    {
        if (board_set_fen(&board, "rnbqkbnr/pppp1ppp/8/4p3/4P2P/8/PPPP1PP1/RNBQKBNR b KQkq - 0 1") &&
            !board_is_check(&board)) {
            printf("  test_board_is_check... PASS\n"); passed++;
        } else {
            printf("  test_board_is_check... FAIL\n"); failed++;
        }
    }
    
    printf("\nBoard: %d/%d passed\n", passed, passed + failed);
    return failed;
}
