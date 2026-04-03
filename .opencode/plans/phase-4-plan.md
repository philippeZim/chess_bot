# Phase 4: Evaluation System Plan (Weeks 11-15)

## Goal
Implement Stockfish-level evaluation system with NNUE (Efficiently Updatable Neural Network) to reach 3000+ ELO.

## Current State
- Basic `evaluate.c` exists with piece values and PSQT tables
- No pawn structure evaluation
- No NNUE implementation
- No evaluation tests
- No material balance or phase-based evaluation

## Architecture Overview

### Evaluation Pipeline
```
Position -> Material Balance -> PST Evaluation -> Pawn Structure -> NNUE -> Final Score
```

### NNUE Architecture (Half-KRNN)
```
Input: 768 features (64 squares × 12 piece types)
Hidden: 256 neurons (ReLU/SCReLU activation)
Output: 1 value (centipawns)
```

## Implementation Tasks

### Task 4.1: Enhanced Piece-Square Tables (Week 11, Days 1-3)

#### Files to Create/Modify
- `src/evaluation/piece_value.c` - Enhanced PSQT implementation
- `src/evaluation/piece_value.h` - PSQT interface
- `test/unit/test_piece_value.c` - PSQT tests

#### Implementation Details

1. **Midgame/Endgame PSQT Split**
   ```c
   typedef struct {
       int mg_values[PIECE_TYPE_NB][64];  // Midgame tables
       int eg_values[PIECE_TYPE_NB][64];  // Endgame tables
   } PieceSquareTables;
   ```

2. **Piece Values**
   - Pawn: 100 (mg), 100 (eg)
   - Knight: 320 (mg), 340 (eg)
   - Bishop: 330 (mg), 340 (eg)
   - Rook: 500 (mg), 550 (eg)
   - Queen: 900 (mg), 1000 (eg)
   - King: 0 (mg), 0 (eg)

3. **King Safety Tables**
   - Separate king safety PSQT for endgame
   - King shelter evaluation
   - Pawn shield bonuses

4. **Piece Activity Bonuses**
   - Mobility bonuses per piece type
   - Outpost bonuses for knights
   - Open file bonuses for rooks
   - Trapped piece detection

#### Acceptance Criteria
- All piece types have mg/eg PSQT values
- Tables are symmetric (flip for black)
- Tests verify known positions score correctly
- Integration with main evaluate() function

---

### Task 4.2: Pawn Structure Evaluation (Week 11, Days 4-5)

#### Files to Create
- `src/evaluation/pawn_table.c` - Pawn structure evaluation
- `src/evaluation/pawn_table.h` - Pawn table interface
- `test/unit/test_pawn_table.c` - Pawn structure tests

#### Implementation Details

1. **Pawn Hash Table**
   ```c
   typedef struct {
       uint64_t key;           // Zobrist pawn hash
       int16_t score_mg;       // Midgame score
       int16_t score_eg;       // Endgame score
       Bitboard passed_pawns[COLOR_NB];
       Bitboard pawn_chains[COLOR_NB];
       uint8_t king_shelter[COLOR_NB];
       uint8_t passed_pawn_count[COLOR_NB];
   } PawnEntry;
   
   #define PAWN_TABLE_SIZE 16384
   PawnEntry pawn_table[PAWN_TABLE_SIZE];
   ```

2. **Pawn Structure Features**
   - **Doubled pawns**: -15 penalty per doubled pawn
   - **Isolated pawns**: -20 penalty per isolated pawn
   - **Backward pawns**: -10 penalty per backward pawn
   - **Passed pawns**: Bonus based on rank advancement
   - **Pawn chains**: Bonus for connected pawns
   - **Candidate passers**: Small bonus for potential passed pawns
   - **Pawn storms**: Bonus/penalty based on king position

3. **Passed Pawn Evaluation**
   ```c
   static const int PASSED_PAWN_BONUS_MG[8] = {0, 10, 20, 35, 60, 100, 150, 200};
   static const int PASSED_PAWN_BONUS_EG[8] = {0, 15, 30, 50, 80, 130, 200, 300};
   ```

4. **King Shelter Evaluation**
   - Pawns in front of king
   - File proximity to king
   - Open file penalties near king

#### Acceptance Criteria
- Pawn hash table with LRU replacement
- All pawn structure features evaluated
- Tests verify doubled/isolated/passed pawn detection
- Known positions score correctly (e.g., isolated queen pawn)

---

