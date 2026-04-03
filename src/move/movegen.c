#include "movegen.h"
#include "../board/bitboard.h"
#include <string.h>
#include <stdlib.h>

#define move_from(m) ((m) & 63)
#define move_to(m) (((m) >> 6) & 63)
#define move_promotion(m) (((m) >> 12) & 7)

void move_list_init(MoveList* list) {
    if (!list) return;
    list->count = 0;
    memset(list->moves, 0, sizeof(list->moves));
}

bool move_list_add(MoveList* list, Move move) {
    if (!list || list->count >= MAX_MOVES_PER_POSITION) return false;
    list->moves[list->count++] = move;
    return true;
}

static bool is_square_attacked(const Board* board, Square square, Color byColor) {
    Bitboard occ = board->byColor[ColorWhite] | board->byColor[ColorBlack];
    
    if (pawn_attacks(byColor, square) & board->byType[PAWN] & board->byColor[byColor]) {
        return true;
    }
    
    if (square_attacks_king(square) & board->byType[KING] & board->byColor[byColor]) {
        return true;
    }
    
    if (square_attacks_knight(square) & board->byType[KNIGHT] & board->byColor[byColor]) {
        return true;
    }
    
    if (bishop_attacks(square, occ) & (board->byType[BISHOP] | board->byType[QUEEN]) & board->byColor[byColor]) {
        return true;
    }
    
    if (rook_attacks(square, occ) & (board->byType[ROOK] | board->byType[QUEEN]) & board->byColor[byColor]) {
        return true;
    }
    
    return false;
}

static Square find_king_square(const Board* board, Color color) {
    Piece king = make_piece(color, KING);
    for (Square sq = 0; sq < 64; sq++) {
        if (board->squares[sq] == king) {
            return sq;
        }
    }
    return SQUARE_NB;
}

static bool is_king_in_check(const Board* board, Color color) {
    Square king_sq = find_king_square(board, color);
    if (king_sq == SQUARE_NB) return false;
    return is_square_attacked(board, king_sq, opposite_color(color));
}

