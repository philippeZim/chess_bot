#include "evaluate.h"
#include "pawn_table.h"
#include "../board/bitboard.h"
#include <stdlib.h>

static const int PHASE_MG[PIECE_TYPE_NB] = {
    [PAWN] = 0,
    [KNIGHT] = 1,
    [BISHOP] = 1,
    [ROOK] = 2,
    [QUEEN] = 4,
    [KING] = 0
};

static const int MAX_PHASE = 24;
static const int TEMPO_BONUS = 12;

int compute_game_phase(const Board* board) {
    int phase = 0;

    Bitboard knights = board->byType[KNIGHT];
    Bitboard bishops = board->byType[BISHOP];
    Bitboard rooks = board->byType[ROOK];
    Bitboard queens = board->byType[QUEEN];

    phase += bitboard_popcount(knights) * PHASE_MG[KNIGHT];
    phase += bitboard_popcount(bishops) * PHASE_MG[BISHOP];
    phase += bitboard_popcount(rooks) * PHASE_MG[ROOK];
    phase += bitboard_popcount(queens) * PHASE_MG[QUEEN];

    if (phase > MAX_PHASE) phase = MAX_PHASE;
    return phase;
}

int blend_scores(int mg, int eg, int phase) {
    return (mg * phase + eg * (MAX_PHASE - phase)) / MAX_PHASE;
}

EvalResult evaluate_full(const Board* board) {
    EvalResult result;
    result.mg_score = 0;
    result.eg_score = 0;
    result.game_phase = 0;

    if (!board) return result;

    for (Square sq = 0; sq < 64; sq++) {
        Piece piece = board->squares[sq];
        if (piece == NO_PIECE) continue;

        PieceType pt = piece_type(piece);
        Color color = piece_color(piece);
        int sign = color == ColorWhite ? 1 : -1;

        result.mg_score += sign * piece_value_mg(pt);
        result.eg_score += sign * piece_value_eg(pt);
    }

    EvalScore whitePsqt = eval_psqt(board, ColorWhite);
    EvalScore blackPsqt = eval_psqt(board, ColorBlack);
    result.mg_score += whitePsqt.mg - blackPsqt.mg;
    result.eg_score += whitePsqt.eg - blackPsqt.eg;

    EvalScore whiteOutposts = eval_knight_outposts(board, ColorWhite);
    EvalScore blackOutposts = eval_knight_outposts(board, ColorBlack);
    result.mg_score += whiteOutposts.mg - blackOutposts.mg;
    result.eg_score += whiteOutposts.eg - blackOutposts.eg;

    EvalScore whiteBishopPair = eval_bishop_pair(board, ColorWhite);
    EvalScore blackBishopPair = eval_bishop_pair(board, ColorBlack);
    result.mg_score += whiteBishopPair.mg - blackBishopPair.mg;
    result.eg_score += whiteBishopPair.eg - blackBishopPair.eg;

    EvalScore whiteRookAct = eval_rook_activity(board, ColorWhite);
    EvalScore blackRookAct = eval_rook_activity(board, ColorBlack);
    result.mg_score += whiteRookAct.mg - blackRookAct.mg;
    result.eg_score += whiteRookAct.eg - blackRookAct.eg;

    EvalScore whiteKingSafety = eval_king_safety_psqt(board, ColorWhite);
    EvalScore blackKingSafety = eval_king_safety_psqt(board, ColorBlack);
    result.mg_score += whiteKingSafety.mg - blackKingSafety.mg;
    result.eg_score += whiteKingSafety.eg - blackKingSafety.eg;

    EvalScore pawnScore = eval_pawns(board);
    result.mg_score += pawnScore.mg;
    result.eg_score += pawnScore.eg;

    result.game_phase = compute_game_phase(board);

    if (board->turn == ColorWhite) {
        result.mg_score += TEMPO_BONUS;
        result.eg_score += TEMPO_BONUS;
    } else {
        result.mg_score -= TEMPO_BONUS;
        result.eg_score -= TEMPO_BONUS;
    }

    return result;
}

int evaluate(const Board* board) {
    if (!board) return 0;

    EvalResult result = evaluate_full(board);
    int score = blend_scores(result.mg_score, result.eg_score, result.game_phase);

    return board->turn == ColorWhite ? score : -score;
}

int evaluate_with_nnue(const Board* board, NNUEContext* ctx) {
    if (!board) return 0;
    if (!ctx || !ctx->loaded) return evaluate(board);

    EvalResult result = evaluate_full(board);
    int classicalScore = blend_scores(result.mg_score, result.eg_score, result.game_phase);

    int nnueScore = nnue_evaluate(board, ctx);

    int blended = (classicalScore * 3 + nnueScore * 7) / 10;

    return board->turn == ColorWhite ? blended : -blended;
}
