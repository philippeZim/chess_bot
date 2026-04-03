#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/search/search.h"
#include "../../src/types.h"
#include "../../src/board/board.h"
#include "../../src/move/movegen.h"

#define ASSERT_EQ(actual, expected, msg) do { \
    if ((actual) != (expected)) { \
        printf("FAIL: %s - Expected %d, got %d\n", msg, (expected), (int)(actual)); \
        failures++; \
    } else { \
        printf("PASS: %s\n", msg); \
        passes++; \
    } \
} while(0)

#define ASSERT_TRUE(cond, msg) do { \
    if (!(cond)) { \
        printf("FAIL: %s\n", msg); \
        failures++; \
    } else { \
        printf("PASS: %s\n", msg); \
        passes++; \
    } \
} while(0)

static int passes = 0;
static int failures = 0;

int test_qsearch_init_params(void) {
    printf("\n=== QSearch Init Params Tests ===\n");
    
    SearchContext ctx;
    search_init(&ctx);
    
    Board board;
    board_init(&board);
    
    int result = qsearch(&ctx, &board, -100, 100);
    
    ASSERT_TRUE(result >= -100 && result <= 100, "QSearch returns value within alpha-beta");
    
    return failures;
}

int test_qsearch_stands_pat(void) {
    printf("\n=== QSearch Stands Pat Tests ===\n");
    
    SearchContext ctx;
    search_init(&ctx);
    
    Board board;
    board_init(&board);
    
    int eval = evaluate_position(&ctx, &board);
    int result = qsearch(&ctx, &board, -10000, 10000);
    
    ASSERT_TRUE(result >= eval, "QSearch result at stands pat");
    
    return failures;
}

int test_qsearch_alpha_beta_cutoff(void) {
    printf("\n=== QSearch Alpha Beta Cutoff Tests ===\n");
    
    SearchContext ctx;
    search_init(&ctx);
    
    Board board;
    board_init(&board);
    
    int beta_result = qsearch(&ctx, &board, -10000, 100);
    (void)qsearch(&ctx, &board, -10000, 10000);
    
    ASSERT_TRUE(beta_result <= 100, "Beta cutoff respected");
    
    return failures;
}

int test_qsearch_respects_bounds(void) {
    printf("\n=== QSearch Respects Bounds Tests ===\n");
    
    SearchContext ctx;
    search_init(&ctx);
    
    Board board;
    board_init(&board);
    
    int result = qsearch(&ctx, &board, -500, 500);
    
    ASSERT_TRUE(result >= -500, "Result respects alpha");
    ASSERT_TRUE(result <= 500, "Result respects beta");
    
    return failures;
}

int test_qsearch_consistency(void) {
    printf("\n=== QSearch Consistency Tests ===\n");
    
    SearchContext ctx;
    search_init(&ctx);
    
    Board board;
    board_init(&board);
    
    int result1 = qsearch(&ctx, &board, -10000, 10000);
    int result2 = qsearch(&ctx, &board, -10000, 10000);
    
    ASSERT_EQ(result1, result2, "QSearch returns consistent results");
    
    return failures;
}

int test_qsearch_immediate_beta_cutoff(void) {
    printf("\n=== QSearch Immediate Beta Cutoff Tests ===\n");
    
    SearchContext ctx;
    search_init(&ctx);
    
    Board board;
    board_init(&board);
    
    int eval = evaluate_position(&ctx, &board);
    int result = qsearch(&ctx, &board, -100, eval + 100);
    
    ASSERT_TRUE(result <= eval + 100, "QSearch respects beta");
    
    return failures;
}

int run_qsearch_tests(void) {
    printf("Quiescence Search Tests\n");
    printf("=======================\n");
    
    test_qsearch_init_params();
    test_qsearch_stands_pat();
    test_qsearch_alpha_beta_cutoff();
    test_qsearch_respects_bounds();
    test_qsearch_consistency();
    test_qsearch_immediate_beta_cutoff();
    
    printf("\n=======================\n");
    printf("Passed: %d, Failed: %d\n", passes, failures);
    printf("=======================\n");
    
    return failures;
}