static void generate_pawn_moves(const Board* board, MoveList* moves) {
    Color color = board->turn;
    Color opp = opposite_color(color);
    Bitboard pawn_bb = board->byType[PAWN] & board->byColor[color];
    Bitboard occ = board->byColor[ColorWhite] | board->byColor[ColorBlack];
    Bitboard opp_bb = board->byColor[opp];
    
    Bitboard push_dir = (color == ColorWhite) ? 8 : -8;
    
    while (pawn_bb) {
        Square from = bitboard_next_square(&pawn_bb);
        int from_rank = square_rank(from);
        int from_file = square_file(from);
        
        Bitboard to_bb = (Bitboard)1ULL << (from + push_dir);
        if (!(occ & to_bb)) {
            /* Promotion: white from rank 7, black from rank 2 */
            bool is_promotion = (color == ColorWhite && from_rank == 6) ||
                                (color == ColorBlack && from_rank == 1);
            if (is_promotion) {
                move_list_add(moves, make_promotion_move(from, from + push_dir, QUEEN));
                move_list_add(moves, make_promotion_move(from, from + push_dir, ROOK));
                move_list_add(moves, make_promotion_move(from, from + push_dir, BISHOP));
                move_list_add(moves, make_promotion_move(from, from + push_dir, KNIGHT));
            } else {
                move_list_add(moves, make_move(from, from + push_dir));
            }
            
            /* Double push: white from rank 2, black from rank 7 */
            bool can_double = (color == ColorWhite && from_rank == 1) ||
                              (color == ColorBlack && from_rank == 6);
            if (can_double) {
                Bitboard double_push_bb = (Bitboard)1ULL << (from + 2 * push_dir);
                if (!(occ & double_push_bb)) {
                    move_list_add(moves, make_move(from, from + 2 * push_dir));
                }
            }
        }
        
        if (from_file > 0) {
            Bitboard left_attack = (Bitboard)1ULL << (from + (color == ColorWhite ? 7 : 9));
            if (left_attack & opp_bb) {
      Square to = from + (color == ColorWhite ? 7 : 9);
                int to_rank = square_rank(to);
                /* Promotion: white to rank 8, black to rank 1 */
                if (to_rank == 7 || to_rank == 0) {
                    move_list_add(moves, make_promotion_move(from, to, QUEEN));
                    move_list_add(moves, make_promotion_move(from, to, ROOK));
                    move_list_add(moves, make_promotion_move(from, to, BISHOP));
                    move_list_add(moves, make_promotion_move(from, to, KNIGHT));
                } else {
                    move_list_add(moves, make_move(from, to));
                }
            }
        }
        
        if (from_file < 7) {
            Bitboard right_attack = (Bitboard)1ULL << (from + (color == ColorWhite ? 9 : 7));
            if (right_attack & opp_bb) {
                Square to = from + (color == ColorWhite ? 9 : 7);
                int to_rank = square_rank(to);
                /* Promotion: white to rank 8, black to rank 1 */
                if (to_rank == 7 || to_rank == 0) {
                    move_list_add(moves, make_promotion_move(from, to, QUEEN));
                    move_list_add(moves, make_promotion_move(from, to, ROOK));
                    move_list_add(moves, make_promotion_move(from, to, BISHOP));
                    move_list_add(moves, make_promotion_move(from, to, KNIGHT));
                } else {
                    move_list_add(moves, make_move(from, to));
                }
            }
        }
        
 if (board->enPassant != SQUARE_NB) {
             int ep_file = square_file(board->enPassant);
             int ep_rank = square_rank(board->enPassant);
             
             /* White pawn on rank 5 captures ep on rank 6 (where black pawn jumped over)
              * Black pawn on rank 4 captures ep on rank 3 (where white pawn jumped over)  
             */
             if (color == ColorWhite && from_rank == 5 && ep_rank == 6 && 
                 (ep_file == from_file - 1 || ep_file == from_file + 1)) {
                 move_list_add(moves, make_move(from, board->enPassant));
             } else if (color == ColorBlack && from_rank == 4 && ep_rank == 3 &&
                        (ep_file == from_file - 1 || ep_file == from_file + 1)) {
                 move_list_add(moves, make_move(from, board->enPassant));
             }
         }
    }
}

static void generate_knight_moves(const Board* board, MoveList* moves) {
    Color color = board->turn;
    Color opp = opposite_color(color);
    Bitboard knight_bb = board->byType[KNIGHT] & board->byColor[color];
    Bitboard opp_bb = board->byColor[opp];
    Bitboard own_bb = board->byColor[color];
    
    while (knight_bb) {
        Square from = bitboard_next_square(&knight_bb);
        Bitboard attacks = square_attacks_knight(from);
        
        while (attacks) {
            Square to = bitboard_next_square(&attacks);
            if (!(own_bb & (1ULL << to))) {
                if (opp_bb & (1ULL << to)) {
                    move_list_add(moves, make_move(from, to));
                } else {
                    move_list_add(moves, make_move(from, to));
                }
            }
        }
    }
}

static void generate_bishop_moves(const Board* board, MoveList* moves) {
    Color color = board->turn;
    Color opp = opposite_color(color);
    Bitboard bishop_bb = board->byType[BISHOP] & board->byColor[color];
    Bitboard opp_bb = board->byColor[opp];
    Bitboard own_bb = board->byColor[color];
    Bitboard occ = board->byColor[ColorWhite] | board->byColor[ColorBlack];
    
    while (bishop_bb) {
        Square from = bitboard_next_square(&bishop_bb);
        Bitboard attacks = bishop_attacks(from, occ);
        
        while (attacks) {
            Square to = bitboard_next_square(&attacks);
            if (!(own_bb & (1ULL << to))) {
                if (opp_bb & (1ULL << to)) {
                    move_list_add(moves, make_move(from, to));
                } else {
                    move_list_add(moves, make_move(from, to));
                }
            }
        }
    }
}

