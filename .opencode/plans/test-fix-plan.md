# Chess Bot - Test Fix Plan

**Current Status**: 17/23 tests passing (74%)  
**Goal**: 23/23 tests passing (100%)  
**Files to Modify**: 3 files (types.h, board.c, bitboard.c, test_bitboard.c)  
**Estimated Time**: 30-45 minutes

---

## Test Failure Summary

```
Bitboard Tests: 8/12 passed
  - test_square_attacks_knight_e4... FAIL (got 6 attacks)
  - test_square_attacks_knight_a1... FAIL (got 4 attacks)
  - test_square_attacks_king_a1... FAIL (got 4 attacks)
  - test_bishop_attacks_empty... FAIL (corner bishop has 7 attacks)

Board Tests: 9/11 passed
  - test_board_set_fen_with_en_passant... FAIL
  - test_board_piece_counts... FAIL
```

---

## Root Cause Analysis

### 1. Piece Type Misalignment (Causes 2 failing tests)

**Evidence from debug output**:
```
byType[0]: 16 (expected 8)  <- All pawns going here
byType[1]: 4  (expected 2)  <- All knights going here  
byColor[ColorWhite]: 16 (correct)
```

**Root Cause**: The `piece_type()` macro returns wrong indices due to enum constant mismatch.

In `src/types.h`:
```c
enum { PAWN = 0, KNIGHT = 1, BISHOP = 2, ROOK = 3, QUEEN = 4, KING = 5 };
enum { W_PAWN = 1, W_KNIGHT = 2, ... B_PAWN = 9, ... };
```

The `make_piece(c, pt)` macro creates `(c << 3) | pt`, giving:
- `make_piece(ColorWhite, PAWN) = (0 << 3) | 0 = 0`
- `make_piece(ColorBlack, PAWN) = (1 << 3) | 0 = 8`

But enum has `W_PAWN = 1, B_PAWN = 9`. This means piece values are off by 1.

**Impact**: `piece_type(piece)` returns wrong index, so all pieces map to wrong byType[] index:
- Pawns (type 0) → byType[1] 
- Knights (type 1) → byType[2]
- ... and so on

Since we count ALL piece types in the FEN parsing, byType[1] ends up with both white pawns (1) AND black pawns (9), giving count of 16 instead of 8.

---

### 2. En Passant Square Not Set (1 failing test)

**Debug result**: `board.enPassant = 64` (SQUARE_NB, not set) but `SQUARE_E3 = 20`

**Root Cause**: After parsing castling rights (e.g., 'KQkq'), the pointer `p` points to the space after castling, but the en passant field in FEN is after more spaces (e.g., ' e3').

The code checks `if (*p != '-' && isalpha(*p) ...)` but `*p` is still pointing at castling field or space, not at the en passant square.

**FEN format**: `rnbqkbnr/ppppppp1/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1`
                                              ↑
                                              After 'KQkq', pointer is at space, not 'e3'

---

### 3. Knight Attack Generation Wrong Direction (2 tests fail)

**Problem**: E4 gets 6 attacks (expected 8), A1 gets 4 attacks (expected 2)

**Root Cause**: The knight attack calculation uses bit shifts with file masks:
```c
attacks |= (bb >> 17) & ~BB_A_FILE;
attacks |= (bb << 17) & ~BB_H_FILE;
```

The issue is that for a knight on A1 (bit 0):
- `(1ULL << 0) >> 17` = 0, so no contribution
- `(1ULL << 0) << 17` = bit 17, which is B2 (correct)
- But `(1ULL << 0) << 15` = bit 15 = A3? No, it should be masked out

Actually the file mask logic is fundamentally flawed for edge cases. The shifts don't work correctly because we're shifting the entire bitboard, not checking individual squares.

---

### 4. King Attack Generation Wrong (1 test fails)

**Problem**: A1 gets 4 attacks (expected 3)

**Root Cause**: Same as knight - file masks don't prevent wrapping correctly.

---

### 5. Bishop Test Expectation Wrong (1 test fails)

**Problem**: Corner bishop A1 has 7 diagonal attacks, test expects 13.

**Root Cause**: Test was written incorrectly. A corner bishop only reaches 7 squares diagonally (B2, C3, D4, E5, F6, G7, H8), 13 is for center board positions like D4.

---

## Fixes Required

### Fix 1: Piece Type Constants (Critical - affects 2 tests)

**File**: `src/types.h`  
**Lines**: 42-47

**Current**:
```c
enum {
    NO_PIECE = 0,
    W_PAWN = 1, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
    B_PAWN = 9, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING,
    PIECE_NB = 16
};
```

