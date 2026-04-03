#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "../src/types.h"
#include "../src/board/board.h"
#include "../src/transposition/zobrist.h"

#define ASSERT_EQ(actual, expected, msg) \
    if ((actual) != (expected)) { \
        printf("FAIL: %s - Expected 0x%llx, Got 0x%llx\n", msg, (unsigned long long)(expected), (unsigned long long)(actual)); \
        failures++; \
    } else { \
        printf("PASS: %s (0x%llx)\n", msg, (unsigned long long)(actual)); \
    }

#define ASSERT_TRUE(cond, msg) \
    if (!(cond)) { \
        printf("FAIL: %s\n", msg); \
        failures++; \
    } else { \
        printf("PASS: %s\n", msg); \
    }

static int test_zobrist_init(void) {
    int failures = 0;
    
    printf("Testing Zobrist initialization...\n");
    zobrist_init();
    ASSERT_TRUE(1, "Zobrist keys initialized without crash");
    
    return failures;
}

static int test_zobrist_empty_board(void) {
    int failures = 0;
    Board board;
    
    printf("Testing empty board hash...\n");
    memset(&board, 0, sizeof(Board));
    board.turn = ColorWhite;
    board.castling = CASTLING_NONE;
    board.enPassant = SQUARE_NB;
    
    zobrist_init();
    uint64_t hash = zobrist_hash_board(&board);
    
    ASSERT_TRUE(hash != 0, "Empty board should have non-zero hash (due to turn)");
    
    return failures;
}

static int test_zobrist_piece_placements(void) {
    int failures = 0;
    Board board1, board2;
    
    printf("Testing piece placement hashes...\n");
    zobrist_init();
    
    memset(&board1, 0, sizeof(Board));
    memset(&board2, 0, sizeof(Board));
    board1.turn = board2.turn = ColorWhite;
    board1.castling = board2.castling = CASTLING_NONE;
    board1.enPassant = board2.enPassant = SQUARE_NB;
    
    board1.squares[SQUARE_E4] = W_PAWN;
    board1.byType[PAWN] |= (1ULL << SQUARE_E4);
    board1.byColor[ColorWhite] |= (1ULL << SQUARE_E4);
    
    board2.squares[SQUARE_E5] = W_PAWN;
    board2.byType[PAWN] |= (1ULL << SQUARE_E5);
    board2.byColor[ColorWhite] |= (1ULL << SQUARE_E5);
    
    uint64_t hash1 = zobrist_hash_board(&board1);
    uint64_t hash2 = zobrist_hash_board(&board2);
    
    ASSERT_TRUE(hash1 != hash2, "Different piece positions should have different hashes");
    ASSERT_TRUE(hash1 != 0, "Board with pawn should have non-zero hash");
    ASSERT_TRUE(hash2 != 0, "Board with pawn should have non-zero hash");
    
    return failures;
}

static int test_zobrist_turn_hash(void) {
    int failures = 0;
    Board board;
    
    printf("Testing side-to-move hash...\n");
    zobrist_init();
    
    memset(&board, 0, sizeof(Board));
    board.turn = ColorWhite;
    board.castling = CASTLING_NONE;
    board.enPassant = SQUARE_NB;
    
    uint64_t hash_white = zobrist_hash_board(&board);
    
    board.turn = ColorBlack;
    uint64_t hash_black = zobrist_hash_board(&board);
    
    ASSERT_TRUE(hash_white != hash_black, "Different turn should produce different hash");
    
    return failures;
}

static int test_zobrist_castling_hash(void) {
    int failures = 0;
    Board board;
    
    printf("Testing castling rights hash...\n");
    zobrist_init();
    
    memset(&board, 0, sizeof(Board));
    board.turn = ColorWhite;
    board.enPassant = SQUARE_NB;
    
    board.castling = CASTLING_NONE;
    uint64_t hash0 = zobrist_hash_board(&board);
    
    board.castling = CASTLING_WK;
    uint64_t hash1 = zobrist_hash_board(&board);
    
    board.castling = CASTLING_WK | CASTLING_WQ;
    uint64_t hash2 = zobrist_hash_board(&board);
    
    board.castling = CASTLING_ALL;
    uint64_t hash3 = zobrist_hash_board(&board);
    
    ASSERT_TRUE(hash0 != hash1, "Different castling rights should produce different hash");
    ASSERT_TRUE(hash1 != hash2, "Different castling rights should produce different hash");
    ASSERT_TRUE(hash2 != hash3, "Different castling rights should produce different hash");
    ASSERT_TRUE(hash0 != hash3, "Different castling rights should produce different hash");
    
    return failures;
}