static void generate_rook_moves(const Board* board, MoveList* moves) {
    Color color = board->turn;
    Color opp = opposite_color(color);
    Bitboard rook_bb = board->byType[ROOK] & board->byColor[color];
    Bitboard opp_bb = board->byColor[opp];
    Bitboard own_bb = board->byColor[color];
    Bitboard occ = board->byColor[ColorWhite] | board->byColor[ColorBlack];
    
    while (rook_bb) {
        Square from = bitboard_next_square(&rook_bb);
        Bitboard attacks = rook_attacks(from, occ);
        
        while (attacks) {
            Square to = bitboard_next_square(&attacks);
            if (!(own_bb & (1ULL << to))) {
                if (opp_bb & (1ULL << to)) {
                    move_list_add(moves, make_move(from, to));
                } else {
                    move_list_add(moves, make_move(from, to));
                }
            }
        }
    }
}

static void generate_queen_moves(const Board* board, MoveList* moves) {
    Color color = board->turn;
    Color opp = opposite_color(color);
    Bitboard queen_bb = board->byType[QUEEN] & board->byColor[color];
    Bitboard opp_bb = board->byColor[opp];
    Bitboard own_bb = board->byColor[color];
    Bitboard occ = board->byColor[ColorWhite] | board->byColor[ColorBlack];
    
    while (queen_bb) {
        Square from = bitboard_next_square(&queen_bb);
        Bitboard attacks = queen_attacks(from, occ);
        
        while (attacks) {
            Square to = bitboard_next_square(&attacks);
            if (!(own_bb & (1ULL << to))) {
                if (opp_bb & (1ULL << to)) {
                    move_list_add(moves, make_move(from, to));
                } else {
                    move_list_add(moves, make_move(from, to));
                }
            }
        }
    }
}

static void generate_king_moves(const Board* board, MoveList* moves) {
    Color color = board->turn;
    Bitboard king_bb = board->byType[KING] & board->byColor[color];
    Bitboard occ = board->byColor[ColorWhite] | board->byColor[ColorBlack];
    Bitboard own_bb = board->byColor[color];
    
    while (king_bb) {
        Square from = bitboard_next_square(&king_bb);
        Bitboard attacks = square_attacks_king(from);
        
        while (attacks) {
            Square to = bitboard_next_square(&attacks);
            if (!(own_bb & (1ULL << to))) {
                move_list_add(moves, make_move(from, to));
            }
        }
        
        if (color == ColorWhite) {
            if ((board->castling & CASTLING_WK) && !(occ & (1ULL << SQUARE_F1)) && !(occ & (1ULL << SQUARE_G1))) {
                if (!is_square_attacked(board, SQUARE_E1, ColorBlack) &&
                    !is_square_attacked(board, SQUARE_F1, ColorBlack) &&
                    !is_square_attacked(board, SQUARE_G1, ColorBlack)) {
                    move_list_add(moves, make_move(SQUARE_E1, SQUARE_G1));
                }
            }
            if ((board->castling & CASTLING_WQ) && !(occ & (1ULL << SQUARE_D1)) && !(occ & (1ULL << SQUARE_C1)) && !(occ & (1ULL << SQUARE_B1))) {
                if (!is_square_attacked(board, SQUARE_E1, ColorBlack) &&
                    !is_square_attacked(board, SQUARE_D1, ColorBlack) &&
                    !is_square_attacked(board, SQUARE_C1, ColorBlack)) {
                    move_list_add(moves, make_move(SQUARE_E1, SQUARE_C1));
                }
            }
        } else {
            if ((board->castling & CASTLING_BK) && !(occ & (1ULL << SQUARE_F8)) && !(occ & (1ULL << SQUARE_G8))) {
                if (!is_square_attacked(board, SQUARE_E8, ColorWhite) &&
                    !is_square_attacked(board, SQUARE_F8, ColorWhite) &&
                    !is_square_attacked(board, SQUARE_G8, ColorWhite)) {
                    move_list_add(moves, make_move(SQUARE_E8, SQUARE_G8));
                }
            }
            if ((board->castling & CASTLING_BQ) && !(occ & (1ULL << SQUARE_D8)) && !(occ & (1ULL << SQUARE_C8)) && !(occ & (1ULL << SQUARE_B8))) {
                if (!is_square_attacked(board, SQUARE_E8, ColorWhite) &&
                    !is_square_attacked(board, SQUARE_D8, ColorWhite) &&
                    !is_square_attacked(board, SQUARE_C8, ColorWhite)) {
                    move_list_add(moves, make_move(SQUARE_E8, SQUARE_C8));
                }
            }
        }
    }
}