### Task 4.3: NNUE Architecture (Week 12-13)

#### Files to Create
- `src/evaluation/nnue.c` - NNUE inference engine
- `src/evaluation/nnue.h` - NNUE interface
- `src/evaluation/nnue_arch.h` - NNUE architecture constants
- `src/evaluation/nnue_io.c` - Network loading utilities
- `src/evaluation/nnue_io.h` - Network loading interface
- `src/evaluation/nnue_update.c` - Incremental update logic
- `src/evaluation/nnue_update.h` - Incremental update interface
- `test/unit/test_nnue.c` - NNUE tests

#### Implementation Details

1. **NNUE Architecture Constants** (`nnue_arch.h`)
   ```c
   #define NNUE_INPUT_SIZE 768      // 64 squares * 12 piece types
   #define NNUE_HIDDEN_SIZE 256     // Hidden layer neurons
   #define NNUE_OUTPUT_SIZE 1       // Single output value
   #define NNUE_FEATURE_BITS 64
   #define NNUE_PIECE_TYPES 12
   ```

2. **Network Structure**
   ```c
   typedef struct {
       // Input -> Hidden layer
       int16_t weights_ih[NNUE_INPUT_SIZE][NNUE_HIDDEN_SIZE];
       int16_t bias_h[NNUE_HIDDEN_SIZE];
       
       // Hidden -> Output layer
       int16_t weights_ho[NNUE_HIDDEN_SIZE * 2];  // *2 for both sides
       int16_t bias_o;
   } NNUEWeights;
   ```

3. **Accumulator State**
   ```c
   typedef struct {
       int16_t values[NNUE_HIDDEN_SIZE];
       bool computed;
   } Accumulator;
   
   typedef struct {
       Accumulator accumulators[COLOR_NB][MAX_PLY];
       int current_ply;
   } NNUEState;
   ```

4. **Feature Index Calculation**
   ```c
   // Half-KP relative to king position
   static int feature_index(Color side, Square king_sq, Square piece_sq, PieceType pt) {
       int color_offset = (piece_color(piece) == side) ? 0 : 64 * 6;
       int piece_offset = (piece_type(piece) - 1) * 64;
       int square = (piece_color(piece) == side) ? piece_sq : (piece_sq ^ 56);
       return color_offset + piece_offset + square;
   }
   ```

5. **Incremental Update**
   - On make_move: Subtract old feature, add new feature
   - On unmake_move: Reverse the update
   - Full refresh when accumulator becomes invalid

6. **Inference Function**
   ```c
   int nnue_evaluate(const Board* board, NNUEState* state) {
       // Compute or use cached accumulator
       // Apply ReLU/SCReLU activation
       // Compute output layer
       // Return centipawn score
   }
   ```

#### SIMD Optimization (AVX2)
```c
// Vectorized accumulator update
#include <immintrin.h>

void nnue_update_accumulator_avx2(int16_t* dst, const int16_t* src,
                                   const int16_t* weights, int add_idx) {
    __m256i* d = (__m256i*)dst;
    const __m256i* s = (const __m256i*)src;
    const __m256i* w = (const __m256i*)&weights[add_idx * NNUE_HIDDEN_SIZE];
    
    for (int i = 0; i < NNUE_HIDDEN_SIZE / 16; i++) {
        d[i] = _mm256_add_epi16(s[i], w[i]);
    }
}
```

#### Network File Format
```
Binary format:
- Magic number: 4 bytes (0x4E4E5545 = "NNUE")
- Version: 4 bytes
- Input size: 4 bytes (768)
- Hidden size: 4 bytes (256)
- Weights IH: 768 * 256 * 2 bytes (int16)
- Bias H: 256 * 2 bytes
- Weights HO: 256 * 2 * 2 bytes (int16)
- Bias O: 2 bytes
- Total: ~400KB
```

#### Acceptance Criteria
- NNUE network loads from binary file
- Incremental update works correctly
- Full refresh produces same result as incremental
- SIMD version matches scalar version
- Tests verify against known network outputs
- Performance: < 1 microsecond per evaluation

---

### Task 4.4: NNUE Network Training (Week 14, Days 1-3)

#### Files to Create
- `tools/train_nnue.py` - Training script (PyTorch)
- `tools/export_nnue.py` - Export to binary format
- `tools/validate_nnue.py` - Validation script

#### Training Pipeline

1. **Data Collection**
   - Use existing game databases (CCRL, TCEC)
   - Self-play positions from current engine
   - Label positions with deeper search values (depth 12+)

