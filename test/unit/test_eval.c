#include <stdio.h>
#include "types.h"
#include "board/board.h"

int evaluate(const Board* board);

static int test_starting_position(void) {
    Board board;
    board_set_fen(&board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    int score = evaluate(&board);

    int pass = 1;
    if (score < -50 || score > 50) { printf("  Starting position score... FAIL (got %d)\n", score); pass = 0; }
    else { printf("  Starting position score... PASS (%d)\n", score); }

    return pass ? 0 : 1;
}

static int test_material_advantage(void) {
    Board board;
    board_set_fen(&board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKB1R w KQkq - 0 1");

    int score = evaluate(&board);

    int pass = 1;
    if (score >= 0) { printf("  White missing knight... FAIL (got %d)\n", score); pass = 0; }
    else { printf("  White missing knight... PASS (%d)\n", score); }

    return pass ? 0 : 1;
}

static int test_endgame_position(void) {
    Board board;
    board_set_fen(&board, "4k3/4P3/4K3/8/8/8/8/8 w - - 0 1");

    int score = evaluate(&board);

    int pass = 1;
    if (score <= 0) { printf("  White winning endgame... FAIL (got %d)\n", score); pass = 0; }
    else { printf("  White winning endgame... PASS (%d)\n", score); }

    return pass ? 0 : 1;
}

static int test_symmetric_position(void) {
    Board board;
    board_set_fen(&board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    int whiteScore = evaluate(&board);

    board_set_fen(&board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1");
    int blackScore = evaluate(&board);

    int pass = 1;
    if (whiteScore != blackScore) {
        printf("  Symmetric position... FAIL (w=%d, b=%d)\n", whiteScore, blackScore);
        pass = 0;
    } else {
        printf("  Symmetric position... PASS (%d, %d)\n", whiteScore, blackScore);
    }

    return pass ? 0 : 1;
}

static int test_null_board(void) {
    int pass = 1;

    if (evaluate(NULL) != 0) { printf("  Null board evaluate... FAIL\n"); pass = 0; }
    else { printf("  Null board evaluate... PASS\n"); }

    return pass ? 0 : 1;
}

int run_eval_tests(void) {
    int failures = 0;

    printf("Full Evaluation Tests\n");
    printf("---------------------\n");
    failures += test_starting_position();
    failures += test_material_advantage();
    failures += test_endgame_position();
    failures += test_symmetric_position();
    failures += test_null_board();

    printf("Full Evaluation Tests: %d failures\n", failures);
    return failures;
}
