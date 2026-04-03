#include <stdio.h>
#include <stdint.h>

int run_bitboard_tests(void);
int run_board_tests(void);
int run_movegen_tests(void);
int run_zobrist_tests(void);
int run_perft_tests(void);
int run_search_tests(void);
int run_tt_tests(void);
int run_qsearch_tests(void);
int run_pv_tests(void);
int run_history_tests(void);
int run_killer_tests(void);
int run_futility_tests(void);
int run_lmr_tests(void);
int run_lmp_tests(void);
int run_null_move_tests(void);
int run_progressive_tests(void);
int run_time_mgmt_tests(void);
int run_piece_value_tests(void);
int run_pawn_table_tests(void);
int run_nnue_tests(void);
int run_eval_tests(void);

int main(int argc, char** argv) {
    (void)argc; (void)argv;
    printf("Chess Bot Test Suite\n");
    printf("====================\n\n");

    int total_failures = 0;

    printf("Bitboard Tests\n");
    printf("--------------\n");
    total_failures += run_bitboard_tests();
    printf("\n");

    printf("Board Tests\n");
    printf("-----------\n");
    total_failures += run_board_tests();
    printf("\n");

    printf("Move Generation Tests\n");
    printf("---------------------\n");
    total_failures += run_movegen_tests();
    printf("\n");

    printf("Zobrist Hashing Tests\n");
    printf("---------------------\n");
    total_failures += run_zobrist_tests();
    printf("\n");

    printf("PERFT Verification Tests\n");
    printf("------------------------\n");
    total_failures += run_perft_tests();
    printf("\n");

    printf("Search Module Tests\n");
    printf("-------------------\n");
    total_failures += run_search_tests();
    printf("\n");

    printf("Transposition Table Tests\n");
    printf("-------------------------\n");
    total_failures += run_tt_tests();
    printf("\n");

    printf("Quiescence Search Tests\n");
    printf("-----------------------\n");
    total_failures += run_qsearch_tests();
    printf("\n");

    printf("Principal Variation Tests\n");
    printf("-------------------------\n");
    total_failures += run_pv_tests();
    printf("\n");

    printf("History Heuristic Tests\n");
    printf("-----------------------\n");
    total_failures += run_history_tests();
    printf("\n");

    printf("Killer Moves Tests\n");
    printf("------------------\n");
    total_failures += run_killer_tests();
    printf("\n");

    printf("Futility Pruning Tests\n");
    printf("----------------------\n");
    total_failures += run_futility_tests();
    printf("\n");

    printf("Late Move Reduction Tests\n");
    printf("-------------------------\n");
    total_failures += run_lmr_tests();
    printf("\n");

    printf("Late Move Pruning Tests\n");
    printf("-----------------------\n");
    total_failures += run_lmp_tests();
    printf("\n");

    printf("Null Move Pruning Tests\n");
    printf("-----------------------\n");
    total_failures += run_null_move_tests();
    printf("\n");

    printf("Progressive Deepening Tests\n");
    printf("---------------------------\n");
    total_failures += run_progressive_tests();
    printf("\n");

    printf("Time Management Tests\n");
    printf("---------------------\n");
    total_failures += run_time_mgmt_tests();
    printf("\n");

    printf("Piece Value Tests\n");
    printf("-----------------\n");
    total_failures += run_piece_value_tests();
    printf("\n");

    printf("Pawn Structure Tests\n");
    printf("--------------------\n");
    total_failures += run_pawn_table_tests();
    printf("\n");

    printf("NNUE Tests\n");
    printf("----------\n");
    total_failures += run_nnue_tests();
    printf("\n");

    printf("Full Evaluation Tests\n");
    printf("---------------------\n");
    total_failures += run_eval_tests();
    printf("\n");

    printf("====================\n");
    printf("Total Failed: %d\n", total_failures);
    printf("====================\n");

    return total_failures > 0 ? 1 : 0;
}