2. **Network Architecture (PyTorch)**
   ```python
   class NNUE(nn.Module):
       def __init__(self):
           super().__init__()
           self.fc1 = nn.Linear(768, 256)
           self.fc2 = nn.Linear(256, 1)
           
       def forward(self, x):
           x = torch.clamp(self.fc1(x), min=0)  # ReLU
           x = self.fc2(x)
           return x
   ```

3. **Training Parameters**
   - Loss function: MSE with search scores
   - Optimizer: AdamW
   - Learning rate: 0.001 with decay
   - Batch size: 16384
   - Epochs: 50-100
   - Regularization: L2 weight decay 0.01

4. **Data Format**
   ```
   Each training sample:
   - 768-bit feature vector (one-hot encoded)
   - Target score (centipawns from search)
   - Game result (optional, for endgame)
   ```

5. **Export Process**
   - Quantize weights to int16
   - Export in binary format matching C structure
   - Validate exported weights match PyTorch output

#### Acceptance Criteria
- Training script produces converging loss
- Exported network loads in C implementation
- Validation set accuracy > 85%
- Network output matches known positions

---

### Task 4.5: Evaluation Entry Point (Week 14, Days 4-5)

#### Files to Modify
- `src/evaluation/evaluate.c` - Main evaluation function
- `src/evaluation/evaluate.h` - Updated interface

#### Implementation Details

1. **Phase-Based Evaluation**
   ```c
   typedef struct {
       int mg_score;
       int eg_score;
       int game_phase;  // 0-256 (0=endgame, 256=midgame)
   } EvalResult;
   ```

2. **Evaluation Components**
   ```c
   int evaluate(const Board* board) {
       EvalResult result = {0, 0, 0};
       
       // 1. Material balance
       eval_material(board, &result);
       
       // 2. Piece-square tables
       eval_psqt(board, &result);
       
       // 3. Pawn structure
       eval_pawns(board, &result);
       
       // 4. King safety
       eval_king_safety(board, &result);
       
       // 5. Piece activity/mobility
       eval_mobility(board, &result);
       
       // 6. NNUE evaluation (if available)
       if (nnue_loaded) {
           result.nnue_score = nnue_evaluate(board, nnue_state);
       }
       
       // Blend mg/eg scores based on game phase
       int phase = compute_game_phase(board);
       int score = blend_scores(result.mg_score, result.eg_score, phase);
       
       // Add NNUE if loaded (weighted blend)
       if (nnue_loaded) {
           score = (score * 3 + result.nnue_score * 7) / 10;
       }
       
       return board->turn == ColorWhite ? score : -score;
   }
   ```

3. **Game Phase Calculation**
   ```c
   static int compute_game_phase(const Board* board) {
       int phase = 0;
       // Piece values for phase calculation
       phase += bitboard_popcount(board->byType[KNIGHT]) * 1;
       phase += bitboard_popcount(board->byType[BISHOP]) * 1;
       phase += bitboard_popcount(board->byType[ROOK]) * 2;
       phase += bitboard_popcount(board->byType[QUEEN]) * 4;
       return MIN(phase, 24);  // Normalize to 0-24
   }
   ```

4. **Tempo Bonus**
   - Add small bonus (10-15 cp) for side to move

#### Acceptance Criteria
- All evaluation components integrated
- Phase-based blending works correctly
- Known positions score within expected ranges
- Performance: < 2 microseconds per evaluation (without NNUE)

---

### Task 4.6: Evaluation Tests (Week 15)

#### Files to Create
- `test/unit/test_piece_value.c` - PSQT tests
- `test/unit/test_pawn_table.c` - Pawn structure tests
- `test/unit/test_nnue.c` - NNUE inference tests
- `test/unit/test_eval.c` - Full evaluation tests
- `test/regression/test_known_positions.c` - Known position tests

#### Test Coverage

1. **Piece-Square Table Tests**
   - Verify piece values are correct
   - Verify PSQT symmetry (white/black)
   - Test known position scores
   - Test edge cases (empty board, single piece)

2. **Pawn Structure Tests**
   - Test doubled pawn detection
   - Test isolated pawn detection
   - Test backward pawn detection
   - Test passed pawn detection
   - Test pawn hash table collisions
   - Verify scores for known pawn structures

