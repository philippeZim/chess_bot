#ifndef NNUE_IO_H
#define NNUE_IO_H

#include "../types.h"
#include "nnue_arch.h"

typedef struct {
    int16_t weights_ih[NNUE_INPUT_SIZE][NNUE_HIDDEN_SIZE];
    int16_t bias_h[NNUE_HIDDEN_SIZE];
    int16_t weights_ho[NNUE_HIDDEN_SIZE * 2];
    int16_t bias_o;
} NNUEWeights;

bool nnue_load_weights(const char* path, NNUEWeights* weights);
bool nnue_save_weights(const char* path, const NNUEWeights* weights);
void nnue_init_default_weights(NNUEWeights* weights);

#endif
