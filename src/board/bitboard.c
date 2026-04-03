#include "bitboard.h"
#include <string.h>

Bitboard BB_A_FILE = 0x0101010101010101ULL;
Bitboard BB_B_FILE = 0x0202020202020202ULL;
Bitboard BB_C_FILE = 0x0404040404040404ULL;
Bitboard BB_D_FILE = 0x0808080808080808ULL;
Bitboard BB_E_FILE = 0x1010101010101010ULL;
Bitboard BB_F_FILE = 0x2020202020202020ULL;
Bitboard BB_G_FILE = 0x4040404040404040ULL;
Bitboard BB_H_FILE = 0x8080808080808080ULL;

Bitboard BB_1ST_RANK = 0x00000000000000FFULL;
Bitboard BB_2ND_RANK = 0x000000000000FF00ULL;
Bitboard BB_3RD_RANK = 0x0000000000FF0000ULL;
Bitboard BB_4TH_RANK = 0x00000000FF000000ULL;
Bitboard BB_5TH_RANK = 0x000000FFFF000000ULL;
Bitboard BB_6TH_RANK = 0x0000FFFF00000000ULL;
Bitboard BB_7TH_RANK = 0x00FF000000000000ULL;
Bitboard BB_8TH_RANK = 0xFF00000000000000ULL;

Bitboard BB_CENTER = 0x0000001818000000ULL;

static const char square_names[64][3] = {
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"
};

Bitboard bitboard_from_square(Square square) {
    return 1ULL << square;
}

Square bitboard_next_square(Bitboard* bb) {
    if (*bb == 0) return SQUARE_NB;
    Square sq = __builtin_ctzll(*bb);
    *bb &= *bb - 1;
    return sq;
}

int bitboard_popcount(Bitboard bb) {
    return __builtin_popcountll(bb);
}

Bitboard bitboard_pop_lsb(Bitboard bb) {
    return bb & (bb - 1);
}

Bitboard bitboard_lsb(Bitboard bb) {
    return bb & -bb;
}

Bitboard square_attacks_knight(Square square) {
    int r = square_rank(square);
    int f = square_file(square);
    Bitboard attacks = 0;
    
    /* All 8 possible knight moves */
    int dr[] = {-2, -2, -1, -1, 1, 1, 2, 2};
    int df[] = {-1, 1, -2, 2, -2, 2, -1, 1};
    
    for (int i = 0; i < 8; i++) {
        int nr = r + dr[i];
        int nf = f + df[i];
        if (nr >= 0 && nr < 8 && nf >= 0 && nf < 8) {
            attacks |= (1ULL << make_square(nf, nr));
        }
    }
    
    return attacks;
}

Bitboard square_attacks_king(Square square) {
    int r = square_rank(square);
    int f = square_file(square);
    Bitboard attacks = 0;
    
    /* All 8 possible king moves */
    int dr[] = {-1, -1, -1, 0, 0, 1, 1, 1};
    int df[] = {-1, 0, 1, -1, 1, -1, 0, 1};
    
    for (int i = 0; i < 8; i++) {
        int nr = r + dr[i];
        int nf = f + df[i];
        if (nr >= 0 && nr < 8 && nf >= 0 && nf < 8) {
            attacks |= (1ULL << make_square(nf, nr));
        }
    }
    
    return attacks;
}

Bitboard pawn_attacks(Color c, Square square) {
    Bitboard bb = 1ULL << square;
    if (c == ColorWhite) {
        return ((bb >> 7) & ~BB_A_FILE) | ((bb >> 9) & ~BB_H_FILE);
    } else {
        return ((bb << 7) & ~BB_H_FILE) | ((bb << 9) & ~BB_A_FILE);
    }
}

static Bitboard bishop_attacks_body(Square square, Bitboard block) {
    Bitboard attacks = 0;
    int tr = square_rank(square);
    int tf = square_file(square);
    for (int r = tr + 1, f = tf + 1; r < 8 && f < 8; r++, f++) {
        attacks |= 1ULL << make_square(f, r);
        if ((block >> make_square(f, r)) & 1) break;
    }
    for (int r = tr + 1, f = tf - 1; r < 8 && f >= 0; r++, f--) {
        attacks |= 1ULL << make_square(f, r);
        if ((block >> make_square(f, r)) & 1) break;
    }
    for (int r = tr - 1, f = tf + 1; r >= 0 && f < 8; r--, f++) {
        attacks |= 1ULL << make_square(f, r);
        if ((block >> make_square(f, r)) & 1) break;
    }
    for (int r = tr - 1, f = tf - 1; r >= 0 && f >= 0; r--, f--) {
        attacks |= 1ULL << make_square(f, r);
        if ((block >> make_square(f, r)) & 1) break;
    }
    return attacks;
}

static Bitboard rook_attacks_body(Square square, Bitboard block) {
    Bitboard attacks = 0;
    int tr = square_rank(square);
    int tf = square_file(square);
    for (int r = tr + 1; r < 8; r++) {
        attacks |= 1ULL << make_square(tf, r);
        if ((block >> make_square(tf, r)) & 1) break;
    }
    for (int r = tr - 1; r >= 0; r--) {
        attacks |= 1ULL << make_square(tf, r);
        if ((block >> make_square(tf, r)) & 1) break;
    }
    for (int f = tf + 1; f < 8; f++) {
        attacks |= 1ULL << make_square(f, tr);
        if ((block >> make_square(f, tr)) & 1) break;
    }
    for (int f = tf - 1; f >= 0; f--) {
        attacks |= 1ULL << make_square(f, tr);
        if ((block >> make_square(f, tr)) & 1) break;
    }
    return attacks;
}

Bitboard bishop_attacks(Square square, Bitboard block) {
    return bishop_attacks_body(square, block);
}

Bitboard rook_attacks(Square square, Bitboard block) {
    return rook_attacks_body(square, block);
}

Bitboard queen_attacks(Square square, Bitboard block) {
    return bishop_attacks(square, block) | rook_attacks(square, block);
}

const char* square_to_string(Square square) {
    if (square >= SQUARE_NB) return "--";
    return square_names[square];
}

Square string_to_square(const char* str) {
    if (strlen(str) != 2) return SQUARE_NB;
    int file = str[0] - 'a';
    int rank = str[1] - '1';
    if (file < 0 || file > 7 || rank < 0 || rank > 7) return SQUARE_NB;
    return make_square(file, rank);
}

void bitboard_to_string(Bitboard bb, char* buf, size_t bufsize) {
    char* p = buf;
    char* end = buf + bufsize - 1;
    for (int r = 7; r >= 0 && p < end; r--) {
        p += snprintf(p, end - p, "%d ", r + 1);
        for (int f = 0; f < 8 && p < end; f++) {
            Bitboard sq = 1ULL << make_square(f, r);
            p += snprintf(p, end - p, "%c", (bb & sq) ? 'X' : '.');
        }
        p += snprintf(p, end - p, "\n");
    }
    if (p < end) {
        snprintf(p, end - p, "  a b c d e f g h");
    }
    *p = '\0';
}
