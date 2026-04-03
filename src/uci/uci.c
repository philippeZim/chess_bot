#include "uci.h"
#include "../board/board.h"
#include "../board/bitboard.h"
#include "../move/move.h"
#include "../search/search.h"
#include "../evaluation/evaluate.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define MAX_INPUT 4096

static SearchContext ctx;
static Board board;
static volatile bool stopped = false;

static void uci_print_info(int depth, int score, uint64_t nodes, double time_ms) {
    printf("info depth %d score cp %d nodes %lu nps %.0f\n",
           depth, score, nodes, time_ms > 0 ? nodes * 1000.0 / time_ms : 0);
}

static void uci_print_best_move(Move move) {
    const char* from_str = square_to_string(move_from(move));
    const char* to_str = square_to_string(move_to(move));
    
    printf("bestmove %s%s", from_str, to_str);
    
    if (move_promotion(move)) {
        const char promo_char[] = " nbrq";
        int promo = move_promotion(move);
        if (promo >= 1 && promo <= 4) {
            printf("%c", promo_char[promo]);
        }
    }
    printf("\n");
    fflush(stdout);
}

static void uci_print_id(void) {
    printf("id name ChessBot 0.2.0\n");
    printf("id author Philippe\n");
    printf("uciok\n");
    fflush(stdout);
}

static void uci_go(const Board* pos, SearchLimits limits) {
    stopped = false;
    search_clear(&ctx);
    
    int best_move = 0;
    clock_t start = clock();
    
    for (int depth = 1; depth <= limits.depth && !stopped; depth++) {
        int score = search_negamax(&ctx, (Board*)pos, depth, -32000, 32000);
        
        clock_t now = clock();
        double elapsed = (now - start) * 1000.0 / CLOCKS_PER_SEC;
        
        uci_print_info(depth, score, ctx.nodes, elapsed);
        
        if (limits.movetime > 0 && elapsed >= limits.movetime) {
            break;
        }
    }
    
    best_move = tt_get_move(&ctx.tt, pos->zobrist);
    if (best_move == 0) {
        MoveList moves;
        move_list_init(&moves);
        generate_pseudolegal_moves(pos, &moves);
        if (moves.count > 0) {
            best_move = moves.moves[0];
        }
    }
    
    uci_print_best_move(best_move);
}