**New**:
```c
enum {
    NO_PIECE = 0,
    W_PAWN = 0, W_KNIGHT = 1, W_BISHOP = 2, W_ROOK = 3, W_QUEEN = 4, W_KING = 5,
    B_PAWN = 8, B_KNIGHT = 9, B_BISHOP = 10, B_ROOK = 11, B_QUEEN = 12, B_KING = 13,
    PIECE_NB = 16
};
```

**Why**: Makes piece values consistent with `make_piece(c, pt) = (c << 3) | pt`:
- `W_PAWN = (0 << 3) | 0 = 0`
- `B_PAWN = (1 << 3) | 0 = 8`

This ensures `piece_type()` and `piece_color()` work correctly.

**Fix 2a: En Passant FEN Parsing**

**File**: `src/board/board.c`  
**Lines**: 86-91

**Current**:
```c
while (*p && isspace(*p)) p++;
board->enPassant = SQUARE_NB;
if (*p != '-' && isalpha(*p) && p[1] >= '1' && p[1] <= '8') {
    board->enPassant = string_to_square(p);
    p += 2;
}
```

**New**:
```c
while (*p && isspace(*p)) p++;
board->enPassant = SQUARE_NB;
if (isalpha(*p) && p[1] >= '1' && p[1] <= '8') {
    board->enPassant = string_to_square(p);
    p += 2;
}
```

**Why**: Remove the erroneous `*p != '-'` check. After skipping spaces with `while (*p && isspace(*p)) p++`, if we have a valid en passant square, it will be `isalpha(square)`. If we have '-' (meaning no en passant), `isalpha('-')` is false, so it works correctly.

---

### Fix 3: Knight Attack Generation (2 tests)

**File**: `src/board/bitboard.c`  
**Lines**: 58-69

**Current**:
```c
Bitboard square_attacks_knight(Square square) {
    Bitboard bb = 1ULL << square;
    Bitboard attacks = 0;
    attacks |= (bb >> 17) & ~BB_A_FILE;
    attacks |= (bb >> 15) & ~BB_H_FILE;
    attacks |= (bb >> 10) & ~(BB_A_FILE | BB_B_FILE);
    attacks |= (bb >> 6) & ~(BB_G_FILE | BB_H_FILE);
    attacks |= (bb << 17) & ~BB_H_FILE;
    attacks |= (bb << 15) & ~BB_A_FILE;
    attacks |= (bb << 10) & ~(BB_G_FILE | BB_H_FILE);
    attacks |= (bb << 6) & ~(BB_A_FILE | BB_B_FILE);
    return attacks;
}
```

**New** (simpler, bounds-checking approach):
```c
Bitboard square_attacks_knight(Square square) {
    int r = square_rank(square);
    int f = square_file(square);
    Bitboard attacks = 0;
    
    /* All 8 possible knight moves */
    int dr[] = {-2, -2, -1, -1, 1, 1, 2, 2};
    int df[] = {-1, 1, -2, 2, -2, 2, -1, 1};
    
    for (int i = 0; i < 8; i++) {
        int nr = r + dr[i];
        int nf = f + df[i];
        if (nr >= 0 && nr < 8 && nf >= 0 && nf < 8) {
            attacks |= (1ULL << make_square(nf, nr));
        }
    }
    
    return attacks;
}
```

**Why**: Explicitly checks board bounds instead of trying to prevent wrapping with file masks. This is:
- 100% correct
- Easier to understand and verify
- Performance difference negligible compared to lookup tables

---

### Fix 4: King Attack Generation (1 test)

**File**: `src/board/bitboard.c`  
**Lines**: 72-83

**Current**:
```c
Bitboard square_attacks_king(Square square) {
    Bitboard bb = 1ULL << square;
    Bitboard attacks = 0;
    attacks |= (bb >> 8);
    attacks |= (bb << 8);
    attacks |= (bb >> 1) & ~BB_A_FILE;
    attacks |= (bb << 1) & ~BB_H_FILE;
    attacks |= (bb >> 9) & ~BB_A_FILE;
    attacks |= (bb >> 7) & ~BB_H_FILE;
    attacks |= (bb << 7) & ~BB_A_FILE;
    attacks |= (bb << 9) & ~BB_H_FILE;
    return attacks;
}
```

