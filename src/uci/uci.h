#ifndef UCI_H
#define UCI_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef struct {
    bool stop;
    int depth;
    int movetime;
    int64_t time_white;
    int64_t time_black;
    int movestogo;
} SearchLimits;

void uci_loop(void);
SearchLimits uci_default_limits(void);
bool uci_parse_position(const char* cmd, char* fen, size_t fen_size, char* moves, size_t moves_size);

#endif