static bool apply_move(Board* b, Move move) {
    int from = move_from(move);
    int to = move_to(move);
    Piece piece = b->squares[from];
    
    if (piece == NO_PIECE) return false;
    
    Piece captured = b->squares[to];
    Color us = piece_color(piece);
    Color them = opposite_color(us);
    PieceType pt = piece_type(piece);
    
    b->byType[pt] &= ~(1ULL << from);
    b->byColor[us] &= ~(1ULL << from);
    b->squares[from] = NO_PIECE;
    
    if (captured != NO_PIECE) {
        b->byType[piece_type(captured)] &= ~(1ULL << to);
        b->byColor[them] &= ~(1ULL << to);
    }
    
    if (piece_type(piece) == PAWN && b->enPassant != SQUARE_NB) {
        if (to == b->enPassant) {
            int ep_rank = square_rank(to);
            int ep_capture_rank = (us == ColorWhite) ? ep_rank - 1 : ep_rank + 1;
            Square ep_capture_sq = make_square(square_file(to), ep_capture_rank);
            b->squares[ep_capture_sq] = NO_PIECE;
            b->byType[PAWN] &= ~(1ULL << ep_capture_sq);
            b->byColor[them] &= ~(1ULL << ep_capture_sq);
        }
    }
    
    if (move_promotion(move)) {
        PieceType promo = move_promotion(move);
        b->squares[to] = make_piece(us, promo);
        b->byType[promo] |= (1ULL << to);
        b->byColor[us] |= (1ULL << to);
    } else {
        b->squares[to] = piece;
        b->byType[pt] |= (1ULL << to);
        b->byColor[us] |= (1ULL << to);
    }
    
    if (piece_type(piece) == PAWN && abs(from - to) == 16) {
        b->enPassant = from + (us == ColorWhite ? 8 : -8);
    } else {
        b->enPassant = SQUARE_NB;
    }
    
    if (piece_type(piece) == KING) {
        if (abs(to - from) == 2) {
            if (to == SQUARE_G1 || to == SQUARE_G8) {
                b->squares[from + 1] = b->squares[from + 3];
                b->squares[from + 3] = NO_PIECE;
                b->byType[ROOK] &= ~(1ULL << (from + 3));
                b->byType[ROOK] |= (1ULL << (from + 1));
                b->byColor[us] &= ~(1ULL << (from + 3));
                b->byColor[us] |= (1ULL << (from + 1));
            } else if (to == SQUARE_C1 || to == SQUARE_C8) {
                b->squares[from - 1] = b->squares[from - 4];
                b->squares[from - 4] = NO_PIECE;
                b->byType[ROOK] &= ~(1ULL << (from - 4));
                b->byType[ROOK] |= (1ULL << (from - 1));
                b->byColor[us] &= ~(1ULL << (from - 4));
                b->byColor[us] |= (1ULL << (from - 1));
            }
        }
    }
    
    if (piece_type(piece) == KING) {
        if (us == ColorWhite) {
            b->castling &= ~(CASTLING_WK | CASTLING_WQ);
        } else {
            b->castling &= ~(CASTLING_BK | CASTLING_BQ);
        }
    }
    if (captured == make_piece(them, ROOK) || to == (them == ColorWhite ? SQUARE_A1 : SQUARE_A8)) {
        b->castling &= ~(them == ColorWhite ? CASTLING_WQ : CASTLING_BQ);
    }
    if (captured == make_piece(them, ROOK) || to == (them == ColorWhite ? SQUARE_H1 : SQUARE_H8)) {
        b->castling &= ~(them == ColorWhite ? CASTLING_WK : CASTLING_BK);
    }
    
    if (piece_type(piece) == ROOK && from == (us == ColorWhite ? SQUARE_H1 : SQUARE_H8)) {
        b->castling &= (us == ColorWhite ? ~CASTLING_WK : ~CASTLING_BK);
    }
    if (piece_type(piece) == ROOK && from == (us == ColorWhite ? SQUARE_A1 : SQUARE_A8)) {
        b->castling &= (us == ColorWhite ? ~CASTLING_WQ : ~CASTLING_BQ);
    }
    
    b->turn = opposite_color(b->turn);
    b->halfMoves++;
    if (piece_type(piece) == PAWN || captured != NO_PIECE) {
        b->halfMoves = 0;
    }
    if (b->turn == ColorWhite) {
        b->moveNumber++;
    }
    
    return true;
}

