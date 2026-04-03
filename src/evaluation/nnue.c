#include "nnue.h"
#include "../board/bitboard.h"
#include <string.h>

void nnue_context_init(NNUEContext* ctx) {
    if (!ctx) return;
    nnue_init_default_weights(&ctx->weights);
    nnue_state_init(&ctx->state);
    ctx->loaded = false;
}

bool nnue_context_load(NNUEContext* ctx, const char* path) {
    if (!ctx || !path) return false;
    if (nnue_load_weights(path, &ctx->weights)) {
        ctx->loaded = true;
        return true;
    }
    ctx->loaded = false;
    return false;
}

void nnue_context_refresh(NNUEContext* ctx, const Board* board) {
    if (!ctx || !board) return;
    nnue_state_reset(&ctx->state);
    for (Color c = ColorWhite; c < COLOR_NB; c++) {
        nnue_full_refresh(board, &ctx->weights, c,
                          &ctx->state.accumulators[c][0]);
    }
}

static int16_t clip(int16_t val) {
    if (val < NNUE_WEIGHT_CLIP_MIN) return NNUE_WEIGHT_CLIP_MIN;
    if (val > NNUE_WEIGHT_CLIP_MAX) return NNUE_WEIGHT_CLIP_MAX;
    return val;
}

int nnue_evaluate_scalar(const Board* board, const NNUEWeights* weights) {
    if (!board || !weights) return 0;

    Square whiteKingSq = SQUARE_NB;
    Square blackKingSq = SQUARE_NB;

    Bitboard wKings = board->byType[KING] & board->byColor[ColorWhite];
    Bitboard bKings = board->byType[KING] & board->byColor[ColorBlack];

    if (wKings) whiteKingSq = bitboard_next_square(&wKings);
    if (bKings) blackKingSq = bitboard_next_square(&bKings);

    if (whiteKingSq == SQUARE_NB || blackKingSq == SQUARE_NB) return 0;

    int16_t accWhite[NNUE_HIDDEN_SIZE];
    int16_t accBlack[NNUE_HIDDEN_SIZE];

    for (int i = 0; i < NNUE_HIDDEN_SIZE; i++) {
        accWhite[i] = weights->bias_h[i];
        accBlack[i] = weights->bias_h[i];
    }

    Bitboard allPieces = board->byColor[ColorWhite] | board->byColor[ColorBlack];
    while (allPieces) {
        Square sq = bitboard_next_square(&allPieces);
        Piece piece = board->squares[sq];
        if (piece == NO_PIECE) continue;

        int whiteFeature = nnue_feature_index(ColorWhite, whiteKingSq, sq, piece);
        int blackFeature = nnue_feature_index(ColorBlack, blackKingSq, sq, piece);

        if (whiteFeature >= 0 && whiteFeature < NNUE_INPUT_SIZE) {
            for (int i = 0; i < NNUE_HIDDEN_SIZE; i++) {
                accWhite[i] += weights->weights_ih[whiteFeature][i];
            }
        }

        if (blackFeature >= 0 && blackFeature < NNUE_INPUT_SIZE) {
            for (int i = 0; i < NNUE_HIDDEN_SIZE; i++) {
                accBlack[i] += weights->weights_ih[blackFeature][i];
            }
        }
    }

    int16_t output = weights->bias_o;

    for (int i = 0; i < NNUE_HIDDEN_SIZE; i++) {
        int16_t w = clip(accWhite[i]);
        int16_t b = clip(accBlack[i]);
        output += (int16_t)((w * weights->weights_ho[i]) / NNUE_SCALE_FACTOR);
        output += (int16_t)((b * weights->weights_ho[NNUE_HIDDEN_SIZE + i]) / NNUE_SCALE_FACTOR);
    }

    output = (int16_t)(output / NNUE_SCALE_FACTOR);

    return (int)output;
}

int nnue_evaluate(const Board* board, NNUEContext* ctx) {
    if (!board || !ctx || !ctx->loaded) {
        return nnue_evaluate_scalar(board, &ctx->weights);
    }

    Accumulator* acc = &ctx->state.accumulators[board->turn][ctx->state.current_ply];

    if (!acc->computed) {
        nnue_full_refresh(board, &ctx->weights, board->turn, acc);
    }

    int16_t output = ctx->weights.bias_o;

    for (int i = 0; i < NNUE_HIDDEN_SIZE; i++) {
        int16_t val = clip(acc->values[i]);
        output += (int16_t)((val * ctx->weights.weights_ho[i]) / NNUE_SCALE_FACTOR);
    }

    output = (int16_t)(output / NNUE_SCALE_FACTOR);

    return (int)output;
}
