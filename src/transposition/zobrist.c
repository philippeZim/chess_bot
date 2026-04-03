#include "zobrist.h"
#include "../types.h"
#include "../board/board.h"
#include <stdint.h>
#include <stdio.h>

static uint64_t ZobristKeys[2][8][64];
static uint64_t ZobristCastling[16];
static uint64_t ZobristEnPassant[8];
static uint64_t ZobristTurn;

static uint64_t splitmix64(uint64_t* state) {
    uint64_t z = *state += 0x9e3779b97f4a7c15ULL;
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}

void zobrist_init(void) {
    uint64_t state = 0x1234567890ABCDEFULL;
    
    for (int c = 0; c < 2; c++) {
        for (int p = 0; p < 6; p++) {
            for (int sq = 0; sq < 64; sq++) {
                ZobristKeys[c][p][sq] = splitmix64(&state);
            }
        }
    }
    
    for (int i = 0; i < 16; i++) {
        ZobristCastling[i] = splitmix64(&state);
    }
    
    for (int f = 0; f < 8; f++) {
        ZobristEnPassant[f] = splitmix64(&state);
    }
    
    ZobristTurn = splitmix64(&state);
}

uint64_t zobrist_hash_piece(uint8_t piece, Square square) {
    if (piece == NO_PIECE) return 0;
    int c = piece_color(piece);
    int pt = piece_type(piece);
    return ZobristKeys[c][pt][square];
}

uint64_t zobrist_hash_side_to_move(Color c) {
    if (c == ColorWhite) return 0;
    return ZobristTurn;
}

uint64_t zobrist_hash_castling_rights(uint8_t castling) {
    return ZobristCastling[castling];
}

uint64_t zobrist_hash_en_passant_file(int file) {
    if (file < 0 || file >= 8) return 0;
    return ZobristEnPassant[file];
}

uint64_t zobrist_hash_board(const Board* board) {
    uint64_t hash = 0;
    
    for (int sq = 0; sq < 64; sq++) {
        Piece piece = board->squares[sq];
        if (piece != NO_PIECE) {
            hash ^= zobrist_hash_piece(piece, sq);
        }
    }
    
    hash ^= zobrist_hash_side_to_move(board->turn);
    hash ^= zobrist_hash_castling_rights(board->castling);
    
    if (board->enPassant != SQUARE_NB) {
        int file = square_file(board->enPassant);
        hash ^= zobrist_hash_en_passant_file(file);
    }
    
    return hash;
}

void zobrist_update_incremental(Board* board, int from, int to, Piece captured) {
    if (!board) return;
    
    Piece moving = board->squares[from];
    
    board->zobrist ^= zobrist_hash_piece(moving, from);
    board->zobrist ^= zobrist_hash_piece(moving, to);
    
    if (captured != NO_PIECE) {
        board->zobrist ^= zobrist_hash_piece(captured, to);
    }
    
    board->zobrist ^= zobrist_hash_castling_rights(board->castling);
    if (moving == W_KING || moving == B_KING) {
        if (moving == W_KING) board->castling = CASTLING_NONE;
        if (moving == B_KING) board->castling = board->castling & ~(CASTLING_WK | CASTLING_WQ);
    }
    if (moving == W_ROOK) {
        if (from == SQUARE_A1) board->castling &= ~CASTLING_WQ;
        if (from == SQUARE_H1) board->castling &= ~CASTLING_WK;
    }
    if (moving == B_ROOK) {
        if (from == SQUARE_A8) board->castling &= ~CASTLING_BQ;
        if (from == SQUARE_H8) board->castling &= ~CASTLING_BK;
    }
    board->zobrist ^= zobrist_hash_castling_rights(board->castling);
    
    board->zobrist ^= zobrist_hash_side_to_move(board->turn);
    board->turn = opposite_color(board->turn);
    board->zobrist ^= zobrist_hash_side_to_move(board->turn);
}
