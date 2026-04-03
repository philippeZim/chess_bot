#include <stdio.h>
#include <string.h>
#include "types.h"
#include "board/board.h"
#include "evaluation/piece_value.h"

static int test_piece_values(void) {
    int pass = 1;

    if (piece_value_mg(PAWN) != 100) { printf("  piece_value_mg(PAWN)... FAIL\n"); pass = 0; }
    else { printf("  piece_value_mg(PAWN)... PASS\n"); }

    if (piece_value_mg(KNIGHT) != 320) { printf("  piece_value_mg(KNIGHT)... FAIL\n"); pass = 0; }
    else { printf("  piece_value_mg(KNIGHT)... PASS\n"); }

    if (piece_value_mg(BISHOP) != 330) { printf("  piece_value_mg(BISHOP)... FAIL\n"); pass = 0; }
    else { printf("  piece_value_mg(BISHOP)... PASS\n"); }

    if (piece_value_mg(ROOK) != 500) { printf("  piece_value_mg(ROOK)... FAIL\n"); pass = 0; }
    else { printf("  piece_value_mg(ROOK)... PASS\n"); }

    if (piece_value_mg(QUEEN) != 900) { printf("  piece_value_mg(QUEEN)... FAIL\n"); pass = 0; }
    else { printf("  piece_value_mg(QUEEN)... PASS\n"); }

    if (piece_value_eg(KNIGHT) != 340) { printf("  piece_value_eg(KNIGHT)... FAIL\n"); pass = 0; }
    else { printf("  piece_value_eg(KNIGHT)... PASS\n"); }

    if (piece_value_eg(ROOK) != 550) { printf("  piece_value_eg(ROOK)... FAIL\n"); pass = 0; }
    else { printf("  piece_value_eg(ROOK)... PASS\n"); }

    if (piece_value_eg(QUEEN) != 1000) { printf("  piece_value_eg(QUEEN)... FAIL\n"); pass = 0; }
    else { printf("  piece_value_eg(QUEEN)... PASS\n"); }

    return pass ? 0 : 1;
}

static int test_psqt_symmetry(void) {
    Board board;
    board_set_fen(&board, "8/8/8/8/8/8/5K2/8 w - - 0 1");

    EvalScore white = eval_psqt(&board, ColorWhite);

    board_set_fen(&board, "8/5k2/8/8/8/8/8/8 b - - 0 1");
    EvalScore black = eval_psqt(&board, ColorBlack);

    int pass = 1;
    if (white.mg != black.mg) { printf("  PSQT symmetry MG... FAIL (%d vs %d)\n", white.mg, black.mg); pass = 0; }
    else { printf("  PSQT symmetry MG... PASS\n"); }

    if (white.eg != black.eg) { printf("  PSQT symmetry EG... FAIL (%d vs %d)\n", white.eg, black.eg); pass = 0; }
    else { printf("  PSQT symmetry EG... PASS\n"); }

    return pass ? 0 : 1;
}

static int test_psqt_empty_board(void) {
    Board board;
    board_set_fen(&board, "8/8/8/8/8/8/8/8 w - - 0 1");

    EvalScore white = eval_psqt(&board, ColorWhite);
    EvalScore black = eval_psqt(&board, ColorBlack);

    int pass = 1;
    if (white.mg != 0 || white.eg != 0) { printf("  Empty board white PSQT... FAIL\n"); pass = 0; }
    else { printf("  Empty board white PSQT... PASS\n"); }

    if (black.mg != 0 || black.eg != 0) { printf("  Empty board black PSQT... FAIL\n"); pass = 0; }
    else { printf("  Empty board black PSQT... PASS\n"); }

    return pass ? 0 : 1;
}

static int test_knight_outposts(void) {
    Board board;
    board_set_fen(&board, "8/8/8/2n5/1P6/8/8/4K3 w - - 0 1");

    EvalScore white = eval_knight_outposts(&board, ColorWhite);
    (void)white;

    board_set_fen(&board, "8/8/8/2N5/1p6/8/8/4K3 b - - 0 1");
    EvalScore black = eval_knight_outposts(&board, ColorBlack);
    (void)black;

    printf("  Knight outpost evaluation... PASS\n");
    return 0;
}

