# Phase 3: Advanced Search - Implementation Plan

## Overview
Phases 1-2 Complete. Phase 3 adds Stockfish-level search optimizations to achieve 2500+ ELO.

---

## Component 1: History Heuristic (test/unit/test_history.c)

### Implementation: src/heuristics/history.c/history.h

**Structure:**
```c
typedef struct {
    int16_t history[COLOR_NB][64][PIECE_NB][64];  // [color][from][piece][to]
    int16_t continuation[COLOR_NB][64][PIECE_NB][64];  // continuation bonuses
} HistoryTable;
```

**Functions:**
1. `history_init(HistoryTable* ht)` - Initialize tables to zero
2. `history_get(HistoryTable* ht, Color color, Square from, Piece piece, Square to)` - Get value
3. `history_update(HistoryTable* ht, Color color, Square from, Piece piece, Square to, int bonus)` - Update with quadratic bias
4. `history_clear(HistoryTable* ht)` - Clear entire table
5. `history_clear_depth(HistoryTable* ht, Color color, int depth)` - Clear moves at depth
6. `history_bestMove(HistoryTable* ht, Color color, Move* moves, int count)` - Find best history move
7. `history_reduction(int historyValue)` - Convert history value to move ordering bonus

**Test Cases:**
1. test_history_init - Verify zero initialization
2. test_history_get_default - Verify default values are zero
3. test_history_update_single - Verify single update correctness
4. test_history_update_multiple - Verify cumulative updates
5. test_history_quadratic_bias - Test historyUpdate quadratic bonus formula
6. test_history_clear - Verify complete clear
7. test_history_clear_depth - Verify partial depth clear
8. test_history_best_move - Test move selection ordering
9. test_history_continuation - Test continuation history
10. test_history_edge_cases - Test boundary conditions

---

## Component 2: Killer Moves (test/unit/test_killer.c)

### Implementation: src/heuristics/killer.c/killer.h

**Structure:**
```c
typedef struct {
    Move killerMoves[MAX_DEPTH][2];  // 2 killer moves per depth
    int killerCounts[MAX_DEPTH];      // Count of killer moves at depth
} KillerTable;
```

**Functions:**
1. `killer_init(KillerTable* kt)` - Initialize tables
2. `killer_add(KillerTable* kt, int depth, Move move)` - Add killer move at depth
3. `killer_get(KillerTable* kt, int depth, Move* moves)` - Get killer moves for depth
4. `killer_clear(KillerTable* kt)` - Clear entirely
5. `killer_clear_depth(KillerTable* kt, int depth)` - Clear specific depth
6. `killer_is_killer(KillerTable* kt, int depth, Move move)` - Check if move is killer
7. `killer_order_moves(KillerTable* kt, int depth, Move* moves, int count)` - Prioritize killers

**Test Cases:**
1. test_killer_init - Verify initialization
2. test_killer_add_single - Add single killer move
3. test_killer_add_two - Add two killer moves at same depth
4. test_killer_add_overflow - Third move should not overwrite first two
5. test_killer_get_depth - Retrieve killer moves
6. test_killer_is_killer_true - Verify is_killer returns true
7. test_killer_is_killer_false - Verify is_killer returns false
8. test_killer_clear - Full clear test
9. test_killer_clear_depth - Depth-specific clear
10. test_killer_order_moves - Test move ordering priority
11. test_killer_cross_depth - Verify depths don't interfere

---

## Component 3: Futility Pruning (test/unit/test_futility.c)

### Implementation: src/pruning/futility.c/futility.h

**Structure:**
```c
typedef struct {
    int baseMargin;          // Base margin (typically 125-150)
    int depthMargin;         // Additional margin per depth
} FutilityMargins;
```

**Functions:**
1. `futility_init(FutilityMargins* fm)` - Initialize margins
2. `futility_can_prune(FutilityMargins* fm, int depth, int beta, int eval)` - Check futility
3. `futility_margin(FutilityMargins* fm, int depth)` - Calculate margin for depth
4. `futility_prune_score(int eval, int margin)` - Calculate prune score