bool uci_parse_position(const char* cmd, char* fen, size_t fen_size, char* moves, size_t moves_size) {
    const char* p = cmd;
    
    while (*p && *p != ' ') p++;
    while (*p && *p == ' ') p++;
    
    bool has_fen = false; (void)has_fen;
    if (strncmp(p, "fen", 3) == 0) {
        p += 3;
        while (*p && *p == ' ') p++;
        has_fen = true;
        
        char* out = fen;
        size_t remaining = fen_size - 1;
        
        while (*p && remaining > 0) {
            if (*p == 'm' && strncmp(p + 1, "oves ", 5) == 0) {
                *out = '\0';
                break;
            }
            *out++ = *p++;
            remaining--;
        }
        *out = '\0';
    } else if (strncmp(p, "startpos", 8) == 0) {
        strncpy(fen, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", fen_size);
        p += 8;
    } else {
        strncpy(fen, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", fen_size);
    }
    
    while (*p && *p != 'm') p++;
    if (strncmp(p, "moves", 5) == 0) {
        p += 5;
        while (*p && *p == ' ') p++;
        
        char* out = moves;
        size_t remaining = moves_size - 1;
        
        while (*p && remaining > 1) {
            if (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t') {
                if (out > moves) {
                    *out++ = ' ';
                }
                while (*p && (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t')) p++;
            } else {
                *out++ = *p++;
                remaining--;
            }
        }
        *out = '\0';
    } else {
        moves[0] = '\0';
    }
    
    return true;
}

SearchLimits uci_default_limits(void) {
    SearchLimits lim = {0};
    lim.depth = 6;
    lim.movetime = -1;
    lim.time_white = -1;
    lim.time_black = -1;
    lim.movestogo = -1;
    lim.stop = false;
    return lim;
}

static SearchLimits parse_go(const char* cmd) {
    SearchLimits lim = uci_default_limits();
    const char* p = cmd + 2;
    
    while (*p == ' ') p++;
    
    while (*p) {
        while (*p == ' ') p++;
        
        if (strncmp(p, "depth", 5) == 0) {
            p += 5;
            while (*p == ' ') p++;
            lim.depth = atoi(p);
        } else if (strncmp(p, "movetime", 8) == 0) {
            p += 8;
            while (*p == ' ') p++;
            lim.movetime = atoi(p);
        } else if (strncmp(p, "wtime", 5) == 0) {
            p += 5;
            while (*p == ' ') p++;
            lim.time_white = atoll(p);
        } else if (strncmp(p, "btime", 5) == 0) {
            p += 5;
            while (*p == ' ') p++;
            lim.time_black = atoll(p);
        } else if (strncmp(p, "winc", 4) == 0) {
            p += 4;
            while (*p == ' ') p++;
        } else if (strncmp(p, "binc", 4) == 0) {
            p += 4;
            while (*p == ' ') p++;
        } else if (strncmp(p, "movestogo", 9) == 0) {
            p += 9;
            while (*p == ' ') p++;
            lim.movestogo = atoi(p);
        } else if (strncmp(p, "infinite", 8) == 0) {
            p += 8;
            lim.depth = 100;
        } else {
            while (*p && *p != ' ') p++;
        }
    }
    
    return lim;
}

static Move parse_move(const char* move_str) {
    if (strlen(move_str) < 4) return 0;
    
    char from_str[3] = {move_str[0], move_str[1], '\0'};
    char to_str[3] = {move_str[2], move_str[3], '\0'};
    
    int from = string_to_square(from_str);
    int to = string_to_square(to_str);
    
    if (from >= SQUARE_NB || to >= SQUARE_NB) return 0;
    
    Move move = make_move(from, to);
    
    if (strlen(move_str) >= 5) {
        char promo = tolower(move_str[4]);
        PieceType pt = 0;
        switch (promo) {
            case 'q': pt = QUEEN; break;
            case 'r': pt = ROOK; break;
            case 'b': pt = BISHOP; break;
            case 'n': pt = KNIGHT; break;
        }
        if (pt) {
            move = make_promotion_move(from, to, pt);
        }
    }
    
    return move;
}

static bool is_legal_move(Board* b, Move move) {
    return move_is_legal(b, move);
}

void uci_loop(void) {
    board_init(&board);
    search_init(&ctx);
    
    char line[MAX_INPUT];
    
    printf("ChessBot UCI interface ready\n");
    fflush(stdout);
    
    while (fgets(line, sizeof(line), stdin)) {
        line[MAX_INPUT - 1] = '\0';
        size_t len = strlen(line);
        while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')) {
            line[--len] = '\0';
        }
        
        if (strcmp(line, "uci") == 0) {
            uci_print_id();
        } else if (strcmp(line, "isready") == 0) {
            printf("readyok\n");
            fflush(stdout);
        } else if (strcmp(line, "quit") == 0) {
            break;
        } else if (strcmp(line, "stop") == 0) {
            stopped = true;
        } else if (strncmp(line, "position", 8) == 0) {
            char fen[256];
            char moves[1024];
            
            if (uci_parse_position(line, fen, sizeof(fen), moves, sizeof(moves))) {
                board_set_fen(&board, fen);
                
                char* m = moves;
                while (*m) {
                    while (*m == ' ') m++;
                    if (!*m) break;
                    
                    char move_str[8];
                    size_t i = 0;
                    while (*m && *m != ' ' && i < sizeof(move_str) - 1) {
                        move_str[i++] = *m++;
                    }
                    move_str[i] = '\0';
                    
                    Move move = parse_move(move_str);
                    if (move && is_legal_move(&board, move)) {
                        apply_move(&board, move);
                    }
                }
            }
        } else if (strncmp(line, "go", 2) == 0 && (line[2] == ' ' || line[2] == '\0')) {
            SearchLimits lim = parse_go(line);
            uci_go(&board, lim);
        } else if (strcmp(line, "ucinewgame") == 0) {
            board_init(&board);
            search_clear(&ctx);
        } else if (strncmp(line, "d", 1) == 0) {
            board_print(&board);
            printf("\nFEN: ");
            char fen[256];
            board_get_fen(&board, fen, sizeof(fen));
            printf("%s\n", fen);
            printf("Hash: %llu\n", (unsigned long long)board.zobrist);
            fflush(stdout);
        }
    }
    
    tt_free(&ctx.tt);
}
