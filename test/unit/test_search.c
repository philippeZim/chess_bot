#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/search/search.h"
#include "../../src/transposition/transposition.h"
#include "../../src/types.h"
#include "../../src/board/board.h"

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

int test_search_init(void) {
    printf("\n=== Search Init Tests ===\n");
    
    SearchContext* ctx = (SearchContext*)malloc(sizeof(SearchContext));
    if (!ctx) {
        printf("FAIL: malloc failed\n");
        return failures;
    }
    search_init(ctx);
    
    ASSERT_EQ(ctx->tt.size, TT_SIZE, "TT size after init");
    ASSERT_EQ(ctx->nodes, 0, "Nodes count after init");
    ASSERT_EQ(ctx->depth, 0, "Depth after init");
    
    search_clear(ctx);
    ASSERT_EQ(ctx->nodes, 0, "Nodes count after clear");
    
    tt_free(&ctx->tt);
    free(ctx);
    
    return failures;
}

int test_search_negamax_basic(void) {
    printf("\n=== Search Negamax Basic Tests ===\n");
    
    SearchContext* ctx = (SearchContext*)malloc(sizeof(SearchContext));
    if (!ctx) {
        printf("FAIL: malloc failed\n");
        return failures;
    }
    search_init(ctx);
    
    Board board;
    board_init(&board);
    
    int score = search_negamax(ctx, &board, 1, -SCORE_INFINITE, SCORE_INFINITE);
    
    ASSERT_TRUE(score != 0 || board.turn == ColorBlack, "Negamax returns valid score");
    ASSERT_TRUE(ctx->nodes > 0, "Node count incremented");
    
    tt_free(&ctx->tt);
    free(ctx);
    
    return failures;
}

int test_search_iterative_deepening(void) {
    printf("\n=== Search Iterative Deepening Tests ===\n");
    
    SearchContext* ctx = (SearchContext*)malloc(sizeof(SearchContext));
    if (!ctx) {
        printf("FAIL: malloc failed\n");
        return failures;
    }
    search_init(ctx);
    
    Board board;
    board_init(&board);
    
    int best_move = 0;
    int score = search_iterative_deepening(ctx, &board, 3, &best_move);
    
    ASSERT_TRUE(score != SCORE_NONE, "Iterative deepening returns score");
    ASSERT_TRUE(ctx->depth > 0, "Depth set during search");
    
    tt_free(&ctx->tt);
    free(ctx);
    
    return failures;
}

int test_search_empty_board(void) {
    printf("\n=== Search Empty Board Tests ===\n");
    
    SearchContext* ctx = (SearchContext*)malloc(sizeof(SearchContext));
    if (!ctx) {
        printf("FAIL: malloc failed\n");
        return failures;
    }
    search_init(ctx);
    
    Board board;
    board_init(&board);
    
    for (int i = 0; i < BOARD_SIZE; i++) {
        board.squares[i] = NO_PIECE;
    }
    for (int i = 0; i < PIECE_TYPE_NB; i++) {
        board.byType[i] = 0;
    }
    for (int i = 0; i < ColorNB; i++) {
        board.byColor[i] = 0;
    }
    board.turn = ColorWhite;
    
    int score = search_negamax(ctx, &board, 1, -SCORE_INFINITE, SCORE_INFINITE);
    
    ASSERT_EQ(score, SCORE_DRAW, "Empty board returns draw score");
    
    tt_free(&ctx->tt);
    free(ctx);
    
    return failures;
}

int test_search_material_evaluation(void) {
    printf("\n=== Search Material Evaluation Tests ===\n");
    
    SearchContext* ctx = (SearchContext*)malloc(sizeof(SearchContext));
    if (!ctx) {
        printf("FAIL: malloc failed\n");
        return failures;
    }
    search_init(ctx);
    
    Board board;
    board_init(&board);
    
    int eval = evaluate_position(ctx, &board);
    
    ASSERT_EQ(eval, 0, "Starting position evaluates to 0");
    
    Board white_up;
    memset(&white_up, 0, sizeof(Board));
    white_up.turn = ColorWhite;
    white_up.squares[SQUARE_A3] = make_piece(ColorWhite, PAWN);
    white_up.byType[PAWN] |= (1ULL << SQUARE_A3);
    white_up.byColor[ColorWhite] |= (1ULL << SQUARE_A3);
    
    eval = evaluate_position(ctx, &white_up);
    ASSERT_TRUE(eval > 0, "Extra pawn favors white");
    
    Board black_up;
    memset(&black_up, 0, sizeof(Board));
    black_up.turn = ColorBlack;
    black_up.squares[SQUARE_A6] = make_piece(ColorBlack, PAWN);
    black_up.byType[PAWN] |= (1ULL << SQUARE_A6);
    black_up.byColor[ColorBlack] |= (1ULL << SQUARE_A6);
    
    eval = evaluate_position(ctx, &black_up);
    ASSERT_TRUE(eval < 0, "Extra pawn favors black");
    
    tt_free(&ctx->tt);
    free(ctx);
    
    return failures;
}

int test_search_respects_beta_cutoff(void) {
    printf("\n=== Search Beta Cutoff Tests ===\n");
    
    SearchContext* ctx = (SearchContext*)malloc(sizeof(SearchContext));
    if (!ctx) {
        printf("FAIL: malloc failed\n");
        return failures;
    }
    search_init(ctx);
    
    Board board;
    board_init(&board);
    
    int score1 = search_negamax(ctx, &board, 2, -SCORE_INFINITE, SCORE_INFINITE);
    int score2 = search_negamax(ctx, &board, 2, -500, 500);
    
    ASSERT_TRUE(score1 >= -500 && score1 <= 500, "Score within bounds");
    ASSERT_TRUE(score2 >= -500 && score2 <= 500, "Beta cutoff respected");
    
    tt_free(&ctx->tt);
    free(ctx);
    
    return failures;
}

int run_search_tests(void) {
    printf("Search Module Tests\n");
    printf("===================\n");
    
    test_search_init();
    test_search_negamax_basic();
    test_search_iterative_deepening();
    test_search_empty_board();
    test_search_material_evaluation();
    test_search_respects_beta_cutoff();
    
    printf("\n===================\n");
    printf("Passed: %d, Failed: %d\n", passes, failures);
    printf("===================\n");
    
    return failures;
}
