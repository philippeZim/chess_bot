#ifndef BOARD_H
#define BOARD_H

#include "../types.h"
#include <stddef.h>

typedef struct {
    Bitboard byType[PIECE_TYPE_NB];
    Bitboard byColor[ColorNB];
    Piece squares[BOARD_SIZE];
    uint8_t castling;
    uint8_t enPassant;
    uint8_t halfMoves;
    uint16_t moveNumber;
    Color turn;
    uint64_t zobrist;
    uint64_t pawnHistory;
} Board;

typedef struct {
    Board board;
    uint64_t history[1024];
    uint8_t historyCount;
} Position;

#define CASTLING_WK 1
#define CASTLING_WQ 2
#define CASTLING_BK 4
#define CASTLING_BQ 8
#define CASTLING_NONE 0
#define CASTLING_ALL 15

#define HAS_CASTLING(b, c) ((b)->castling & (c))

bool board_init(Board* board);
bool board_set_fen(Board* board, const char* fen);
bool board_get_fen(Board* board, char* fen, size_t size);
bool board_is_valid(const Board* board);
bool board_is_check(const Board* board);
bool board_is_checkmate(const Board* board);
bool board_is_stalemate(const Board* board);
bool board_is_insufficient_material(const Board* board);
bool board_is_repetition(const Board* board, const Position* pos);
void board_clone(const Board* src, Board* dst);
void board_print(const Board* board);

#define IS_WHITE_CASTLING(b) ((b)->turn == ColorWhite)
#define IS_BLACK_CASTLING(b) ((b)->turn == ColorBlack)

#endif
