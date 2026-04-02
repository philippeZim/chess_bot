# Chess Bot - Agent Instructions

## Build Commands

```bash
# Build the project
make

# Build with debug symbols and sanitizers
make debug

# Build and run all tests
make test

# Clean build artifacts
make clean

# Full rebuild
make clean && make test
```

## Running a Single Test

The test suite is compiled into a single executable. Individual tests are printed to stdout:

```bash
make test
```

To run specific test files independently, modify the Makefile to only build specific test objects.

## Code Style Guidelines

### General
- Use C11 standard (`-std=c11`)
- Zero warnings allowed (`-Wall -Wextra -Werror`)
- 4 spaces for indentation (no tabs)
- Maximum line length: 100 characters
- K&R style braces

### Naming Conventions
- **Files**: lowercase with underscores (`move_generation.c`)
- **Types**: PascalCase (`ChessBoard`, `MoveList`)
- **Functions**: lowercase with underscores, module prefix (`board_init()`, `board_clone()`)
- **Constants**: uppercase with underscores (`MAX_MOVES`, `SQUARE_A1`)
- **Static functions**: prefix with underscore (`_helper_function()`)

### Types
- Use `<stdint.h>` types: `uint8_t`, `uint64_t`, etc.
- Use `bool` from `<stdbool.h>`
- Use `typedef` for structs
- Explicit types only - no implicit int

### Error Handling
- Return `bool` for success/failure
- Return negative values for errors
- Always check return values
- Use assertions for impossible conditions

### Memory
- Always check malloc return values
- Free in reverse allocation order
- No memory leaks in error paths

### Headers
- Include guards: `#ifndef FOO_H` / `#define FOO_H` / `#endif`
- Sort includes: system first, then local
- Use `< >` for system, `" "` for local

## Testing

### Custom Testing Framework
- Tests in `test/unit/` directory
- Simple assertion macros: `ASSERT_EQ`, `ASSERT_TRUE`, `ASSERT_FALSE`
- Each module has test file
- Tests run via `make test`

### Coverage Goal
- 100% line coverage for all modules
- Test all success and failure paths
- Use PERFT for move generation verification

### Test Structure
```c
int test_function_name(void) {
    // Setup
    // Execute
    // Assert
    // Return 1 for pass, 0 for fail
}
```

## Architecture

### Bitboards
- 64-bit integers for piece positions
- A1 = bit 0, H8 = bit 63
- One bitboard per piece type and color
- Attack tables precomputed

### Board Representation
```c
typedef struct {
    Bitboard byType[PIECE_TYPE_NB];  // Bitboards for each piece type
    Bitboard byColor[COLOR_NB];      // Bitboards for each color
    Piece squares[BOARD_SIZE];       // Piece on each square
    uint8_t castling;                // Castling rights
    uint8_t enPassant;               // En passant square
    Color turn;                      // Side to move
} Board;
```

### Move Representation
- Packed 32-bit integer
- Contains from, to, promotion piece
- Special move flags

## Project Structure

```
src/
├── types.h              # Core types
├── board/
│   ├── bitboard.c/h     # Bitboard operations
│   └── board.c/h        # Board state
├── move/
│   ├── movegen.c/h      # Move generation
│   └── move.c/h         # Move utilities
├── search/              # Search algorithms
├── evaluation/          # Position evaluation (NNUE)
├── transposition/       # Transposition tables
├── uci/                 # UCI protocol
└── utils/               # Utilities
test/unit/
├── test_bitboard.c      # Bitboard tests
├── test_board.c         # Board tests
└── ...
```

## Git Workflow
- Commit frequently with descriptive messages
- Run tests before committing
- No push with failing tests
- Feature branches for new functionality

## Key Implementation Notes

### Knight Attacks
Use precomputed tables or bit manipulation with file masks to prevent wrapping.

### Sliding Piece Attacks  
Use magic bitboards or ray-based calculation with occupancy masks.

### FEN Parsing/handle
- Piece placement (rank by rank)
- Side to move
- Castling rights
- En passant target
- Halfmove clock
- Fullmove number

### UCI Protocol
Implement: `uci`, `isready`, `position`, `go`, `stop`, `quit`

## Performance Targets (3500+ ELO)
- Search speed: 500,000+ nodes/second
- NNUE evaluation: SIMD optimized
- Multi-threaded search
- Transposition table: 128MB minimum
