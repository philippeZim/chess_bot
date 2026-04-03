#ifndef ZOBRIST_H
#define ZOBRIST_H

#include "../types.h"
#include "../board/board.h"

#define CASTLING_MASK 16

void zobrist_init(void);
uint64_t zobrist_hash_piece(uint8_t piece, Square square);
uint64_t zobrist_hash_side_to_move(Color c);
uint64_t zobrist_hash_castling_rights(uint8_t castling);
uint64_t zobrist_hash_en_passant_file(int file);

uint64_t zobrist_hash_board(const Board* board);
void zobrist_update_incremental(Board* board, int from, int to, Piece captured);

#endif
