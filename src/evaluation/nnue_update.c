#include "nnue_update.h"
#include "../board/bitboard.h"
#include <string.h>

int nnue_feature_index(Color side, Square king_sq, Square piece_sq, Piece piece) {
    (void)king_sq;
    if (piece == NO_PIECE) return -1;
    Color c = piece_color(piece);
    PieceType pt = piece_type(piece);

    int color_offset = (c == side) ? 0 : NNUE_FEATURES_PER_COLOR;
    int piece_offset = (pt - 1) * 64;

    int sq = (c == side) ? piece_sq : (piece_sq ^ 56);

    return color_offset + piece_offset + sq;
}

void nnue_state_init(NNUEState* state) {
    if (!state) return;
    memset(state, 0, sizeof(NNUEState));
    state->current_ply = 0;
}

void nnue_state_reset(NNUEState* state) {
    if (!state) return;
    memset(state, 0, sizeof(NNUEState));
    state->current_ply = 0;
}

void nnue_state_push(NNUEState* state) {
    if (!state) return;
    if (state->current_ply < MAX_PLY - 1) {
        state->current_ply++;
    }
}

void nnue_state_pop(NNUEState* state) {
    if (!state) return;
    if (state->current_ply > 0) {
        state->current_ply--;
    }
}

void nnue_full_refresh(const Board* board, const NNUEWeights* weights,
                       Color perspective, Accumulator* acc) {
    if (!board || !weights || !acc) return;

    Square kingSq = SQUARE_NB;
    Bitboard kings = board->byType[KING] & board->byColor[perspective];
    if (kings) {
        kingSq = bitboard_next_square(&kings);
    }

    if (kingSq == SQUARE_NB) {
        acc->computed = false;
        return;
    }

    for (int i = 0; i < NNUE_HIDDEN_SIZE; i++) {
        acc->values[i] = weights->bias_h[i];
    }

    Bitboard allPieces = board->byColor[ColorWhite] | board->byColor[ColorBlack];
    while (allPieces) {
        Square sq = bitboard_next_square(&allPieces);
        Piece piece = board->squares[sq];
        if (piece == NO_PIECE) continue;

        int feature = nnue_feature_index(perspective, kingSq, sq, piece);
        if (feature < 0 || feature >= NNUE_INPUT_SIZE) continue;

        for (int i = 0; i < NNUE_HIDDEN_SIZE; i++) {
            acc->values[i] += weights->weights_ih[feature][i];
        }
    }

    acc->computed = true;
}

static void add_feature(Accumulator* acc, const NNUEWeights* weights, int feature) {
    if (feature < 0 || feature >= NNUE_INPUT_SIZE) return;
    for (int i = 0; i < NNUE_HIDDEN_SIZE; i++) {
        acc->values[i] += weights->weights_ih[feature][i];
    }
}

static void remove_feature(Accumulator* acc, const NNUEWeights* weights, int feature) {
    if (feature < 0 || feature >= NNUE_INPUT_SIZE) return;
    for (int i = 0; i < NNUE_HIDDEN_SIZE; i++) {
        acc->values[i] -= weights->weights_ih[feature][i];
    }
}

void nnue_update_incremental(NNUEState* state, const Board* board,
                             const NNUEWeights* weights, int from, int to,
                             Piece captured) {
    if (!state || !board || !weights) return;

    Square kingSq = SQUARE_NB;
    Bitboard kings = board->byType[KING] & board->byColor[board->turn];
    if (kings) {
        kingSq = bitboard_next_square(&kings);
    }

    if (kingSq == SQUARE_NB) return;

    Piece moving = board->squares[from];
    if (moving == NO_PIECE) return;

    bool kingMoved = (piece_type(moving) == KING);

    for (Color perspective = ColorWhite; perspective < COLOR_NB; perspective++) {
        Accumulator* acc = &state->accumulators[perspective][state->current_ply];

        if (kingMoved && piece_color(moving) == perspective) {
            nnue_full_refresh(board, weights, perspective, acc);
            continue;
        }

        if (!acc->computed) {
            nnue_full_refresh(board, weights, perspective, acc);
            continue;
        }

        int fromFeature = nnue_feature_index(perspective, kingSq, (Square)from, moving);
        int toFeature = nnue_feature_index(perspective, kingSq, (Square)to, moving);

        remove_feature(acc, weights, fromFeature);
        add_feature(acc, weights, toFeature);

        if (captured != NO_PIECE) {
            int capturedFeature = nnue_feature_index(perspective, kingSq, (Square)to, captured);
            remove_feature(acc, weights, capturedFeature);
        }

        acc->computed = true;
    }
}
