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
    
    printf("====================\n");
    printf("Total Failed: %d\n", total_failures);
    printf("====================\n");
    
    return total_failures > 0 ? 1 : 0;
}
