#include "pawn_table.h"
#include "../board/bitboard.h"

static PawnEntry PawnTable[PAWN_TABLE_SIZE];

static const int DOUBLED_PAWN_MG = -15;
static const int DOUBLED_PAWN_EG = -20;
static const int ISOLATED_PAWN_MG = -20;
static const int ISOLATED_PAWN_EG = -25;
static const int BACKWARD_PAWN_MG = -10;
static const int BACKWARD_PAWN_EG = -15;

static const int PASSED_PAWN_MG[8] = {0, 10, 20, 35, 60, 100, 150, 200};
static const int PASSED_PAWN_EG[8] = {0, 15, 30, 50, 80, 130, 200, 300};

static const int CONNECTED_PAWN_MG = 8;
static const int CONNECTED_PAWN_EG = 12;

static const int CANDIDATE_PASSER_MG = 5;
static const int CANDIDATE_PASSER_EG = 10;

void pawn_table_init(void) {
    for (int i = 0; i < PAWN_TABLE_SIZE; i++) {
        PawnTable[i].key = 0;
        PawnTable[i].score_mg = 0;
        PawnTable[i].score_eg = 0;
        PawnTable[i].passed_pawns[ColorWhite] = 0;
        PawnTable[i].passed_pawns[ColorBlack] = 0;
        PawnTable[i].pawn_chains[ColorWhite] = 0;
        PawnTable[i].pawn_chains[ColorBlack] = 0;
        PawnTable[i].king_shelter[ColorWhite] = 0;
        PawnTable[i].king_shelter[ColorBlack] = 0;
        PawnTable[i].passed_pawn_count[ColorWhite] = 0;
        PawnTable[i].passed_pawn_count[ColorBlack] = 0;
    }
}

static uint64_t pawn_hash_key(const Board* board) {
    Bitboard whitePawns = board->byType[PAWN] & board->byColor[ColorWhite];
    Bitboard blackPawns = board->byType[PAWN] & board->byColor[ColorBlack];
    return whitePawns ^ (blackPawns << 1) ^ (blackPawns >> 63);
}

static EvalScore evaluate_pawn_structure(const Board* board, Color color) {
    EvalScore result = SCORE_ZERO;
    Bitboard pawns = board->byType[PAWN] & board->byColor[color];
    Bitboard enemyPawns = board->byType[PAWN] & board->byColor[opposite_color(color)];
    Bitboard ourPawns = pawns;

    Bitboard passedPawns = 0;
    Bitboard pawnChains = 0;
    uint8_t passedCount = 0;

    while (pawns) {
        Square sq = bitboard_next_square(&pawns);
        int file = square_file(sq);
        int rank = square_rank(sq);

        bool isDoubled = false;
        bool isIsolated = true;
        bool isPassed = true;
        bool isConnected = false;

        Bitboard sqBB = bitboard_from_square(sq);

        Bitboard sameFile = BB_FILE(file);
        if ((sameFile & ourPawns & ~sqBB) != 0) {
            isDoubled = true;
            result.mg += DOUBLED_PAWN_MG;
            result.eg += DOUBLED_PAWN_EG;
        }

        Bitboard adjacentFiles = 0;
        if (file > FILE_A) adjacentFiles |= BB_FILE(file - 1);
        if (file < FILE_H) adjacentFiles |= BB_FILE(file + 1);

        if ((adjacentFiles & ourPawns) != 0) {
            isIsolated = false;
        }

        if (isIsolated) {
            result.mg += ISOLATED_PAWN_MG;
            result.eg += ISOLATED_PAWN_EG;
        }

        int forward = color == ColorWhite ? 1 : -1;
        bool hasPawnBehind = false;
        int rankBehind = rank - forward;
        if (rankBehind >= 0 && rankBehind < 8) {
            if (BB_RANK(rankBehind) & adjacentFiles & ourPawns) {
                hasPawnBehind = true;
            }
        }

        bool isBlocked = false;
        int rankAhead = rank + forward;
        if (rankAhead >= 0 && rankAhead < 8) {
            if (BB_RANK(rankAhead) & BB_FILE(file) & enemyPawns) {
                isBlocked = true;
            }
        }

        if (!hasPawnBehind && isBlocked && !isIsolated) {
            result.mg += BACKWARD_PAWN_MG;
            result.eg += BACKWARD_PAWN_EG;
        }

        for (int r = rank + forward;
             (forward > 0 && r < 8) || (forward < 0 && r >= 0);
             r += forward) {
            Bitboard aheadRank = BB_RANK(r);
            if (aheadRank & BB_FILE(file) & enemyPawns) {
                isPassed = false;
                break;
            }
            if (file > FILE_A && (aheadRank & BB_FILE(file - 1) & enemyPawns)) {
                isPassed = false;
                break;
            }
            if (file < FILE_H && (aheadRank & BB_FILE(file + 1) & enemyPawns)) {
                isPassed = false;
                break;
            }
        }

        if (isPassed) {
            passedPawns |= sqBB;
            passedCount++;
            int relRank = color == ColorWhite ? rank : (7 - rank);
            result.mg += PASSED_PAWN_MG[relRank];
            result.eg += PASSED_PAWN_EG[relRank];
        }

        if (file > FILE_A && (BB_FILE(file - 1) & ourPawns)) {
            isConnected = true;
        }
        if (file < FILE_H && (BB_FILE(file + 1) & ourPawns)) {
            isConnected = true;
        }

        if (isConnected) {
            pawnChains |= sqBB;
            result.mg += CONNECTED_PAWN_MG;
            result.eg += CONNECTED_PAWN_EG;
        }

        if (!isPassed && !isDoubled && !isIsolated) {
            bool canBecomePasser = false;
            int pushRank = rank + forward;
            if (pushRank >= 0 && pushRank < 8) {
                Bitboard pushSq = BB_RANK(pushRank) & BB_FILE(file);
                if ((pushSq & enemyPawns) == 0) {
                    canBecomePasser = true;
                }
            }
            if (canBecomePasser) {
                result.mg += CANDIDATE_PASSER_MG;
                result.eg += CANDIDATE_PASSER_EG;
            }
        }
    }

    return result;
}

EvalScore eval_pawns(const Board* board) {
    uint64_t key = pawn_hash_key(board);
    uint32_t index = key & (PAWN_TABLE_SIZE - 1);

    if (PawnTable[index].key == key) {
        EvalScore result;
        result.mg = PawnTable[index].score_mg;
        result.eg = PawnTable[index].score_eg;
        return result;
    }

    EvalScore whiteScore = evaluate_pawn_structure(board, ColorWhite);
    EvalScore blackScore = evaluate_pawn_structure(board, ColorBlack);

    EvalScore result;
    result.mg = whiteScore.mg - blackScore.mg;
    result.eg = whiteScore.eg - blackScore.eg;

    PawnTable[index].key = key;
    PawnTable[index].score_mg = (int16_t)result.mg;
    PawnTable[index].score_eg = (int16_t)result.eg;

    return result;
}
