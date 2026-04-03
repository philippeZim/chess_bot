#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/transposition/transposition.h"
#include "../../src/transposition/zobrist.h"
#include "../../src/types.h"
#include "../../src/board/board.h"
#include "../../src/move/movegen.h"

#define ASSERT_EQ(actual, expected, msg) do { \
    if ((actual) != (expected)) { \
        printf("FAIL: %s - Expected %ld, got %ld\n", msg, (long)(expected), (long)(actual)); \
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

int test_tt_init(void) {
    printf("\n=== TT Init Tests ===\n");
    
    TranspositionTable* tt = (TranspositionTable*)malloc(sizeof(TranspositionTable));
    if (!tt) {
        printf("FAIL: malloc failed\n");
        return failures;
    }
    tt_alloc(tt);
    
    ASSERT_EQ(tt->size, TT_SIZE, "TT size after init");
    ASSERT_EQ(tt->table[0].key, 0, "TT entry key after init");
    
    tt_free(tt);
    free(tt);
    
    return failures;
}

int test_tt_clear(void) {
    printf("\n=== TT Clear Tests ===\n");
    
    TranspositionTable* tt = (TranspositionTable*)malloc(sizeof(TranspositionTable));
    if (!tt) {
        printf("FAIL: malloc failed\n");
        return failures;
    }
    tt_alloc(tt);
    
    uint64_t test_key = 0xDEADBEEF;
    tt_store(tt, test_key, 3, 100, TT_FLAG_EXACT, make_move(0, 10));
    ASSERT_TRUE(tt_probe(tt, test_key) != NULL, "TT entry stored");
    
    memset(tt->table, 0, sizeof(TranspositionEntry) * tt->size);
    ASSERT_TRUE(tt_probe(tt, test_key) == NULL, "TT entry cleared");
    
    tt_free(tt);
    free(tt);
    
    return failures;
}

int test_tt_store_and_retrieve(void) {
    printf("\n=== TT Store and Retrieve Tests ===\n");
    
    TranspositionTable* tt = (TranspositionTable*)malloc(sizeof(TranspositionTable));
    if (!tt) {
        printf("FAIL: malloc failed\n");
        return failures;
    }
    tt_alloc(tt);
    
    uint64_t key = 0x123456789ABCDEF0ULL;
    uint16_t depth = 5;
    int16_t score = 150;
    uint16_t flag = TT_FLAG_EXACT;
    Move move = make_move(SQUARE_A1, SQUARE_A2);
    
    tt_store(tt, key, depth, score, flag, move);
    
    TranspositionEntry* entry = tt_probe(tt, key);
    
    ASSERT_TRUE(entry != NULL, "Entry found after store");
    ASSERT_EQ(entry->key, key, "Entry key matches");
    ASSERT_EQ(entry->depth, depth, "Entry depth matches");
    ASSERT_EQ(entry->score, score, "Entry score matches");
    ASSERT_EQ(entry->flag, flag, "Entry flag matches");
    ASSERT_EQ(entry->best_move, move, "Entry move matches");
    
    tt_free(tt);
    free(tt);
    
    return failures;
}

int test_tt_collision_handling(void) {
    printf("\n=== TT Collision Handling Tests ===\n");
    
    TranspositionTable* tt = (TranspositionTable*)malloc(sizeof(TranspositionTable));
    if (!tt) {
        printf("FAIL: malloc failed\n");
        return failures;
    }
    tt_alloc(tt);
    
    uint64_t key1 = 0x123456789ABCDEF0ULL;
    uint64_t key2 = 0xFEDCBA9876543210ULL;
    
    tt_store(tt, key1, 5, 100, TT_FLAG_EXACT, make_move(0, 10));
    tt_store(tt, key2, 3, 50, TT_FLAG_ALPHA, make_move(10, 20));
    
    TranspositionEntry* entry1 = tt_probe(tt, key1);
    TranspositionEntry* entry2 = tt_probe(tt, key2);
    
    ASSERT_TRUE(entry1 != NULL, "Entry 1 exists");
    ASSERT_TRUE(entry2 != NULL, "Entry 2 exists");
    ASSERT_EQ(entry1->score, 100, "Entry 1 score correct");
    ASSERT_EQ(entry2->score, 50, "Entry 2 score correct");
    
    tt_free(tt);
    free(tt);
    
    return failures;
}

int test_tt_overwrite_shallower_depth(void) {
    printf("\n=== TT Overwrite Shallow Depth Tests ===\n");
    
    TranspositionTable* tt = (TranspositionTable*)malloc(sizeof(TranspositionTable));
    if (!tt) {
        printf("FAIL: malloc failed\n");
        return failures;
    }
    tt_alloc(tt);
    
    uint64_t key = 0x123456789ABCDEF0ULL;
    
    tt_store(tt, key, 3, 100, TT_FLAG_EXACT, make_move(0, 10));
    tt_store(tt, key, 5, 200, TT_FLAG_BETA, make_move(20, 30));
    
    TranspositionEntry* entry = tt_probe(tt, key);
    
    ASSERT_EQ(entry->depth, 5, "Depth updated to deeper");
    ASSERT_EQ(entry->score, 200, "Score updated");
    ASSERT_EQ(entry->flag, TT_FLAG_BETA, "Flag updated");
    
    tt_free(tt);
    free(tt);
    
    return failures;
}

int test_tt_keep_deeper_entry(void) {
    printf("\n=== TT Keep Deeper Entry Tests ===\n");
    
    TranspositionTable* tt = (TranspositionTable*)malloc(sizeof(TranspositionTable));
    if (!tt) {
        printf("FAIL: malloc failed\n");
        return failures;
    }
    tt_alloc(tt);
    
    uint64_t key = 0x123456789ABCDEF0ULL;
    
    tt_store(tt, key, 5, 200, TT_FLAG_BETA, make_move(20, 30));
    tt_store(tt, key, 3, 100, TT_FLAG_EXACT, make_move(0, 10));
    
    TranspositionEntry* entry = tt_probe(tt, key);
    
    ASSERT_EQ(entry->depth, 5, "Deeper entry kept");
    ASSERT_EQ(entry->score, 200, "Deeper entry score kept");
    
    tt_free(tt);
    free(tt);
    
    return failures;
}

int test_tt_get_move(void) {
    printf("\n=== TT Get Move Tests ===\n");
    
    TranspositionTable* tt = (TranspositionTable*)malloc(sizeof(TranspositionTable));
    if (!tt) {
        printf("FAIL: malloc failed\n");
        return failures;
    }
    tt_alloc(tt);
    
    Move expected_move = make_move(SQUARE_E2, SQUARE_E4);
    uint64_t key = 0xAAAAAAAAAAAAAAAAULL;
    
    tt_store(tt, key, 5, 0, TT_FLAG_EXACT, expected_move);
    
    Move retrieved = tt_get_move(tt, key);
    
    ASSERT_EQ(retrieved, expected_move, "Move retrieved correctly");
    
    uint64_t non_existent_key = 0xBBBBBBBBBBBBBBBBULL;
    Move no_move = tt_get_move(tt, non_existent_key);
    ASSERT_EQ(no_move, 0, "No move for non-existent key");
    
    tt_free(tt);
    free(tt);
    
    return failures;
}

int test_tt_usage(void) {
    printf("\n=== TT Usage Tests ===\n");
    
    TranspositionTable* tt = (TranspositionTable*)malloc(sizeof(TranspositionTable));
    if (!tt) {
        printf("FAIL: malloc failed\n");
        return failures;
    }
    tt_alloc(tt);
    
    ASSERT_EQ(tt_get_usage(tt), 0, "TT usage 0 after init");
    
    tt_store(tt, 0x1111111111111111ULL, 3, 100, TT_FLAG_EXACT, 0);
    tt_store(tt, 0x2222222222222222ULL, 3, 100, TT_FLAG_EXACT, 0);
    tt_store(tt, 0x3333333333333333ULL, 3, 100, TT_FLAG_EXACT, 0);
    
    ASSERT_EQ(tt_get_usage(tt), 3, "TT usage 3 after storing 3 entries");
    
    tt_free(tt);
    free(tt);
    
    return failures;
}

int test_tt_board_hash_integration(void) {
    printf("\n=== TT Board Hash Integration Tests ===\n");
    
    TranspositionTable* tt = (TranspositionTable*)malloc(sizeof(TranspositionTable));
    if (!tt) {
        printf("FAIL: malloc failed\n");
        return failures;
    }
    tt_alloc(tt);
    
    Board board;
    memset(&board, 0, sizeof(Board));
    board.turn = ColorWhite;
    zobrist_init();
    uint64_t key = zobrist_hash_board(&board);
    
    tt_store(tt, key, 3, 0, TT_FLAG_EXACT, make_move(0, 10));
    
    TranspositionEntry* entry = tt_probe(tt, key);
    ASSERT_TRUE(entry != NULL, "Entry found with board hash");
    
    tt_free(tt);
    free(tt);
    
    return failures;
}

int run_tt_tests(void) {
    printf("Transposition Table Tests\n");
    printf("=========================\n");
    
    test_tt_init();
    test_tt_clear();
    test_tt_store_and_retrieve();
    test_tt_collision_handling();
    test_tt_overwrite_shallower_depth();
    test_tt_keep_deeper_entry();
    test_tt_get_move();
    test_tt_usage();
    test_tt_board_hash_integration();
    
    printf("\n=========================\n");
    printf("Passed: %d, Failed: %d\n", passes, failures);
    printf("=========================\n");
    
    return failures;
}