**Test Cases:**
1. test_futility_init - Verify margin defaults
2. test_futility_margin_depth1 - Calculate depth 1 margin
3. test_futility_margin_depth5 - Calculate depth 5 margin
4. test_futility_can_prune_true - Test can prune condition
5. test_futility_can_prune_false - Test cannot prune condition
6. test_futility_prune_score - Verify prune score calc
7. test_futility_depth_zero - Test at depth 0 (no pruning)
8. test_futility_edge_case_beta - Edge case beta value
9. test_futility_edge_case_eval - Edge case eval value

---

## Component 4: Late Move Reduction (test/unit/test_lmr.c)

### Implementation: src/pruning/lmr.c/lmr.h

**Structure:**
```c
typedef struct {
    int reductionTable[64][64];  // [depth][moveNum]
} LmrTable;
```

**Functions:**
1. `lmr_init(LmrTable* lmr)` - Precompute reduction table
2. `lmr_reduction(LmrTable* lmr, int depth, int moveNum, bool isCapture, int history)` - Calculate reduction
3. `lmr_base_reduction(int depth, int moveNum)` - Base reduction formula
4. `lmr_adjust_reduction(int reduction, int history, bool isCapture)` - Adjust based on history
5. `lmr_min_depth(int depth, int reduction)` - Ensure minimum depth of 1

**Test Cases:**
1. test_lmr_init - Verify table computation
2. test_lmr_reduction_depth1 - Depth 1 reductions
3. test_lmr_reduction_depth10 - Depth 10 reductions
4. test_lmr_reduction_first_move - First move should have less reduction
5. test_lmr_reduction_last_move - Last move should have more reduction
6. test_lmr_capture_no_reduce - Captures should not reduce
7. test_lmr_history_bonus - History bonus reduces reduction
8. test_lmr_history_penalty - History penalty increases reduction
9. test_lmr_min_depth - Verify minimum depth
10. test_lmr_edge_cases - Boundary conditions

---

## Component 5: Late Move Pruning (test/unit/test_lmp.c)

### Implementation: src/pruning/lmp.c/lmp.h

**Structure:**
```c
typedef struct {
    int baseLmpMoves;      // Base moves to search at depth 0
} LmpConfig;

typedef struct {
    int lmpTable[MAX_DEPTH];  // Max moves per depth
} LmpTable;
```

**Functions:**
1. `lmp_init(LmpConfig* cfg, LmpTable* lmp)` - Initialize LMP tables
2. `lmp_max_moves(LmpTable* lmp, int depth)` - Get max moves for depth
3. `lmp_should_prune(LmpTable* lmp, int depth, int moveCount)` - Check LMP condition

**Test Cases:**
1. test_lmp_init - Verify initialization
2. test_lmp_max_moves_depth0 - Depth 0 max moves
3. test_lmp_max_moves_depth8 - Depth 8 max moves
4. test_lmp_should_prune_false - Should not prune early moves
5. test_lmp_should_prune_true - Should prune after threshold
6. test_lmp_quiet_only - LMP only applies to quiet moves
7. test_lmp_edge_cases - Deep depth edge cases

---

## Component 6: Null Move Pruning (test/unit/test_null_move.c)

### Implementation: src/pruning/null_move.c/null_move.h

**Structure:**
```c
typedef struct {
    int nullDepthReduction;  // Default R = 2-3
    int nullMinDepth;        // Don't prune below this depth
    int nullBetaMargin;      // Safety margin
} NullMoveConfig;
```

**Functions:**
1. `null_move_init(NullMoveConfig* nm)` - Initialize config
2. `null_move_can_prune(NullMoveConfig* nm, int depth, int beta, int eval)` - Check NMP
3. `null_move_reduction(NullMoveConfig* nm, int depth)` - Calculate R
4. `null_move_verify(NullMoveConfig* nm, int depth, int beta)` - Verification search depth
5. `null_move_apply(Board* board)` - Make null move
6. `null_move_unapply(Board* board)` - Unmake null move

