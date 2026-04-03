#ifndef HISTORY_H
#define HISTORY_H

#include "../types.h"

#define HISTORY_MAX_DEPTH 128
#define HISTORY_BONUS_MAX 16384
#define HISTORY_BONUS_MIN -16384
#define HISTORY_BONUS_DEPTH_FACTOR 32

typedef struct {
    int16_t history[COLOR_NB][SQUARE_NB][PIECE_NB][SQUARE_NB];
    int16_t continuation[COLOR_NB][SQUARE_NB][PIECE_NB][SQUARE_NB];
} HistoryTable;

void history_init(HistoryTable* ht);
int16_t history_get(HistoryTable* ht, Color color, Square from, Piece piece, Square to);
void history_update(HistoryTable* ht, Color color, Square from, Piece piece, Square to, int bonus);
void history_clear(HistoryTable* ht);
void history_clear_depth(HistoryTable* ht, Color color, int depth);
Move history_best_move(HistoryTable* ht, Color color, Move* moves, int count);
int history_reduction(int historyValue);
void history_update_continuation(HistoryTable* ht, Color color, Square from, Piece piece, Square to, int bonus);
int16_t history_get_continuation(HistoryTable* ht, Color color, Square from, Piece piece, Square to);

#endif