**New**:
```c
Bitboard square_attacks_king(Square square) {
    int r = square_rank(square);
    int f = square_file(square);
    Bitboard attacks = 0;
    
    /* All 8 possible king moves */
    int dr[] = {-1, -1, -1, 0, 0, 1, 1, 1};
    int df[] = {-1, 0, 1, -1, 1, -1, 0, 1};
    
    for (int i = 0; i < 8; i++) {
        int nr = r + dr[i];
        int nf = f + df[i];
        if (nr >= 0 && nr < 8 && nf >= 0 && nf < 8) {
            attacks |= (1ULL << make_square(nf, nr));
        }
    }
    
    return attacks;
}
```

**Why**: Same as knight fix - explicit bounds checking prevents wrapping issues.

---

### Fix 5: Bishop Test Expectation (1 test)

**File**: `test/unit/test_bitboard.c`  
**Line**: 107

**Current**:
```c
if (bitboard_popcount(a1_attacks) == 13) {
```

**New**:
```c
if (bitboard_popcount(a1_attacks) == 7) {
```

**Why**: Corner bishop A1 has diagonals B2, C3, D4, E5, F6, G7, H8 = 7 squares. The test expectation of 13 is for a center square like D4.

---

## Validation Plan

After applying all fixes:

### Step 1: Clean Build
```bash
make clean
ls build/  # Should be empty or not exist
```

### Step 2: Run Tests
```bash
make test
```

**Expected output**:
```
Chess Bot Test Suite
====================

Bitboard Tests
--------------
  test_bitboard_from_square... PASS
  test_bitboard_popcount_all... PASS
  test_bitboard_pop_lsb... PASS
  test_square_attacks_knight_e4... PASS
  test_square_attacks_knight_a1... PASS
  test_square_attacks_king_a1... PASS
  test_pawn_attacks_white... PASS
  test_pawn_attacks_black... PASS
  test_bishop_attacks_empty... PASS
  test_rook_and_queen_attacks... PASS
  test_string_conversion... PASS
  test_files_and_ranks... PASS

Bitboard: 12/12 passed

Board Tests
-----------
  test_board_init... PASS
  test_board_set_fen_startpos... PASS
  test_board_set_fen_black_to_move... PASS
  test_board_set_fen_no_castling... PASS
  test_board_set_fen_with_en_passant... PASS
  test_board_get_fen... PASS
  test_board_clone... PASS
  test_board_is_invalid... PASS
  test_board_piece_counts... PASS
  test_board_color_bitboards... PASS
  test_board_is_check... PASS

Board: 11/11 passed

====================
Test run complete!
====================
```

### Step 3: Verify Critical Values
Run debug tests to confirm:
```c
// Piece type counts
bitboard_popcount(board.byType[PAWN]) == 8        // Was 16
bitboard_popcount(board.byType[KNIGHT]) == 2      // Was 4

// En passant
board.enPassant == SQUARE_E3                      // Was SQUARE_NB (64)

// Knight attacks
bitboard_popcount(square_attacks_knight(SQUARE_A1)) == 2
bitboard_popcount(square_attacks_knight(SQUARE_E4)) == 8

// King attacks
bitboard_popcount(square_attacks_king(SQUARE_A1)) == 3

// Bishop attacks
bitboard_popcount(bishop_attacks(SQUARE_A1, 0)) == 7
```

---

## File Change Summary

| File | Lines | Change Type | Impact |
|------|-------|-------------|--------|
| `src/types.h` | 43-47 | Modify enum values | Fixes piece type detection |
| `src/board/board.c` | 88-91 | Remove condition check | Fixes en passant parsing |
| `src/board/bitboard.c` | 58-69 | Rewrite function | Fixes knight attacks |
| `src/board/bitboard.c` | 72-83 | Rewrite function | Fixes king attacks |
| `test/unit/test_bitboard.c` | 107 | Change expected value | Fixes false failure |

**Total changes**: 5 files, ~50 lines modified

---

## Risk Assessment

| Fix | Risk Level | Testing Required |
|-----|------------|------------------|
| Fix 1 (Piece types) | HIGH | Full test suite + any code using W_PAWN/B_PAWN |
| Fix 2 (En passant) | LOW | FEN parsing tests only |
| Fix 3 (Knight attacks) | MEDIUM | Move generation + check detection |
| Fix 4 (King attacks) | MEDIUM | Check detection + king safety |
| Fix 5 (Test expectation) | NONE | Just update test, no functional change |

**Rollback Plan**: If any issue arises after Fix 1, revert to previous commit and investigate piece type usage throughout codebase.

---

## Next Steps

1. Review this plan carefully
2. Apply fixes in order (1→5)  
3. Run `make test` after each fix to verify
4. Run full test suite when all fixes applied
5. Commit changes with message "Fix all unit test failures"