static int test_zobrist_en_passant_hash(void) {
    int failures = 0;
    Board board;
    
    printf("Testing en passant hash...\n");
    
    memset(&board, 0, sizeof(Board));
    board.turn = ColorWhite;
    board.castling = CASTLING_NONE;
    
    board.enPassant = SQUARE_NB;
    uint64_t hash0 = zobrist_hash_board(&board);
    
    board.enPassant = SQUARE_E3;
    uint64_t hash1 = zobrist_hash_board(&board);
    
    board.enPassant = SQUARE_F6;
    uint64_t hash2 = zobrist_hash_board(&board);
    
    ASSERT_TRUE(hash0 != hash1, "En passant set should differ from none");
    ASSERT_TRUE(hash1 != hash2, "Different en passant files should produce different hash");
    
    return failures;
}

static int test_zobrist_full_position(void) {
    int failures = 0;
    Board board;
    char fen[256];
    
    printf("Testing full FEN position hash...\n");
    zobrist_init();
    
    board_init(&board);
    uint64_t start_hash = zobrist_hash_board(&board);
    ASSERT_TRUE(start_hash != 0, "Starting position should have non-zero hash");
    
    if (board_get_fen(&board, fen, sizeof(fen))) {
        printf("  FEN: %s\n", fen);
    }
    
    board_set_fen(&board, "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1");
    uint64_t e4_hash = zobrist_hash_board(&board);
    ASSERT_TRUE(e4_hash != start_hash, "After 1.e4 hash should differ from start");
    
    board_set_fen(&board, "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2");
    uint64_t e4e5_hash = zobrist_hash_board(&board);
    ASSERT_TRUE(e4e5_hash != e4_hash, "After 1.e4 e5 hash should differ");
    
    return failures;
}

static int test_zobrist_incremental_update(void) {
    int failures = 0;
    Board board;
    
    printf("Testing incremental hash update...\n");
    zobrist_init();
    
    board_init(&board);
    uint64_t start_hash = board.zobrist;
    
    board_set_fen(&board, "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1");
    uint64_t after_e4 = zobrist_hash_board(&board);
    
    ASSERT_TRUE(start_hash != after_e4, "Hash should change after move");
    
    zobrist_update_incremental(&board, SQUARE_E2, SQUARE_E4, NO_PIECE);
    
    return failures;
}

static int test_zobrist_uniqueness(void) {
    int failures = 0;
    Board board;
    uint64_t hashes[10];
    
    printf("Testing hash uniqueness across positions...\n");
    zobrist_init();
    
    const char* fens[] = {
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1",
        "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2",
        "rnbqkbnr/pppp1ppp/4p3/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 3",
        "rnbqkbnr/1ppppp1p/p6p/8/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 3",
        "rnbqkbnr/pppp1ppp/8/4p3/4Pp2/8/PPPP2PP/RNBQKBNR w KQkq - 0 3",
        "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3",
        "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 0 3",
        "r1bqkbnr/pppp1ppp/2n5/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R b KQkq - 3 3",
        "r1bqkbnr/pppp1ppp/2n5/1B2p3/4P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 0 4"
    };
    
    for (int i = 0; i < 10; i++) {
        board_set_fen(&board, fens[i]);
        hashes[i] = zobrist_hash_board(&board);
    }
    
    int unique = 1;
    for (int i = 0; i < 10; i++) {
        for (int j = i + 1; j < 10; j++) {
            if (hashes[i] == hashes[j]) {
                printf("  WARNING: Hash collision detected at positions %d and %d\n", i, j);
                unique = 0;
            }
        }
    }
    
    ASSERT_TRUE(unique, "All 10 positions should have unique hashes");
    
    return failures;
}

int run_zobrist_tests(void) {
    int failures = 0;
    
    printf("Zobrist Hashing Tests\n");
    printf("----------------------\n\n");
    
    failures += test_zobrist_init();
    printf("\n");
    
    failures += test_zobrist_empty_board();
    printf("\n");
    
    failures += test_zobrist_piece_placements();
    printf("\n");
    
    failures += test_zobrist_turn_hash();
    printf("\n");
    
    failures += test_zobrist_castling_hash();
    printf("\n");
    
    failures += test_zobrist_en_passant_hash();
    printf("\n");
    
    failures += test_zobrist_full_position();
    printf("\n");
    
    failures += test_zobrist_incremental_update();
    printf("\n");
    
    failures += test_zobrist_uniqueness();
    printf("\n");
    
    return failures;
}
