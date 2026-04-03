#include "board.h"
#include "bitboard.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

bool board_init(Board* board) {
    memset(board, 0, sizeof(Board));
    for (int i = 0; i < PIECE_TYPE_NB; i++) board->byType[i] = 0;
    for (int i = 0; i < ColorNB; i++) board->byColor[i] = 0;
    board->castling = CASTLING_NONE;
    board->enPassant = SQUARE_NB;
    board->halfMoves = 0;
    board->moveNumber = 1;
    board->turn = ColorWhite;
    return board_set_fen(board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

bool board_set_fen(Board* board, const char* fen) {
    if (!board || !fen) return false;
    
    memset(board, 0, sizeof(Board));
    
    const char* p = fen;
    int rank = 7;
    int file = 0;
    
    while (*p && rank >= 0) {
        if (*p >= '1' && *p <= '8') {
            file += *p - '0';
            p++;
        } else if (*p == '/') {
            p++;
            rank--;
            file = 0;
        } else if (isalpha(*p)) {
            Square sq = make_square(file, rank);
            Piece piece;
            Color color = islower(*p) ? ColorBlack : ColorWhite;
            char type = tolower(*p);
            
            switch (type) {
                case 'p': piece = make_piece(color, PAWN); break;
                case 'n': piece = make_piece(color, KNIGHT); break;
                case 'b': piece = make_piece(color, BISHOP); break;
                case 'r': piece = make_piece(color, ROOK); break;
                case 'q': piece = make_piece(color, QUEEN); break;
                case 'k': piece = make_piece(color, KING); break;
                default: return false;
            }
            
            board->squares[sq] = piece;
            board->byType[piece_type(piece)] |= 1ULL << sq;
            board->byColor[color] |= 1ULL << sq;
            
            file++;
            p++;
        } else {
            break;
        }
    }
    
    while (*p && isspace(*p)) p++;
    if (*p == 'w') board->turn = ColorWhite;
    else if (*p == 'b') board->turn = ColorBlack;
    else return false;
    p++;
    
    while (*p && isspace(*p)) p++;
    board->castling = CASTLING_NONE;
    if (*p == '-') {
        p++;
    } else {
        while (*p && !isspace(*p)) {
            switch (*p) {
                case 'K': board->castling |= CASTLING_WK; break;
                case 'Q': board->castling |= CASTLING_WQ; break;
                case 'k': board->castling |= CASTLING_BK; break;
                case 'q': board->castling |= CASTLING_BQ; break;
            }
            p++;
        }
    }
    
    while (*p && isspace(*p)) p++;
    board->enPassant = SQUARE_NB;
    if (isalpha(*p) && isdigit(p[1])) {
        int file = *p - 'a';
        int rank = p[1] - '1';
        board->enPassant = make_square(file, rank);
        p += 2;
    }
    
    while (*p && isspace(*p)) p++;
    board->halfMoves = 0;
    if (isdigit(*p)) {
        while (isdigit(*p)) {
            board->halfMoves = board->halfMoves * 10 + (*p - '0');
            p++;
        }
    }
    
    while (*p && isspace(*p)) p++;
    board->moveNumber = 1;
    if (isdigit(*p)) {
        while (isdigit(*p)) {
            board->moveNumber = board->moveNumber * 10 + (*p - '0');
            p++;
        }
    }
    
    return true;
}

bool board_get_fen(Board* board, char* fen, size_t size) {
    if (!board || !fen || size < 50) return false;
    
    char* p = fen;
    char* end = fen + size;
    
    for (int rank = 7; rank >= 0; rank--) {
        int empty = 0;
        for (int file = 0; file < 8; file++) {
            Square sq = make_square(file, rank);
            Piece piece = board->squares[sq];
            if (piece == NO_PIECE) {
                empty++;
            } else {
                if (empty > 0) {
                    p += snprintf(p, end - p, "%d", empty);
                    empty = 0;
                }
                char c;
                switch (piece_type(piece)) {
                    case PAWN: c = 'p'; break;
                    case KNIGHT: c = 'n'; break;
                    case BISHOP: c = 'b'; break;
                    case ROOK: c = 'r'; break;
                    case QUEEN: c = 'q'; break;
                    case KING: c = 'k'; break;
                    default: c = '?'; break;
                }
                if (piece_color(piece) == ColorWhite) c = toupper(c);
                p += snprintf(p, end - p, "%c", c);
            }
        }
        if (empty > 0) {
            p += snprintf(p, end - p, "%d", empty);
        }
        if (rank > 0) {
            p += snprintf(p, end - p, "/");
        }
    }
    
    p += snprintf(p, end - p, " %c ", board->turn == ColorWhite ? 'w' : 'b');
    
    if (board->castling == CASTLING_NONE) {
        p += snprintf(p, end - p, "-");
    } else {
        if (board->castling & CASTLING_WK) p += snprintf(p, end - p, "K");
        if (board->castling & CASTLING_WQ) p += snprintf(p, end - p, "Q");
        if (board->castling & CASTLING_BK) p += snprintf(p, end - p, "k");
        if (board->castling & CASTLING_BQ) p += snprintf(p, end - p, "q");
    }
    
    p += snprintf(p, end - p, " ");
    if (board->enPassant == SQUARE_NB) {
        p += snprintf(p, end - p, "-");
    } else {
        p += snprintf(p, end - p, "%s", square_to_string(board->enPassant));
    }
    
    p += snprintf(p, end - p, " %d %d", board->halfMoves, board->moveNumber);
    
    return true;
}

bool board_is_valid(const Board* board) {
    if (!board) return false;
    
    int wking = 0, bking = 0;
    for (int i = 0; i < BOARD_SIZE; i++) {
        Piece p = board->squares[i];
        if (p == NO_PIECE) continue;
        if (piece_type(p) == KING) {
            if (piece_color(p) == ColorWhite) wking++;
            else bking++;
        }
    }
    
    if (wking != 1 || bking != 1) return false;
    return true;
}

void board_clone(const Board* src, Board* dst) {
    if (!src || !dst) return;
    memcpy(dst, src, sizeof(Board));
}

void board_print(const Board* board) {
    if (!board) return;
    
    printf("+---+---+---+---+---+---+---+---+\n");
    for (int rank = 7; rank >= 0; rank--) {
        printf("%d ", rank + 1);
        for (int file = 0; file < 8; file++) {
            Square sq = make_square(file, rank);
            Piece p = board->squares[sq];
            if (p == NO_PIECE) {
                printf("| . ");
            } else {
                char c;
                switch (piece_type(p)) {
                    case PAWN: c = 'p'; break;
                    case KNIGHT: c = 'n'; break;
                    case BISHOP: c = 'b'; break;
                    case ROOK: c = 'r'; break;
                    case QUEEN: c = 'q'; break;
                    case KING: c = 'k'; break;
                    default: c = '?'; break;
                }
                if (piece_color(p) == ColorWhite) c = toupper(c);
                printf("| %c ", c);
            }
        }
        printf("|\n");
        printf("+---+---+---+---+---+---+---+---+\n");
    }
    printf("  a   b   c   d   e   f   g   h\n");
}

bool board_is_check(const Board* board) {
    if (!board) return false;
    
    Square kingSq = SQUARE_NB;
    for (int i = 0; i < BOARD_SIZE; i++) {
        if (board->squares[i] == make_piece(board->turn, KING)) {
            kingSq = i;
            break;
        }
    }
    
    if (kingSq == SQUARE_NB) return false;
    
    Color opp = opposite_color(board->turn);
    Bitboard occ = board->byColor[ColorWhite] | board->byColor[ColorBlack];
    
    Bitboard pawnAttacks = pawn_attacks(opp, kingSq);
    if (pawnAttacks & board->byType[PAWN] & board->byColor[opp]) {
        return true;
    }
    
    if (square_attacks_king(kingSq) & board->byType[KING] & board->byColor[opp]) {
        return true;
    }
    
    if (square_attacks_knight(kingSq) & board->byType[KNIGHT] & board->byColor[opp]) {
        return true;
    }
    
    if (bishop_attacks(kingSq, occ) & board->byType[BISHOP] & board->byColor[opp]) {
        return true;
    }
    
    if (bishop_attacks(kingSq, occ) & board->byType[QUEEN] & board->byColor[opp]) {
        return true;
    }
    
    if (rook_attacks(kingSq, occ) & board->byType[ROOK] & board->byColor[opp]) {
        return true;
    }
    
    if (rook_attacks(kingSq, occ) & board->byType[QUEEN] & board->byColor[opp]) {
        return true;
    }
    
    return false;
}
