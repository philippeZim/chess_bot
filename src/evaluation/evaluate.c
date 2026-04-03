#include "evaluate.h"
#include "../board/bitboard.h"
#include <stdlib.h>

static const int PIECE_VALUES[] = {
    [PAWN] = 100,
    [KNIGHT] = 320,
    [BISHOP] = 330,
    [ROOK] = 500,
    [QUEEN] = 900,
    [KING] = 0
};

static const int PAWN_PSQT[64] = {
      0,   0,   0,   0,   0,   0,  0,   0,
     50,  50,  50,  50,  50,  50, 50,  50,
     10,  10,  20,  30,  30,  20, 10,  10,
      5,   5,  10,  25,  25,  10,  5,   5,
      0,   0,   0,  20,  20,   0,  0,   0,
      5,  -5, -10,   0,   0, -10, -5,   5,
      5,  10,  10, -20, -20,  10, 10,   5,
      0,   0,   0,   0,   0,   0,  0,   0
};

static const int KNIGHT_PSQT[64] = {
    -50, -40, -30, -30, -30, -30, -40, -50,
    -40, -20,   0,   0,   0,   0, -20, -40,
    -30,   0,  10,  15,  15,  10,   0, -30,
    -30,   5,  15,  20,  20,  15,   5, -30,
    -30,   0,  15,  20,  20,  15,   0, -30,
    -30,   5,  10,  15,  15,  10,   5, -30,
    -40, -20,   0,   5,   5,   0, -20, -40,
    -50, -40, -30, -30, -30, -30, -40, -50
};

static const int BISHOP_PSQT[64] = {
    -20, -10, -10, -10, -10, -10, -10, -20,
    -10,   0,   0,   0,   0,   0,   0, -10,
    -10,   0,   5,  10,  10,   5,   0, -10,
    -10,   5,   5,  10,  10,   5,   5, -10,
    -10,   0,  10,  10,  10,  10,   0, -10,
    -10,  10,  10,  10,  10,  10,  10, -10,
    -10,   5,   0,   0,   0,   0,   5, -10,
    -20, -10, -10, -10, -10, -10, -10, -20
};

static const int ROOK_PSQT[64] = {
      0,   0,   0,   0,   0,   0,   0,   0,
      5,  10,  10,  10,  10,  10,  10,   5,
     -5,   0,   0,   0,   0,   0,   0,  -5,
     -5,   0,   0,   0,   0,   0,   0,  -5,
     -5,   0,   0,   0,   0,   0,   0,  -5,
     -5,   0,   0,   0,   0,   0,   0,  -5,
     -5,   0,   0,   0,   0,   0,   0,  -5,
      0,   0,   0,   5,   5,   0,   0,   0
};

static const int QUEEN_PSQT[64] = {
    -20, -10, -10,  -5,  -5, -10, -10, -20,
    -10,   0,   0,   0,   0,   0,   0, -10,
    -10,   0,   5,   5,   5,   5,   0, -10,
     -5,   0,   5,   5,   5,   5,   0,  -5,
      0,   0,   5,   5,   5,   5,   0,  -5,
    -10,   5,   5,   5,   5,   5,   0, -10,
    -10,   0,   5,   0,   0,   0,   0, -10,
    -20, -10, -10,  -5,  -5, -10, -10, -20
};

static const int KING_PSQT[64] = {
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -20, -30, -30, -40, -40, -30, -30, -20,
    -10, -20, -20, -20, -20, -20, -20, -10,
     20,  20,   0,   0,   0,   0,  20,  20,
     20,  30,  10,   0,   0,  10,  30,  20
};

static int mg_value(PieceType pt) {
    return PIECE_VALUES[pt];
}

static int piece_psqt(Piece piece, Square sq) {
    int pt = piece_type(piece);
    int color = piece_color(piece);
    
    const int* table;
    switch (pt) {
        case PAWN:   table = PAWN_PSQT; break;
        case KNIGHT: table = KNIGHT_PSQT; break;
        case BISHOP: table = BISHOP_PSQT; break;
        case ROOK:   table = ROOK_PSQT; break;
        case QUEEN:  table = QUEEN_PSQT; break;
        case KING:   table = KING_PSQT; break;
        default: return 0;
    }
    
    int idx = color == ColorWhite ? sq : (sq ^ 56);
    return table[idx];
}

int evaluate(const Board* board) {
    int mgScore = 0;
    
    for (Square sq = 0; sq < 64; sq++) {
        Piece piece = board->squares[sq];
        if (piece == NO_PIECE) continue;
        
        int pt = piece_type(piece);
        int color = piece_color(piece);
        int sign = color == ColorWhite ? 1 : -1;
        
        mgScore += sign * mg_value(pt);
        mgScore += sign * piece_psqt(piece, sq);
    }
    
    return mgScore;
}
