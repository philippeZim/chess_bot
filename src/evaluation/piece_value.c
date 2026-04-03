#include "piece_value.h"
#include "../board/bitboard.h"

static const int PIECE_VALUES_MG[PIECE_TYPE_NB] = {
    [PAWN] = 100,
    [KNIGHT] = 320,
    [BISHOP] = 330,
    [ROOK] = 500,
    [QUEEN] = 900,
    [KING] = 0
};

static const int PIECE_VALUES_EG[PIECE_TYPE_NB] = {
    [PAWN] = 100,
    [KNIGHT] = 340,
    [BISHOP] = 340,
    [ROOK] = 550,
    [QUEEN] = 1000,
    [KING] = 0
};

static const int PAWN_PSQT_MG[64] = {
      0,   0,   0,   0,   0,   0,  0,   0,
     50,  50,  50,  50,  50,  50, 50,  50,
     10,  10,  20,  30,  30,  20, 10,  10,
      5,   5,  10,  25,  25,  10,  5,   5,
      0,   0,   0,  20,  20,   0,  0,   0,
      5,  -5, -10,   0,   0, -10, -5,   5,
      5,  10,  10, -20, -20,  10, 10,   5,
      0,   0,   0,   0,   0,   0,  0,   0
};

static const int PAWN_PSQT_EG[64] = {
      0,   0,   0,   0,   0,   0,  0,   0,
     80,  80,  80,  80,  80,  80, 80,  80,
     20,  20,  30,  40,  40,  30, 20,  20,
     10,  10,  15,  30,  30,  15, 10,  10,
      5,   5,  10,  25,  25,  10,  5,   5,
      0,   0,   0,  10,  10,   0,  0,   0,
      0,   5,   5, -10, -10,   5,  5,   0,
      0,   0,   0,   0,   0,   0,  0,   0
};

static const int KNIGHT_PSQT_MG[64] = {
    -50, -40, -30, -30, -30, -30, -40, -50,
    -40, -20,   0,   0,   0,   0, -20, -40,
    -30,   0,  10,  15,  15,  10,   0, -30,
    -30,   5,  15,  20,  20,  15,   5, -30,
    -30,   0,  15,  20,  20,  15,   0, -30,
    -30,   5,  10,  15,  15,  10,   5, -30,
    -40, -20,   0,   5,   5,   0, -20, -40,
    -50, -40, -30, -30, -30, -30, -40, -50
};

static const int KNIGHT_PSQT_EG[64] = {
    -30, -20, -15, -10, -10, -15, -20, -30,
    -20, -10,  -5,   0,   0,  -5, -10, -20,
    -15,  -5,   5,  10,  10,   5,  -5, -15,
    -10,   0,  10,  15,  15,  10,   0, -10,
    -10,   0,  10,  15,  15,  10,   0, -10,
    -15,  -5,   5,  10,  10,   5,  -5, -15,
    -20, -10,  -5,   0,   0,  -5, -10, -20,
    -30, -20, -15, -10, -10, -15, -20, -30
};

static const int BISHOP_PSQT_MG[64] = {
    -20, -10, -10, -10, -10, -10, -10, -20,
    -10,   0,   0,   0,   0,   0,   0, -10,
    -10,   0,   5,  10,  10,   5,   0, -10,
    -10,   5,   5,  10,  10,   5,   5, -10,
    -10,   0,  10,  10,  10,  10,   0, -10,
    -10,  10,  10,  10,  10,  10,  10, -10,
    -10,   5,   0,   0,   0,   0,   5, -10,
    -20, -10, -10, -10, -10, -10, -10, -20
};

static const int BISHOP_PSQT_EG[64] = {
    -10,  -5,  -5,  -5,  -5,  -5,  -5, -10,
     -5,   0,   0,   0,   0,   0,   0,  -5,
     -5,   0,   5,   5,   5,   5,   0,  -5,
     -5,   0,   5,  10,  10,   5,   0,  -5,
     -5,   0,   5,  10,  10,   5,   0,  -5,
     -5,   5,   5,   5,   5,   5,   5,  -5,
     -5,   0,   0,   0,   0,   0,   0,  -5,
    -10,  -5,  -5,  -5,  -5,  -5,  -5, -10
};

static const int ROOK_PSQT_MG[64] = {
      0,   0,   0,   0,   0,   0,   0,   0,
      5,  10,  10,  10,  10,  10,  10,   5,
     -5,   0,   0,   0,   0,   0,   0,  -5,
     -5,   0,   0,   0,   0,   0,   0,  -5,
     -5,   0,   0,   0,   0,   0,   0,  -5,
     -5,   0,   0,   0,   0,   0,   0,  -5,
     -5,   0,   0,   0,   0,   0,   0,  -5,
      0,   0,   0,   5,   5,   0,   0,   0
};

