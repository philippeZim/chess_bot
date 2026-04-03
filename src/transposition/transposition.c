#include "transposition.h"
#include "zobrist.h"
#include <string.h>
#include <stdlib.h>

bool tt_alloc(TranspositionTable* tt) {
    if (!tt) return false;
    tt->table = (TranspositionEntry*)malloc(sizeof(TranspositionEntry) * TT_SIZE);
    if (!tt->table) return false;
    tt->size = TT_SIZE;
    memset(tt->table, 0, sizeof(TranspositionEntry) * TT_SIZE);
    return true;
}

void tt_free(TranspositionTable* tt) {
    if (tt && tt->table) {
        free(tt->table);
        tt->table = NULL;
        tt->size = 0;
    }
}

void tt_init(TranspositionTable* tt) {
    if (tt) {
        memset(tt->table, 0, sizeof(TranspositionEntry) * tt->size);
    }
}

void tt_clear(TranspositionTable* tt) {
    if (tt) {
        memset(tt->table, 0, sizeof(TranspositionEntry) * tt->size);
    }
}

TranspositionEntry* tt_probe(TranspositionTable* tt, uint64_t key) {
    if (!tt) return NULL;
    
    uint32_t index = (uint32_t)(key & 0xFFFFFFFF) % tt->size;
    
    if (tt->table[index].key == key) {
        return &tt->table[index];
    }
    
    return NULL;
}

void tt_store(TranspositionTable* tt, uint64_t key, uint16_t depth, int16_t score, uint16_t flag, Move best_move) {
    if (!tt) return;
    
    uint32_t index = (uint32_t)(key & 0xFFFFFFFF) % tt->size;
    TranspositionEntry* entry = &tt->table[index];
    
    if (entry->key != key || entry->depth < depth) {
        entry->key = key;
        entry->depth = depth;
        entry->score = score;
        entry->flag = flag;
        entry->best_move = best_move;
    }
}

Move tt_get_move(TranspositionTable* tt, uint64_t key) {
    TranspositionEntry* entry = tt_probe(tt, key);
    
    if (entry && entry->best_move) {
        return entry->best_move;
    }
    
    return 0;
}

uint64_t tt_get_usage(TranspositionTable* tt) {
    if (!tt) return 0;
    
    uint64_t used = 0;
    for (int i = 0; i < tt->size; i++) {
        if (tt->table[i].key != 0) {
            used++;
        }
    }
    return used;
}
