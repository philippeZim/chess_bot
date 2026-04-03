#ifndef CHESS_TYPES_H
#define CHESS_TYPES_H

#include <stdint.h>
#include <stdbool.h>

typedef uint64_t Bitboard;
typedef uint8_t Square;
typedef uint8_t Piece;
typedef uint8_t Color;
typedef uint8_t PieceType;
typedef uint32_t Move;

enum {
    SQUARE_A1 = 0, SQUARE_B1, SQUARE_C1, SQUARE_D1, SQUARE_E1, SQUARE_F1, SQUARE_G1, SQUARE_H1,
    SQUARE_A2, SQUARE_B2, SQUARE_C2, SQUARE_D2, SQUARE_E2, SQUARE_F2, SQUARE_G2, SQUARE_H2,
    SQUARE_A3, SQUARE_B3, SQUARE_C3, SQUARE_D3, SQUARE_E3, SQUARE_F3, SQUARE_G3, SQUARE_H3,
    SQUARE_A4, SQUARE_B4, SQUARE_C4, SQUARE_D4, SQUARE_E4, SQUARE_F4, SQUARE_G4, SQUARE_H4,
    SQUARE_A5, SQUARE_B5, SQUARE_C5, SQUARE_D5, SQUARE_E5, SQUARE_F5, SQUARE_G5, SQUARE_H5,
    SQUARE_A6, SQUARE_B6, SQUARE_C6, SQUARE_D6, SQUARE_E6, SQUARE_F6, SQUARE_G6, SQUARE_H6,
    SQUARE_A7, SQUARE_B7, SQUARE_C7, SQUARE_D7, SQUARE_E7, SQUARE_F7, SQUARE_G7, SQUARE_H7,
    SQUARE_A8, SQUARE_B8, SQUARE_C8, SQUARE_D8, SQUARE_E8, SQUARE_F8, SQUARE_G8, SQUARE_H8,
    SQUARE_NB = 64
};

enum {
    ColorWhite,
    ColorBlack,
    ColorNB,
    COLOR_NB = ColorNB
};

enum {
    PAWN = 1,
    KNIGHT = 2,
    BISHOP = 3,
    ROOK = 4,
    QUEEN = 5,
    KING = 6,
    PIECE_TYPE_NB = 7
};

enum {
    NO_PIECE = 0,
    W_PAWN = 1, W_KNIGHT = 2, W_BISHOP = 3, W_ROOK = 4, W_QUEEN = 5, W_KING = 6,
    B_PAWN = 9, B_KNIGHT = 10, B_BISHOP = 11, B_ROOK = 12, B_QUEEN = 13, B_KING = 14,
    PIECE_NB = 16
};

#define make_piece(c, pt) (((c) << 3) | (pt))
#define piece_color(p) ((p) >> 3)
#define piece_type(p) ((p) & 7)
#define is_piece_valid(p) ((p) != NO_PIECE)

enum {
    FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H, FILE_NB
};

enum {
    RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8, RANK_NB
};

#define square_file(sq) ((sq) & 7)
#define square_rank(sq) ((sq) >> 3)
#define make_square(f, r) (((r) << 3) | (f))

#define WHITE_KING_SQUARE SQUARE_E1
#define WHITE_KING_START SQUARE_E1
#define BLACK_KING_SQUARE SQUARE_E8
#define WHITE_KING_FILE FILE_E
#define BLACK_KING_FILE FILE_E
#define WHITE_KING_RANK RANK_1
#define BLACK_KING_RANK RANK_8

#define square_opposite(color) ((color) ^ 1)
#define opposite_color(c) ((c) ^ 1)

enum {
    MAX_MOVES_PER_POSITION = 256,
    MAX_PLY = 128,
    MAX_POSITIONS = 1024,
    BOARD_SIZE = 64,
    MAX_DEPTH = 64
};

#define SCORE_DRAW 0
#define SCORE_MATE 30000
#define SCORE_MATE_IN_PLY(ply) (SCORE_MATE - (ply))
#define SCORE_INFINITE 32000
#define SCORE_NONE 32001

enum {
    VERSION_MAJOR = 0,
    VERSION_MINOR = 1,
    VERSION_PATCH = 0
};

#endif