static const int ROOK_PSQT_EG[64] = {
      0,   0,   0,   5,   5,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   5,  10,  10,   5,   0,   0,
      0,   0,   5,  10,  10,   5,   0,   0,
      0,   0,   5,  10,  10,   5,   0,   0,
      0,   0,   5,  10,  10,   5,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   5,   5,   0,   0,   0
};

static const int QUEEN_PSQT_MG[64] = {
    -20, -10, -10,  -5,  -5, -10, -10, -20,
    -10,   0,   0,   0,   0,   0,   0, -10,
    -10,   0,   5,   5,   5,   5,   0, -10,
     -5,   0,   5,   5,   5,   5,   0,  -5,
      0,   0,   5,   5,   5,   5,   0,  -5,
    -10,   5,   5,   5,   5,   5,   0, -10,
    -10,   0,   5,   0,   0,   0,   0, -10,
    -20, -10, -10,  -5,  -5, -10, -10, -20
};

static const int QUEEN_PSQT_EG[64] = {
    -10,  -5,  -5,  -5,  -5,  -5,  -5, -10,
     -5,   0,   0,   0,   0,   0,   0,  -5,
     -5,   0,   5,   5,   5,   5,   0,  -5,
     -5,   0,   5,   5,   5,   5,   0,  -5,
     -5,   0,   5,   5,   5,   5,   0,  -5,
     -5,   0,   5,   5,   5,   5,   0,  -5,
     -5,   0,   0,   0,   0,   0,   0,  -5,
    -10,  -5,  -5,  -5,  -5,  -5,  -5, -10
};

static const int KING_PSQT_MG[64] = {
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -30, -40, -40, -50, -50, -40, -40, -30,
    -20, -30, -30, -40, -40, -30, -30, -20,
    -10, -20, -20, -20, -20, -20, -20, -10,
     20,  20,   0,   0,   0,   0,  20,  20,
     20,  30,  10,   0,   0,  10,  30,  20
};

static const int KING_PSQT_EG[64] = {
    -20, -10,   0,   5,   5,   0, -10, -20,
    -10,   0,  10,  15,  15,  10,   0, -10,
      0,  10,  20,  25,  25,  20,  10,   0,
      5,  15,  25,  30,  30,  25,  15,   5,
      5,  15,  25,  30,  30,  25,  15,   5,
      0,  10,  20,  25,  25,  20,  10,   0,
    -10,   0,  10,  15,  15,  10,   0, -10,
    -20, -10,   0,   5,   5,   0, -10, -20
};

static const int* PSQT_MG[PIECE_TYPE_NB] = {
    NULL, PAWN_PSQT_MG, KNIGHT_PSQT_MG, BISHOP_PSQT_MG,
    ROOK_PSQT_MG, QUEEN_PSQT_MG, KING_PSQT_MG
};

static const int* PSQT_EG[PIECE_TYPE_NB] = {
    NULL, PAWN_PSQT_EG, KNIGHT_PSQT_EG, BISHOP_PSQT_EG,
    ROOK_PSQT_EG, QUEEN_PSQT_EG, KING_PSQT_EG
};

static const int OUTPOST_BONUS_MG = 25;
static const int OUTPOST_BONUS_EG = 15;
static const int BISHOP_PAIR_MG = 30;
static const int BISHOP_PAIR_EG = 50;
static const int ROOK_OPEN_FILE_MG = 20;
static const int ROOK_OPEN_FILE_EG = 30;
static const int ROOK_SEMI_OPEN_FILE_MG = 10;
static const int ROOK_SEMI_OPEN_FILE_EG = 15;

int piece_value_mg(PieceType pt) {
    return PIECE_VALUES_MG[pt];
}

int piece_value_eg(PieceType pt) {
    return PIECE_VALUES_EG[pt];
}

static int psqt_index(Square sq, Color color) {
    return color == ColorWhite ? sq : (sq ^ 56);
}

EvalScore eval_psqt(const Board* board, Color color) {
    EvalScore result = SCORE_ZERO;
    Bitboard pieces = board->byColor[color];

    while (pieces) {
        Square sq = bitboard_next_square(&pieces);
        Piece piece = board->squares[sq];
        if (piece == NO_PIECE) continue;

        PieceType pt = piece_type(piece);
        int idx = psqt_index(sq, color);

        result.mg += PSQT_MG[pt][idx];
        result.eg += PSQT_EG[pt][idx];
    }

    return result;
}