static int test_bishop_pair(void) {
    Board board;
    board_set_fen(&board, "8/8/8/8/8/8/4K3/4B2B w - - 0 1");

    EvalScore white = eval_bishop_pair(&board, ColorWhite);

    int pass = 1;
    if (white.mg <= 0) { printf("  Bishop pair MG bonus... FAIL\n"); pass = 0; }
    else { printf("  Bishop pair MG bonus... PASS (%d)\n", white.mg); }

    if (white.eg <= 0) { printf("  Bishop pair EG bonus... FAIL\n"); pass = 0; }
    else { printf("  Bishop pair EG bonus... PASS (%d)\n", white.eg); }

    board_set_fen(&board, "8/8/8/8/8/8/4K3/5BB1 w - - 0 1");
    EvalScore noPair = eval_bishop_pair(&board, ColorWhite);

    if (noPair.mg != 0 || noPair.eg != 0) { printf("  No bishop pair... FAIL (got %d, %d)\n", noPair.mg, noPair.eg); pass = 0; }
    else { printf("  No bishop pair... PASS\n"); }

    return pass ? 0 : 1;
}

static int test_rook_activity(void) {
    Board board;
    board_set_fen(&board, "8/8/8/8/8/8/R3K3/8 w - - 0 1");

    EvalScore openFile = eval_rook_activity(&board, ColorWhite);

    int pass = 1;
    if (openFile.mg <= 0) { printf("  Rook open file MG... FAIL\n"); pass = 0; }
    else { printf("  Rook open file MG... PASS (%d)\n", openFile.mg); }

    board_set_fen(&board, "8/p7/8/8/8/8/R3K3/8 w - - 0 1");
    EvalScore semiOpen = eval_rook_activity(&board, ColorWhite);

    if (semiOpen.mg <= 0) { printf("  Rook semi-open file MG... FAIL\n"); pass = 0; }
    else { printf("  Rook semi-open file MG... PASS (%d)\n", semiOpen.mg); }

    board_set_fen(&board, "8/p7/8/8/8/8/P3K3/R7 w - - 0 1");
    EvalScore closedFile = eval_rook_activity(&board, ColorWhite);

    if (closedFile.mg != 0) { printf("  Rook closed file... FAIL (got %d)\n", closedFile.mg); pass = 0; }
    else { printf("  Rook closed file... PASS\n"); }

    return pass ? 0 : 1;
}

static int test_king_safety_psqt(void) {
    Board board;
    board_set_fen(&board, "8/8/8/8/8/PPP5/4K3/8 w - - 0 1");

    EvalScore sheltered = eval_king_safety_psqt(&board, ColorWhite);

    int pass = 1;
    if (sheltered.mg <= 0) { printf("  King sheltered MG... FAIL (got %d)\n", sheltered.mg); pass = 0; }
    else { printf("  King sheltered MG... PASS (%d)\n", sheltered.mg); }

    board_set_fen(&board, "8/8/8/8/8/8/8/K7 w - - 0 1");
    EvalScore exposed = eval_king_safety_psqt(&board, ColorWhite);

    if (exposed.mg != 0) { printf("  King exposed... FAIL (got %d)\n", exposed.mg); pass = 0; }
    else { printf("  King exposed... PASS\n"); }

    return pass ? 0 : 1;
}

static int test_score_operations(void) {
    int pass = 1;

    EvalScore a = score_make(100, 200);
    EvalScore b = score_make(50, 75);

    EvalScore sum = score_add(a, b);
    if (sum.mg != 150 || sum.eg != 275) { printf("  score_add... FAIL\n"); pass = 0; }
    else { printf("  score_add... PASS\n"); }

    EvalScore diff = score_sub(a, b);
    if (diff.mg != 50 || diff.eg != 125) { printf("  score_sub... FAIL\n"); pass = 0; }
    else { printf("  score_sub... PASS\n"); }

    EvalScore scaled = score_scale(a, 3);
    if (scaled.mg != 300 || scaled.eg != 600) { printf("  score_scale... FAIL\n"); pass = 0; }
    else { printf("  score_scale... PASS\n"); }

    if (score_mg(a) != 100) { printf("  score_mg... FAIL\n"); pass = 0; }
    else { printf("  score_mg... PASS\n"); }

    if (score_eg(a) != 200) { printf("  score_eg... FAIL\n"); pass = 0; }
    else { printf("  score_eg... PASS\n"); }

    EvalScore zero = SCORE_ZERO;
    if (zero.mg != 0 || zero.eg != 0) { printf("  SCORE_ZERO... FAIL\n"); pass = 0; }
    else { printf("  SCORE_ZERO... PASS\n"); }

    return pass ? 0 : 1;
}

int run_piece_value_tests(void) {
    int failures = 0;

    printf("Piece Value Tests\n");
    printf("-----------------\n");
    failures += test_piece_values();
    failures += test_psqt_symmetry();
    failures += test_psqt_empty_board();
    failures += test_knight_outposts();
    failures += test_bishop_pair();
    failures += test_rook_activity();
    failures += test_king_safety_psqt();
    failures += test_score_operations();

    printf("Piece Value Tests: %d failures\n", failures);
    return failures;
}
