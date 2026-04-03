#include "search.h"
#include "../transposition/zobrist.h"
#include "../evaluation/evaluate.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void find_king_square(const Board* board, Color color, Square* king_sq) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        if (board->squares[i] == make_piece(color, KING)) {
            *king_sq = i;
            return;
        }
    }
    *king_sq = SQUARE_NB;
}

static void make_move_simple(Board* board, Move move) {
    int from = move & 0x3F;
    int to = (move >> 6) & 0x3F;
    Piece piece = board->squares[from];
    (void)board->squares[to];
    
    board->squares[to] = piece;
    board->squares[from] = NO_PIECE;
    
    board->turn = opposite_color(board->turn);
    board->zobrist = zobrist_hash_board(board);
}

static void unmake_move_simple(Board* board, Move move, Piece captured, Color prev_color, uint64_t prev_zob) {
    int from = move & 0x3F;
    int to = (move >> 6) & 0x3F;
    Piece piece = board->squares[to];
    
    board->squares[from] = piece;
    board->squares[to] = captured;
    
    board->turn = prev_color;
    board->zobrist = prev_zob;
}

static int is_capture(const Board* board, Move move) {
    int to = (move >> 6) & 0x3F;
    return board->squares[to] != NO_PIECE;
}

static int is_promotion(Move move) {
    return (move >> 12) != 0;
}

static int piece_value(Piece piece) {
    const int values[] = {0, 100, 320, 330, 500, 900, 0};
    return values[piece_type(piece)];
}

static int mvv_lva_score(const Board* board, Move move) {
    int to = (move >> 6) & 0x3F;
    Piece captured = board->squares[to];
    int from = move & 0x3F;
    Piece attacker = board->squares[from];
    
    if (captured == NO_PIECE) return 0;
    
    return (piece_value(captured) << 4) - piece_value(attacker);
}

static Move get_tt_move(TranspositionTable* tt, uint64_t key) {
    TranspositionEntry* entry = tt_probe(tt, key);
    if (entry) {
        return entry->best_move;
    }
    return 0;
}

static void order_moves(const Board* board, TranspositionTable* tt, 
                       Move* moves, int move_count) {
    uint64_t key = board->zobrist;
    Move tt_move = get_tt_move(tt, key);
    
    Move* ordered = malloc(sizeof(Move) * move_count);
    int* scores = malloc(sizeof(int) * move_count);
    if (!ordered || !scores) {
        if (ordered) free(ordered);
        if (scores) free(scores);
        return;
    }
    
    for (int i = 0; i < move_count; i++) {
        ordered[i] = moves[i];
        if (moves[i] == tt_move) {
            scores[i] = 1000000;
        } else if (is_capture(board, moves[i])) {
            scores[i] = mvv_lva_score(board, moves[i]);
        } else {
            scores[i] = 0;
        }
    }
    
    for (int i = 0; i < move_count; i++) {
        int best_idx = i;
        for (int j = i + 1; j < move_count; j++) {
            if (scores[j] > scores[best_idx]) {
                best_idx = j;
            }
        }
        if (best_idx != i) {
            Move tmp_move = ordered[i];
            int tmp_score = scores[i];
            ordered[i] = ordered[best_idx];
            scores[i] = scores[best_idx];
            ordered[best_idx] = tmp_move;
            scores[best_idx] = tmp_score;
        }
    }
    
    memcpy(moves, ordered, sizeof(Move) * move_count);
    
    free(ordered);
    free(scores);
}

void search_init(SearchContext* ctx) {
    if (ctx) {
        memset(ctx, 0, sizeof(SearchContext));
        tt_alloc(&ctx->tt);
    }
}

void search_clear(SearchContext* ctx) {
    if (ctx) {
        if (ctx->tt.table) {
            memset(ctx->tt.table, 0, sizeof(TranspositionEntry) * ctx->tt.size);
        }
        ctx->nodes = 0;
        ctx->depth = 0;
    }
}

int evaluate_position(SearchContext* ctx, const Board* board) {
    (void)ctx;
    return evaluate(board);
}

