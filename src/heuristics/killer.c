#include "killer.h"
#include <string.h>

#define MOVE_NONE_VALUE 0

void killer_init(KillerTable* kt) {
    memset(kt->killerMoves, 0, sizeof(kt->killerMoves));
    memset(kt->killerCounts, 0, sizeof(kt->killerCounts));
}

void killer_add(KillerTable* kt, int depth, Move move) {
    if (move == MOVE_NONE_VALUE) return;

    if (kt->killerMoves[depth][0] == move) return;

    if (kt->killerMoves[depth][0] == MOVE_NONE_VALUE) {
        kt->killerMoves[depth][0] = move;
        kt->killerCounts[depth]++;
    } else if (kt->killerMoves[depth][1] == MOVE_NONE_VALUE) {
        kt->killerMoves[depth][1] = move;
        kt->killerCounts[depth]++;
    }
}

void killer_get(KillerTable* kt, int depth, Move* moves) {
    moves[0] = kt->killerMoves[depth][0];
    moves[1] = kt->killerMoves[depth][1];
}

void killer_clear(KillerTable* kt) {
    memset(kt->killerMoves, 0, sizeof(kt->killerMoves));
    memset(kt->killerCounts, 0, sizeof(kt->killerCounts));
}

void killer_clear_depth(KillerTable* kt, int depth) {
    kt->killerMoves[depth][0] = MOVE_NONE_VALUE;
    kt->killerMoves[depth][1] = MOVE_NONE_VALUE;
    kt->killerCounts[depth] = 0;
}

bool killer_is_killer(KillerTable* kt, int depth, Move move) {
    return move == kt->killerMoves[depth][0] || move == kt->killerMoves[depth][1];
}

void killer_order_moves(KillerTable* kt, int depth, Move* moves, int count) {
    Move killer1 = kt->killerMoves[depth][0];
    Move killer2 = kt->killerMoves[depth][1];

    int writeIdx = 0;

    if (killer1 != MOVE_NONE_VALUE) {
        for (int i = 0; i < count; i++) {
            if (moves[i] == killer1) {
                Move tmp = moves[writeIdx];
                moves[writeIdx] = moves[i];
                moves[i] = tmp;
                writeIdx++;
                break;
            }
        }
    }

    if (killer2 != MOVE_NONE_VALUE && writeIdx < count) {
        for (int i = 0; i < count; i++) {
            if (moves[i] == killer2) {
                Move tmp = moves[writeIdx];
                moves[writeIdx] = moves[i];
                moves[i] = tmp;
                writeIdx++;
                break;
            }
        }
    }
}
