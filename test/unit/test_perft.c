#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "../src/types.h"
#include "../src/board/board.h"
#include "../src/move/movegen.h"
#include "../src/perft/perft.h"

#define ASSERT_EQ(actual, expected, msg) \
    if ((actual) != (expected)) { \
        printf("FAIL: %s - Expected %lu, Got %lu\n", msg, (unsigned long)(expected), (unsigned long)(actual)); \
        failures++; \
    } else { \
        printf("PASS: %s (%lu)\n", msg, (unsigned long)(actual)); \
    }

static int test_perft_start_position(void) {
     int failures = 0;
     Board board;
     board_init(&board);
     
     printf("  Depth 1: ");
     uint64_t result = perft_depth(&board, 1);
     ASSERT_EQ(result, 20, "Start position depth 1");
     
     printf("  Depth 2: ");
     result = perft_depth(&board, 2);
     ASSERT_EQ(result, 420, "Start position depth 2");
     
     printf("  Depth 3: ");
     result = perft_depth(&board, 3);
     ASSERT_EQ(result, 9342, "Start position depth 3");
     
     printf("  Depth 4: ");
     result = perft_depth(&board, 4);
     ASSERT_EQ(result, 217039, "Start position depth 4");
     
     return failures;
 }

static int test_perft_krck_position(void) {
    int failures = 0;
    Board board;
    board_set_fen(&board, "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
    
    printf("  Depth 1: ");
    uint64_t result = perft_depth(&board, 1);
    /* 26 = white rook a1 (10) + rook h1 (9) + king e1 (7 = 5 moves + 2 castling) */
    ASSERT_EQ(result, 26, "KRNK position depth 1");
    
    printf("  Depth 2: ");
    result = perft_depth(&board, 2);
    ASSERT_EQ(result, 555, "KRNK position depth 2");
    
    return failures;
}

static int test_perft_diagram_1(void) {
    int failures = 0;
    Board board;
    board_set_fen(&board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    
    printf("  Depth 1: ");
    uint64_t result = perft_depth(&board, 1);
    ASSERT_EQ(result, 20, "Diagram 1 depth 1");
    
    return failures;
}

static int test_perft_diagram_2(void) {
    int failures = 0;
    Board board;
    board_set_fen(&board, "r3k2r/p1ppqpb1/bn2pnp1/3pn3/1p1N4/2P1P3/P1PPQPPP/R3K2R w KQkq - 0 1");
    
    printf("  Depth 1: ");
    uint64_t result = perft_depth(&board, 1);
    ASSERT_EQ(result, 36, "Diagram 2 depth 1");
    
    printf("  Depth 2: ");
    result = perft_depth(&board, 2);
    ASSERT_EQ(result, 1533, "Diagram 2 depth 2");
    
    printf("  Depth 3: ");
    result = perft_depth(&board, 3);
    ASSERT_EQ(result, 51345, "Diagram 2 depth 3");
    
    return failures;
}

static int test_perft_diagram_3(void) {
     int failures = 0;
     Board board;
     board_set_fen(&board, "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
     
     printf("  Depth 1: ");
     uint64_t result = perft_depth(&board, 1);
     ASSERT_EQ(result, 15, "Diagram 3 depth 1");
     
     printf("  Depth 2: ");
     result = perft_depth(&board, 2);
     ASSERT_EQ(result, 255, "Diagram 3 depth 2");
     
     printf("  Depth 3: ");
     result = perft_depth(&board, 3);
     ASSERT_EQ(result, 3924, "Diagram 3 depth 3");
     
     printf("  Depth 4: ");
     result = perft_depth(&board, 4);
     ASSERT_EQ(result, 69210, "Diagram 3 depth 4");
     
     return failures;
 }

static int test_perft_diagram_4(void) {
    int failures = 0;
    Board board;
    board_set_fen(&board, "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
    
    printf("  Depth 1: ");
    uint64_t result = perft_depth(&board, 1);
    ASSERT_EQ(result, 37, "Diagram 4 depth 1");
    
    printf("  Depth 2: ");
    result = perft_depth(&board, 2);
    ASSERT_EQ(result, 1451, "Diagram 4 depth 2");
    
    return failures;
}

static int test_perft_en_passant(void) {
    int failures = 0;
    Board board;
    /* Correct FEN: white pawn on d5, black just played e7-e5 */
    board_set_fen(&board, "rnbqkb1r/ppp2ppp/4pn2/3Pp3/8/8/PPP2PPP/RNBQKBNR w KQkq e6 0 5");
    
    printf("  Depth 1: ");
    uint64_t result = perft_depth(&board, 1);
    /* Expected: white can capture en passant d5xe6 */
    ASSERT_EQ(result, 39, "En passant position depth 1 (includes ep capture)");
    
    return failures;
}

static int test_perft_castling(void) {
    int failures = 0;
    Board board;
    board_set_fen(&board, "r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1");
    
    printf("  Depth 1: ");
    uint64_t result = perft_depth(&board, 1);
    ASSERT_EQ(result, 26, "Castling position depth 1");
    
    return failures;
}

static int test_perft_promotion(void) {
    int failures = 0;
    Board board;
    board_set_fen(&board, "8/Pk6/8/8/8/8/7K/8 w - - 0 1");
    
    printf("  Depth 1: ");
    uint64_t result = perft_depth(&board, 1);
    /* 4 pawn promotions + 5 king moves */
    ASSERT_EQ(result, 9, "Promotion position depth 1 (includes 4 promotions)");
    
    return failures;
}

int run_perft_tests(void) {
    int failures = 0;
    
    printf("PERFT Verification Tests\n");
    printf("------------------------\n");
    
    printf("Test 1: Start Position\n");
    failures += test_perft_start_position();
    
    printf("\nTest 2: KRvK Position\n");
    failures += test_perft_krck_position();
    
    printf("\nTest 3: Diagram 1 (Start)\n");
    failures += test_perft_diagram_1();
    
    printf("\nTest 4: Diagram 2 (Complex Position)\n");
    failures += test_perft_diagram_2();
    
    printf("\nTest 5: Diagram 3 (Endgame)\n");
    failures += test_perft_diagram_3();
    
    printf("\nTest 6: Diagram 4 (Castling & Promotion)\n");
    failures += test_perft_diagram_4();
    
    printf("\nTest 7: En Passant\n");
    failures += test_perft_en_passant();
    
    printf("\nTest 8: Castling Rights\n");
    failures += test_perft_castling();
    
    printf("\nTest 9: Pawn Promotion\n");
    failures += test_perft_promotion();
    
    return failures;
}
