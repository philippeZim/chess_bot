#include "nnue_io.h"
#include <stdio.h>
#include <string.h>

bool nnue_load_weights(const char* path, NNUEWeights* weights) {
    if (!path || !weights) return false;

    FILE* f = fopen(path, "rb");
    if (!f) return false;

    uint32_t magic = 0;
    uint32_t version = 0;
    uint32_t input_size = 0;
    uint32_t hidden_size = 0;

    if (fread(&magic, sizeof(uint32_t), 1, f) != 1) { fclose(f); return false; }
    if (fread(&version, sizeof(uint32_t), 1, f) != 1) { fclose(f); return false; }
    if (fread(&input_size, sizeof(uint32_t), 1, f) != 1) { fclose(f); return false; }
    if (fread(&hidden_size, sizeof(uint32_t), 1, f) != 1) { fclose(f); return false; }

    if (magic != NNUE_MAGIC) { fclose(f); return false; }
    if (version != NNUE_VERSION) { fclose(f); return false; }
    if (input_size != NNUE_INPUT_SIZE) { fclose(f); return false; }
    if (hidden_size != NNUE_HIDDEN_SIZE) { fclose(f); return false; }

    size_t ih_size = (size_t)NNUE_INPUT_SIZE * NNUE_HIDDEN_SIZE * sizeof(int16_t);
    size_t bias_h_size = NNUE_HIDDEN_SIZE * sizeof(int16_t);
    size_t ho_size = NNUE_HIDDEN_SIZE * 2 * sizeof(int16_t);
    size_t bias_o_size = sizeof(int16_t);

    if (fread(weights->weights_ih, 1, ih_size, f) != ih_size) { fclose(f); return false; }
    if (fread(weights->bias_h, 1, bias_h_size, f) != bias_h_size) { fclose(f); return false; }
    if (fread(weights->weights_ho, 1, ho_size, f) != ho_size) { fclose(f); return false; }
    if (fread(&weights->bias_o, 1, bias_o_size, f) != bias_o_size) { fclose(f); return false; }

    fclose(f);
    return true;
}

bool nnue_save_weights(const char* path, const NNUEWeights* weights) {
    if (!path || !weights) return false;

    FILE* f = fopen(path, "wb");
    if (!f) return false;

    uint32_t magic = NNUE_MAGIC;
    uint32_t version = NNUE_VERSION;
    uint32_t input_size = NNUE_INPUT_SIZE;
    uint32_t hidden_size = NNUE_HIDDEN_SIZE;

    if (fwrite(&magic, sizeof(uint32_t), 1, f) != 1) { fclose(f); return false; }
    if (fwrite(&version, sizeof(uint32_t), 1, f) != 1) { fclose(f); return false; }
    if (fwrite(&input_size, sizeof(uint32_t), 1, f) != 1) { fclose(f); return false; }
    if (fwrite(&hidden_size, sizeof(uint32_t), 1, f) != 1) { fclose(f); return false; }

    size_t ih_size = (size_t)NNUE_INPUT_SIZE * NNUE_HIDDEN_SIZE * sizeof(int16_t);
    size_t bias_h_size = NNUE_HIDDEN_SIZE * sizeof(int16_t);
    size_t ho_size = NNUE_HIDDEN_SIZE * 2 * sizeof(int16_t);
    size_t bias_o_size = sizeof(int16_t);

    if (fwrite(weights->weights_ih, 1, ih_size, f) != ih_size) { fclose(f); return false; }
    if (fwrite(weights->bias_h, 1, bias_h_size, f) != bias_h_size) { fclose(f); return false; }
    if (fwrite(weights->weights_ho, 1, ho_size, f) != ho_size) { fclose(f); return false; }
    if (fwrite(&weights->bias_o, 1, bias_o_size, f) != bias_o_size) { fclose(f); return false; }

    fclose(f);
    return true;
}

void nnue_init_default_weights(NNUEWeights* weights) {
    if (!weights) return;

    uint64_t state = 0xDEADBEEF12345678ULL;
    for (int i = 0; i < NNUE_INPUT_SIZE; i++) {
        for (int j = 0; j < NNUE_HIDDEN_SIZE; j++) {
            state = state * 6364136223846793005ULL + 1442695040888963407ULL;
            int16_t val = (int16_t)((state >> 48) % 255 - 127);
            weights->weights_ih[i][j] = val;
        }
    }

    for (int i = 0; i < NNUE_HIDDEN_SIZE; i++) {
        state = state * 6364136223846793005ULL + 1442695040888963407ULL;
        weights->bias_h[i] = (int16_t)((state >> 48) % 255 - 127);
    }

    for (int i = 0; i < NNUE_HIDDEN_SIZE * 2; i++) {
        state = state * 6364136223846793005ULL + 1442695040888963407ULL;
        weights->weights_ho[i] = (int16_t)((state >> 48) % 255 - 127);
    }

    weights->bias_o = 0;
}
