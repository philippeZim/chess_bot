#include <stdio.h>
#include <stdint.h>

int run_bitboard_tests(void);

int main(int argc, char** argv) {
    printf("Chess Bot Test Suite\n");
    printf("====================\n\n");
    
    printf("Bitboard Tests\n");
    printf("--------------\n");
    int failures = run_bitboard_tests();
    
    printf("\n====================\n");
    printf("Failed: %d\n", failures);
    printf("====================\n");
    
    return failures > 0 ? 1 : 0;
}