int qsearch(SearchContext* ctx, Board* board, int alpha, int beta) {
    int stand_pat = evaluate(board);
    
    if (stand_pat >= beta) {
        return beta;
    }
    
    if (stand_pat > alpha) {
        alpha = stand_pat;
    }
    
    MoveList moves;
    move_list_init(&moves);
    generate_legal_moves(board, &moves);
    
    for (int i = 0; i < moves.count; i++) {
        if (!is_capture(board, moves.moves[i]) && !is_promotion(moves.moves[i])) {
            continue;
        }
        
        Color prev_color = board->turn;
        uint64_t prev_zob = board->zobrist;
        Piece captured = board->squares[(moves.moves[i] >> 6) & 0x3F];
        
        make_move_simple(board, moves.moves[i]);
        
        int score = -qsearch(ctx, board, -beta, -alpha);
        
        unmake_move_simple(board, moves.moves[i], captured, prev_color, prev_zob);
        
        if (score >= beta) {
            return beta;
        }
        
        if (score > alpha) {
            alpha = score;
        }
    }
    
    return alpha;
}

int search_negamax(SearchContext* ctx, Board* board, int depth, int alpha, int beta) {
    if (depth == 0) {
        return qsearch(ctx, board, alpha, beta);
    }
    
    uint64_t key = board->zobrist;
    TranspositionEntry* tt_entry = tt_probe(&ctx->tt, key);
    
    if (tt_entry && tt_entry->depth >= depth) {
        if (tt_entry->flag == TT_FLAG_EXACT) {
            return tt_entry->score;
        } else if (tt_entry->flag == TT_FLAG_ALPHA) {
            if (tt_entry->score <= alpha) {
                return alpha;
            }
        } else if (tt_entry->flag == TT_FLAG_BETA) {
            if (tt_entry->score >= beta) {
                return beta;
            }
        }
    }
    
    MoveList moves;
    move_list_init(&moves);
    generate_legal_moves(board, &moves);
    
    if (moves.count == 0) {
        Square king_sq;
        find_king_square(board, board->turn, &king_sq);
        if (king_sq != SQUARE_NB) {
            return -SCORE_MATE_IN_PLY(0);
        }
        return SCORE_DRAW;
    }
    
    order_moves(board, &ctx->tt, moves.moves, moves.count);
    
    Move best_move = 0;
    int best_score = -SCORE_INFINITE;
    int old_alpha = alpha;
    
    ctx->nodes++;
    
    for (int i = 0; i < moves.count; i++) {
        Move move = moves.moves[i];
        Color prev_color = board->turn;
        uint64_t prev_zob = board->zobrist;
        Piece captured = board->squares[(move >> 6) & 0x3F];
        
        make_move_simple(board, move);
        
        int score = -search_negamax(ctx, board, depth - 1, -beta, -alpha);
        
        unmake_move_simple(board, move, captured, prev_color, prev_zob);
        
        if (score > best_score) {
            best_score = score;
            best_move = move;
        }
        
        if (score > alpha) {
            alpha = score;
        }
        
        if (alpha >= beta) {
            break;
        }
    }
    
    uint16_t flag;
    if (best_score <= old_alpha) {
        flag = TT_FLAG_ALPHA;
    } else if (best_score >= beta) {
        flag = TT_FLAG_BETA;
    } else {
        flag = TT_FLAG_EXACT;
    }
    
    tt_store(&ctx->tt, key, depth, best_score, flag, best_move);
    
    return best_score;
}

int search_iterative_deepening(SearchContext* ctx, Board* board, int max_depth, int* best_move) {
    if (!ctx || !board || !best_move) {
        return 0;
    }
    
    int best_score = -SCORE_INFINITE;
    *best_move = 0;
    
    for (int depth = 1; depth <= max_depth; depth++) {
        ctx->depth = depth;
        ctx->nodes = 0;
        
        int score = search_negamax(ctx, board, depth, -SCORE_INFINITE, SCORE_INFINITE);
        
        if (score > best_score) {
            best_score = score;
        }
    }
    
    uint64_t key = board->zobrist;
    *best_move = tt_get_move(&ctx->tt, key);
    
    return best_score;
}