3. **NNUE Tests**
   - Test network loading (valid/invalid files)
   - Test feature index calculation
   - Test incremental update correctness
   - Test full refresh matches incremental
   - Test SIMD vs scalar output match
   - Test accumulator state management

4. **Full Evaluation Tests**
   - Test material balance
   - Test phase-based evaluation
   - Test known position scores:
     - Starting position: ~0-20 cp
     - Material advantage positions
     - Pawn structure positions
     - King safety positions

5. **Regression Tests**
   - CCRL benchmark positions
   - EPD test suite
   - Known mate positions
   - Draw positions

#### Test Structure
```c
int test_pawn_structure_doubled(void) {
    Board board;
    board_set_fen(&board, "8/8/8/8/8/P7/P7/8 w - - 0 1");
    
    int score = eval_pawns(&board);
    ASSERT_TRUE(score < 0);  // White has doubled pawns
    
    return 1;
}
```

---

## File Structure After Phase 4

```
src/evaluation/
├── evaluate.c           # Main evaluation entry (modified)
├── evaluate.h           # Updated interface
├── piece_value.c        # PSQT implementation (new)
├── piece_value.h        # PSQT interface (new)
├── pawn_table.c         # Pawn structure evaluation (new)
├── pawn_table.h         # Pawn table interface (new)
├── nnue.c               # NNUE inference engine (new)
├── nnue.h               # NNUE interface (new)
├── nnue_arch.h          # NNUE architecture constants (new)
├── nnue_io.c            # Network loading (new)
├── nnue_io.h            # Network loading interface (new)
├── nnue_update.c        # Incremental update (new)
└── nnue_update.h        # Incremental update interface (new)

test/unit/
├── test_piece_value.c   # PSQT tests (new)
├── test_pawn_table.c    # Pawn structure tests (new)
├── test_nnue.c          # NNUE tests (new)
└── test_eval.c          # Full evaluation tests (new)

test/regression/
└── test_known_positions.c  # Known position tests (new)

tools/
├── train_nnue.py        # NNUE training script (new)
├── export_nnue.py       # Export to binary (new)
└── validate_nnue.py     # Validation script (new)

data/
└── nnue/
    └── network.bin      # Trained network (new)
```

---

## Makefile Updates

Add to `SRCS`:
```makefile
src/evaluation/piece_value.c \
src/evaluation/pawn_table.c \
src/evaluation/nnue.c \
src/evaluation/nnue_io.c \
src/evaluation/nnue_update.c
```

Add to `TEST_OBJS`:
```makefile
$(TEST_BUILD_DIR)/test_piece_value.o \
$(TEST_BUILD_DIR)/test_pawn_table.o \
$(TEST_BUILD_DIR)/test_nnue.o \
$(TEST_BUILD_DIR)/test_eval.o
```

Add compiler flags for SIMD:
```makefile
CFLAGS += -mavx2 -mfma  # Optional: for SIMD acceleration
```

---

## Performance Targets

| Metric | Target |
|--------|--------|
| Basic evaluation (PST + pawns) | < 2 microseconds |
| NNUE evaluation (scalar) | < 10 microseconds |
| NNUE evaluation (AVX2) | < 3 microseconds |
| Pawn hash hit rate | > 80% |
| NNUE incremental update | < 1 microsecond |
| Memory usage (NNUE) | < 1MB |

---

## Dependencies

1. **Phase 1**: Bitboard operations, board state, move generation
2. **Phase 2**: Search engine (for training data generation)
3. **Phase 3**: Advanced search (for deeper position labeling)

---

## Risks and Mitigations

| Risk | Mitigation |
|------|------------|
| NNUE training complexity | Start with handcrafted evaluation as fallback |
| SIMD compatibility | Provide scalar fallback for all SIMD functions |
| Network quality | Use pre-trained weights if training fails |
| Performance regression | Benchmark after each component addition |
| Memory usage | Monitor accumulator state allocation |

---

## Milestones

| Week | Milestone |
|------|-----------|
| 11 | PSQT and pawn structure complete |
| 12 | NNUE architecture implemented |
| 13 | NNUE incremental update working |
| 14 | NNUE training and evaluation entry complete |
| 15 | All tests passing, benchmarks met |

---

## Success Criteria

- [ ] All evaluation components implemented and tested
- [ ] 100% line coverage for evaluation modules
- [ ] Known positions score correctly
- [ ] NNUE network loads and evaluates correctly
- [ ] Performance targets met
- [ ] Integration with search engine working
- [ ] Engine reaches 3000+ ELO in testing