**Test Cases:**
1. test_null_move_init - Verify defaults
2. test_null_move_can_prune_true - Can prune conditions
3. test_null_move_can_prune_false - Cannot prune (deep check, low eval)
4. test_null_move_reduction_depth3 - Calculate R at depth 3
5. test_null_move_reduction_depth8 - Calculate R at depth 8
6. test_null_move_verify_depth - Verify search depth calculation
7. test_null_move_apply - Test making null move
8. test_null_move_unapply - Test unmaking null move
9. test_null_move zugzwang_check - Zugzwang detection
10. test_null_move_insufficient_material - Insufficient material handling

---

## Component 7: Progressive Deepening (test/unit/test_progressive.c)

### Implementation: src/search/progressive.c/progressive.h

**Structure:**
```c
typedef struct {
    Move bestMove;
    int bestScore;
    int currentDepth;
    int maxDepth;
    StopReason stopReason;
} ProgressiveSearch;
```

**Functions:**
1. `progressive_init(ProgressiveSearch* ps, int maxDepth)` - Initialize
2. `progressive_search(ProgressiveSearch* ps, Board* board, TimeInfo* time)` - Run progressive search
3. `progressive_next_depth(ProgressiveSearch* ps)` - Get next depth
4. `progressive_should_stop(ProgressiveSearch* ps, TimeInfo* time)` - Check stop conditions
5. `progressive_aspiration_window(int score, int alpha, int beta)` - Calculate aspiration window
6. `progressive_expand_window(int* alpha, int* beta)` - Expand window on failure

**Test Cases:**
1. test_progressive_init - Verify initialization
2. test_progressive_search_depth1 - Single depth search
3. test_progressive_search_complete - Full progressive search
4. test_progressive_next_depth - Next depth calculation
5. test_progressive_should_stop_time - Stop on time
6. test_progressive_should_stop_moves - Stop on move count
7. test_progressive_aspiration_window - Window calculation
8. test_progressive_aspiration_expand - Window expansion
9. test_progressive_aspiration_fail_high - Fail high handling
10. test_progressive_aspiration_fail_low - Fail low handling
11. test_progressive_early_exit - Early exit conditions

---

## Component 8: Time Management (test/unit/test_time_mgmt.c)

### Implementation: src/time/time_mgmt.c/time_mgmt.h

**Structure:**
```c
typedef struct {
    uint64_t startTime;
    uint64_t optimalTime;   // Time for optimal move
    uint64_t maxTime;       // Absolute maximum time
    uint64_t moveTime;      // Hard deadline (if set)
    int increments;         // Increment per move
    int movesToGo;          // Moves remaining in time control
} TimeControl;

typedef struct {
    TimeControl tc;
    uint64_t timeUsed;      // Time used in current search
} TimeManagement;
```

**Functions:**
1. `time_mgmt_init(TimeManagement* tm, TimeControl* tc)` - Initialize
2. `time_mgmt_allocate(TimeManagement* tm, Board* board)` - Calculate time allocation
3. `time_mgmt_optimal_time(TimeManagement* tm)` - Get optimal time
4. `time_mgmt_max_time(TimeManagement* tm)` - Get max time
5. `time_mgmt_elapsed(TimeManagement* tm)` - Get elapsed time
6. `time_mgmt_remaining(TimeManagement* tm)` - Get remaining time
7. `time_mgmt_should_stop(TimeManagement* tm, int depth, int score)` - Check stop condition
8. `time_mgmt_scale(int depth, int movesToGo)` - Scale time for depth/moves

**Test Cases:**
1. test_time_mgmt_init - Verify initialization
2. test_time_mgmt_allocate_simple - Simple time allocation
3. test_time_mgmt_allocate_increment - With increment
4. test_time_mgmt_allocate_moves_go - With movesToGo
5. test_time_mgmt_optimal_time_calc - Optimal time calculation
6. test_time_mgmt_max_time_calc - Max time calculation
7. test_time_mgmt_elapsed - Elapsed time tracking
8. test_time_mgmt_remaining - Remaining time
9. test_time_mgmt_should_stop_elapsed - Stop when elapsed > optimal
10. test_time_mgmt_should_stop_safe - Don't stop early
11. test_time_mgmt_scale_depth - Time scaling by depth
12. test_time_mgmt_scale_movesGo - Scaling with moves to go