EvalScore eval_knight_outposts(const Board* board, Color color) {
    EvalScore result = SCORE_ZERO;
    Bitboard knights = board->byType[KNIGHT] & board->byColor[color];
    Bitboard enemyPawns = board->byType[PAWN] & board->byColor[opposite_color(color)];
    Bitboard ourPawns = board->byType[PAWN] & board->byColor[color];

    while (knights) {
        Square sq = bitboard_next_square(&knights);
        int rank = square_rank(sq);
        int file = square_file(sq);

        if (rank < 2 || rank > 5) continue;

        bool defended = false;
        Bitboard pawnAttacks = pawn_attacks(color, sq);
        if (pawnAttacks & ourPawns) {
            defended = true;
        }

        bool shielded = false;
        int forward = color == ColorWhite ? 1 : -1;
        int rankAhead = rank + forward;
        if (rankAhead >= 0 && rankAhead < 8) {
            Bitboard aheadRank = BB_RANK(rankAhead);
            if (file > FILE_A) {
                if (aheadRank & BB_FILE(file - 1) & ourPawns) shielded = true;
            }
            if (aheadRank & BB_FILE(file) & ourPawns) shielded = true;
            if (file < FILE_H) {
                if (aheadRank & BB_FILE(file + 1) & ourPawns) shielded = true;
            }
        }

        bool noEnemyPawn = true;
        int enemyForward = opposite_color(color) == ColorWhite ? 1 : -1;
        for (int r = rank + enemyForward;
             (enemyForward > 0 && r < 8) || (enemyForward < 0 && r >= 0);
             r += enemyForward) {
            if (BB_RANK(r) & BB_FILE(file) & enemyPawns) {
                noEnemyPawn = false;
                break;
            }
        }

        if (defended && shielded && noEnemyPawn) {
            result.mg += OUTPOST_BONUS_MG;
            result.eg += OUTPOST_BONUS_EG;
        } else if (defended && noEnemyPawn) {
            result.mg += OUTPOST_BONUS_MG / 2;
            result.eg += OUTPOST_BONUS_EG / 2;
        }
    }

    return result;
}

EvalScore eval_bishop_pair(const Board* board, Color color) {
    EvalScore result = SCORE_ZERO;
    Bitboard bishops = board->byType[BISHOP] & board->byColor[color];

    if (bitboard_popcount(bishops) >= 2) {
        bool lightSquare = false;
        bool darkSquare = false;

        while (bishops) {
            Square sq = bitboard_next_square(&bishops);
            int file = square_file(sq);
            int rank = square_rank(sq);
            if ((file + rank) % 2 == 0) {
                darkSquare = true;
            } else {
                lightSquare = true;
            }
        }

        if (lightSquare && darkSquare) {
            result.mg += BISHOP_PAIR_MG;
            result.eg += BISHOP_PAIR_EG;
        }
    }

    return result;
}

EvalScore eval_rook_activity(const Board* board, Color color) {
    EvalScore result = SCORE_ZERO;
    Bitboard rooks = board->byType[ROOK] & board->byColor[color];
    Bitboard enemyPawns = board->byType[PAWN] & board->byColor[opposite_color(color)];
    Bitboard ourPawns = board->byType[PAWN] & board->byColor[color];

    while (rooks) {
        Square sq = bitboard_next_square(&rooks);
        int file = square_file(sq);

        bool openFile = (BB_FILE(file) & ourPawns) == 0;
        bool semiOpen = (BB_FILE(file) & ourPawns) == 0 && (BB_FILE(file) & enemyPawns) != 0;

        if (openFile) {
            result.mg += ROOK_OPEN_FILE_MG;
            result.eg += ROOK_OPEN_FILE_EG;
        } else if (semiOpen) {
            result.mg += ROOK_SEMI_OPEN_FILE_MG;
            result.eg += ROOK_SEMI_OPEN_FILE_EG;
        }

        int rank = square_rank(sq);
        if (color == ColorWhite && rank == RANK_7) {
            result.mg += 15;
            result.eg += 20;
        } else if (color == ColorBlack && rank == RANK_2) {
            result.mg += 15;
            result.eg += 20;
        }
    }

    return result;
}

EvalScore eval_king_safety_psqt(const Board* board, Color color) {
    EvalScore result = SCORE_ZERO;
    Square kingSq = SQUARE_NB;

    Bitboard kings = board->byType[KING] & board->byColor[color];
    if (kings) {
        kingSq = bitboard_next_square(&kings);
    }

    if (kingSq == SQUARE_NB) return result;

    int kingFile = square_file(kingSq);
    int kingRank = square_rank(kingSq);

    Bitboard ourPawns = board->byType[PAWN] & board->byColor[color];

    int shelterBonus = 0;
    int forward = color == ColorWhite ? 1 : -1;

    for (int df = -1; df <= 1; df++) {
        int f = kingFile + df;
        if (f < 0 || f >= FILE_NB) continue;

        for (int dr = 1; dr <= 2; dr++) {
            int r = kingRank + forward * dr;
            if (r < 0 || r >= RANK_NB) continue;

            if (BB_RANK(r) & BB_FILE(f) & ourPawns) {
                shelterBonus += (dr == 1) ? 10 : 5;
            }
        }
    }

    result.mg += shelterBonus;
    result.eg += shelterBonus / 2;

    return result;
}