void generate_pseudolegal_moves(const Board* board, MoveList* moves) {
    if (!board || !moves) return;
    move_list_init(moves);
    
    generate_pawn_moves(board, moves);
    generate_knight_moves(board, moves);
    generate_bishop_moves(board, moves);
    generate_rook_moves(board, moves);
    generate_queen_moves(board, moves);
    generate_king_moves(board, moves);
}

static void make_move_on_board(Board* board, Move move) {
    int from = move_from(move);
    int to = move_to(move);
    Piece piece = board->squares[from];
    Color color = piece_color(piece);
    Piece captured = board->squares[to];
    PieceType pt = piece_type(piece);
    
    board->byType[pt] &= ~(1ULL << from);
    board->byColor[color] &= ~(1ULL << from);
    board->squares[from] = NO_PIECE;
    
    if (captured != NO_PIECE) {
        board->byType[pt] &= ~(1ULL << to);
        board->byColor[piece_color(captured)] &= ~(1ULL << to);
    }
    
    if (piece_type(piece) == PAWN && board->enPassant != SQUARE_NB) {
        if (to == board->enPassant) {
            int ep_rank = square_rank(to);
            int ep_capture_rank = (color == ColorWhite) ? ep_rank - 1 : ep_rank + 1;
            Square ep_capture_sq = make_square(square_file(to), ep_capture_rank);
            board->squares[ep_capture_sq] = NO_PIECE;
            board->byType[PAWN] &= ~(1ULL << ep_capture_sq);
            board->byColor[opposite_color(color)] &= ~(1ULL << ep_capture_sq);
        }
    }
    
    if (move_promotion(move)) {
        PieceType promo = move_promotion(move);
        board->squares[to] = make_piece(color, promo);
        board->byType[promo] |= (1ULL << to);
        board->byColor[color] |= (1ULL << to);
    } else {
        board->squares[to] = piece;
        board->byType[pt] |= (1ULL << to);
        board->byColor[color] |= (1ULL << to);
    }
    
    if (piece_type(piece) == PAWN && abs(from - to) == 16) {
        board->enPassant = from + (color == ColorWhite ? 8 : -8);
    } else {
        board->enPassant = SQUARE_NB;
    }
    
    if (piece_type(piece) == KING) {
        if (abs(to - from) == 2) {
            if (to == SQUARE_G1 || to == SQUARE_G8) {
                board->squares[from + 1] = board->squares[from + 3];
                board->squares[from + 3] = NO_PIECE;
                board->byType[ROOK] &= ~(1ULL << (from + 3));
                board->byType[ROOK] |= (1ULL << (from + 1));
                board->byColor[color] &= ~(1ULL << (from + 3));
                board->byColor[color] |= (1ULL << (from + 1));
            } else if (to == SQUARE_C1 || to == SQUARE_C8) {
                board->squares[from - 1] = board->squares[from - 4];
                board->squares[from - 4] = NO_PIECE;
                board->byType[ROOK] &= ~(1ULL << (from - 4));
                board->byType[ROOK] |= (1ULL << (from - 1));
                board->byColor[color] &= ~(1ULL << (from - 4));
                board->byColor[color] |= (1ULL << (from - 1));
            }
        }
    }
    
    board->turn = opposite_color(board->turn);
}

bool move_is_legal(const Board* board, Move move) {
    Board copy;
    memcpy(&copy, board, sizeof(Board));
    make_move_on_board(&copy, move);
    return !is_king_in_check(&copy, piece_color(board->squares[move_from(move)]));
}

void generate_legal_moves(const Board* board, MoveList* moves) {
    if (!board || !moves) return;
    
    move_list_init(moves);
    generate_pseudolegal_moves(board, moves);
    
    int legal_count = 0;
    for (int i = 0; i < moves->count; i++) {
        if (move_is_legal(board, moves->moves[i])) {
            moves->moves[legal_count++] = moves->moves[i];
        }
    }
    moves->count = legal_count;
}