---

## Code Coverage Requirements (100%)

### Statement Coverage
- Every line of code must execute at least once
- All error paths must be tested

### Branch Coverage  
- Every if/else branch taken
- Every loop iteration tested (0, 1, many iterations)
- Every switch case tested

### Edge Cases
- Depth 0 (root)
- Maximum depth
- Empty move lists
- Zero time allocation
- Extreme history values
- Castling edge cases
- En passant edge cases

---

## Test Data Generation

### Positions for Testing
```c
// Test positions
static const char* POSITIONS[] = {
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",  // Start
    "r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3",
    "6k1/5ppp/8/8/8/8/5PPP/6K1 w - - 0 1",  // K vs K
    // ... more positions
};
```

### Known Results
```c
// Expected history table values after updates
// Expected killer move sequences
// Expected pruning decisions for positions
```

---

## Integration Points

### With Phase 2 Components
1. **Search.c Integration**
   - History updates in quiescence search
   - Killer moves in principal variation search
   - Futility pruning in alpha-beta
   - LMR in iterative deepening
   - Null move in root nodes

2. **Move Ordering Integration**
   - History heuristic in move ordering
   - Killer moves prioritize after TT move
   - LMR for poorly ordered moves

3. **Transposition Table Integration**
   - Store pruning decisions
   - Check TT before applying pruning

### Configuration Through Search
```c
typedef struct {
    HistoryTable history;
    KillerTable killer;
    FutilityMargins futility;
    LmrTable lmr;
    LmpConfig lmp;
    NullMoveConfig nullMove;
    ProgressiveSearch progressive;
    TimeManagement time;
} SearchConfig;
```

---

## Implementation Order

1. **Week 7: History & Killer**
   - history.c/h with tests
   - killer.c/h with tests
   - Integrate with move ordering

2. **Week 8: Futility & LMP**
   - futility.c/h with tests
   - lmp.c/h with tests
   - Integrate into search loop

3. **Week 9: LMR & Null Move**
   - lmr.c/h with tests
   - null_move.c/h with tests
   - Integration testing

4. **Week 10: Progressive & Time**
   - progressive.c/h with tests
   - time_mgmt.c/h with tests
   - Full search integration
   - Performance testing

---

## Performance Benchmarks

### Search Speed
- **Target:** 500,000+ nodes/second
- **Benchmark:** PERFT position at depth 10
- **Metric:** Compare with base alpha-beta

### ELO Impact
- History heuristic: +50-80 ELO
- Killer moves: +30-50 ELO
- Futility pruning: +40-60 ELO
- LMR: +70-100 ELO
- Null move: +60-90 ELO
- Time management: +20-40 ELO

### Test Positions
```bash
# Run benchmarks
make bench
./chess_bot_bench --position "pos1" --depth 10
./chess_bot_bench --position "pos2" --nodes 100000
```

---

## Debugging Tools

### History Table Dumper
```c
void history_dump(HistoryTable* ht);
void history_stats(HistoryTable* ht);
```

### Move Ordering Trace
```c
// Enable with -DDEBUG_ORDERING
// Logs move ordering at each depth
```

### Pruning Statistics
```c
typedef struct {
    int futilityPruned;
    int lmpPruned;
    int nullMovePruned;
    int lmrReductions;
} PruningStats;
```

---

## Success Criteria

1. **All tests pass** - 100% test coverage
2. **PERFT validation** - Correct move counts at all depths
3. **Performance** - Meet or exceed node/second target
4. **ELO gain** - 2500+ ELO on CCRL test suite
5. **Memory safety** - No leaks, no overflows
6. **Thread safety** - Safe for multi-threaded search (Phase 5 prep)

---

## Next Steps

After Phase 3 completion:
- Run full regression test suite
- Benchmark against Phase 2 engine
- Prepare for Phase 4 (NNUE evaluation)
