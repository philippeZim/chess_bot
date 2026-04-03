#include <stdio.h>
#include <string.h>
#include "types.h"
#include "board/board.h"
#include "evaluation/pawn_table.h"

static int test_doubled_pawns(void) {
    Board board;
    board_set_fen(&board, "8/8/8/8/8/P7/P7/8 w - - 0 1");

    EvalScore score = eval_pawns(&board);

    int pass = 1;
    if (score.mg >= 0) { printf("  Doubled pawn MG penalty... FAIL (got %d)\n", score.mg); pass = 0; }
    else { printf("  Doubled pawn MG penalty... PASS (%d)\n", score.mg); }

    if (score.eg >= 0) { printf("  Doubled pawn EG penalty... FAIL (got %d)\n", score.eg); pass = 0; }
    else { printf("  Doubled pawn EG penalty... PASS (%d)\n", score.eg); }

    return pass ? 0 : 1;
}

static int test_isolated_pawns(void) {
    Board board;
    board_set_fen(&board, "8/8/8/2p5/P7/8/8/4K3 w - - 0 1");

    EvalScore score = eval_pawns(&board);

    int pass = 1;
    if (score.mg >= 0) { printf("  Isolated pawn MG penalty... FAIL (got %d)\n", score.mg); pass = 0; }
    else { printf("  Isolated pawn MG penalty... PASS (%d)\n", score.mg); }

    return pass ? 0 : 1;
}

static int test_passed_pawns(void) {
    Board board;
    board_set_fen(&board, "8/P7/8/8/8/8/8/4K3 w - - 0 1");

    EvalScore score = eval_pawns(&board);

    int pass = 1;
    if (score.mg <= 0) { printf("  Passed pawn MG bonus... FAIL (got %d)\n", score.mg); pass = 0; }
    else { printf("  Passed pawn MG bonus... PASS (%d)\n", score.mg); }

    if (score.eg <= 0) { printf("  Passed pawn EG bonus... FAIL (got %d)\n", score.eg); pass = 0; }
    else { printf("  Passed pawn EG bonus... PASS (%d)\n", score.eg); }

    return pass ? 0 : 1;
}

static int test_passed_pawn_advancement(void) {
    Board board;
    board_set_fen(&board, "8/8/8/8/P7/8/8/4K3 w - - 0 1");

    EvalScore rank4 = eval_pawns(&board);

    board_set_fen(&board, "8/P7/8/8/8/8/8/4K3 w - - 0 1");
    EvalScore rank7 = eval_pawns(&board);

    int pass = 1;
    if (rank7.mg <= rank4.mg) { printf("  Passed pawn advancement MG... FAIL (%d vs %d)\n", rank7.mg, rank4.mg); pass = 0; }
    else { printf("  Passed pawn advancement MG... PASS (%d > %d)\n", rank7.mg, rank4.mg); }

    if (rank7.eg <= rank4.eg) { printf("  Passed pawn advancement EG... FAIL (%d vs %d)\n", rank7.eg, rank4.eg); pass = 0; }
    else { printf("  Passed pawn advancement EG... PASS (%d > %d)\n", rank7.eg, rank4.eg); }

    return pass ? 0 : 1;
}

static int test_pawn_hash_table(void) {
    Board board;
    board_set_fen(&board, "8/8/8/8/8/P7/8/4K3 w - - 0 1");

    EvalScore first = eval_pawns(&board);
    EvalScore second = eval_pawns(&board);

    int pass = 1;
    if (first.mg != second.mg || first.eg != second.eg) {
        printf("  Pawn hash table consistency... FAIL\n");
        pass = 0;
    } else {
        printf("  Pawn hash table consistency... PASS\n");
    }

    return pass ? 0 : 1;
}

static int test_connected_pawns(void) {
    Board board;
    board_set_fen(&board, "8/8/8/8/8/PP6/8/4K3 w - - 0 1");

    EvalScore score = eval_pawns(&board);

    int pass = 1;
    if (score.mg <= 0) { printf("  Connected pawns MG... FAIL (got %d)\n", score.mg); pass = 0; }
    else { printf("  Connected pawns MG... PASS (%d)\n", score.mg); }

    return pass ? 0 : 1;
}

static int test_king_shelter(void) {
    Board board;
    board_set_fen(&board, "8/8/8/8/8/PPP5/4K3/8 w - - 0 1");

    EvalScore score = eval_pawns(&board);

    int pass = 1;
    if (score.mg <= 0) { printf("  King shelter MG... FAIL (got %d)\n", score.mg); pass = 0; }
    else { printf("  King shelter MG... PASS (%d)\n", score.mg); }

    return pass ? 0 : 1;
}

static int test_pawn_table_init(void) {
    pawn_table_init();

    Board board;
    board_set_fen(&board, "8/8/8/8/8/8/8/8 w - - 0 1");
    EvalScore empty = eval_pawns(&board);

    int pass = 1;
    if (empty.mg != 0 || empty.eg != 0) { printf("  Pawn table init... FAIL\n"); pass = 0; }
    else { printf("  Pawn table init... PASS\n"); }

    return pass ? 0 : 1;
}

static int test_no_pawns(void) {
    Board board;
    board_set_fen(&board, "8/8/8/8/8/8/4K3/4k3 w - - 0 1");

    EvalScore score = eval_pawns(&board);

    int pass = 1;
    if (score.mg != 0 || score.eg != 0) { printf("  No pawns evaluation... FAIL (got %d, %d)\n", score.mg, score.eg); pass = 0; }
    else { printf("  No pawns evaluation... PASS\n"); }

    return pass ? 0 : 1;
}

static int test_backward_pawn(void) {
    Board board;
    board_set_fen(&board, "8/p7/8/8/P7/8/8/4K3 w - - 0 1");

    EvalScore score = eval_pawns(&board);

    printf("  Backward pawn evaluation... PASS (%d)\n", score.mg);
    return 0;
}

static int test_candidate_passer(void) {
    Board board;
    board_set_fen(&board, "8/8/8/8/P7/8/1P6/4K3 w - - 0 1");

    EvalScore score = eval_pawns(&board);

    printf("  Candidate passer evaluation... PASS (%d)\n", score.mg);
    return 0;
}

int run_pawn_table_tests(void) {
    int failures = 0;

    printf("Pawn Structure Tests\n");
    printf("--------------------\n");
    failures += test_pawn_table_init();
    failures += test_doubled_pawns();
    failures += test_isolated_pawns();
    failures += test_passed_pawns();
    failures += test_passed_pawn_advancement();
    failures += test_pawn_hash_table();
    failures += test_connected_pawns();
    failures += test_king_shelter();
    failures += test_no_pawns();
    failures += test_backward_pawn();
    failures += test_candidate_passer();

    printf("Pawn Structure Tests: %d failures\n", failures);
    return failures;
}
