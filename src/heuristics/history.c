#include "history.h"
#include <string.h>
#include <stdlib.h>

void history_init(HistoryTable* ht) {
    memset(ht->history, 0, sizeof(ht->history));
    memset(ht->continuation, 0, sizeof(ht->continuation));
}

int16_t history_get(HistoryTable* ht, Color color, Square from, Piece piece, Square to) {
    return ht->history[color][from][piece][to];
}

void history_update(HistoryTable* ht, Color color, Square from, Piece piece, Square to, int bonus) {
    int16_t* entry = &ht->history[color][from][piece][to];
    int16_t current = *entry;
    int16_t delta = bonus - (current * abs(bonus) / HISTORY_BONUS_MAX);
    *entry = current + delta;
    
    if (*entry > HISTORY_BONUS_MAX) *entry = HISTORY_BONUS_MAX;
    if (*entry < HISTORY_BONUS_MIN) *entry = HISTORY_BONUS_MIN;
}

void history_clear(HistoryTable* ht) {
    memset(ht->history, 0, sizeof(ht->history));
    memset(ht->continuation, 0, sizeof(ht->continuation));
}

void history_clear_depth(HistoryTable* ht, Color color, int depth) {
    Square from, to;
    Piece piece;
    int factor = depth * HISTORY_BONUS_DEPTH_FACTOR;
    
    for (from = 0; from < SQUARE_NB; from++) {
        for (piece = 0; piece < PIECE_NB; piece++) {
            for (to = 0; to < SQUARE_NB; to++) {
                if (ht->history[color][from][piece][to] != 0) {
                    ht->history[color][from][piece][to] = 0;
                }
            }
        }
    }
}

Move history_best_move(HistoryTable* ht, Color color, Move* moves, int count) {
    Move best = moves[0];
    int bestScore = history_get(ht, color, move_from(moves[0]), move_to(moves[0]), move_to(moves[0]));
    
    for (int i = 1; i < count; i++) {
        int score = history_get(ht, color, move_from(moves[i]), move_to(moves[i]), move_to(moves[i]));
        if (score > bestScore) {
            bestScore = score;
            best = moves[i];
        }
    }
    
    return best;
}

int history_reduction(int historyValue) {
    if (historyValue > 0) {
        return -historyValue / 256;
    } else if (historyValue < 0) {
        return (-historyValue) / 256;
    }
    return 0;
}

void history_update_continuation(HistoryTable* ht, Color color, Square from, Piece piece, Square to, int bonus) {
    int16_t* entry = &ht->continuation[color][from][piece][to];
    int16_t current = *entry;
    int16_t delta = bonus - (current * abs(bonus) / HISTORY_BONUS_MAX);
    *entry = current + delta;
    
    if (*entry > HISTORY_BONUS_MAX) *entry = HISTORY_BONUS_MAX;
    if (*entry < HISTORY_BONUS_MIN) *entry = HISTORY_BONUS_MIN;
}

int16_t history_get_continuation(HistoryTable* ht, Color color, Square from, Piece piece, Square to) {
    return ht->continuation[color][from][piece][to];
}
