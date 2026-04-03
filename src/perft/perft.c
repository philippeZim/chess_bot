#include "perft.h"
#include "../board/board.h"
#include "../board/bitboard.h"
#include "../move/move.h"
#include "../move/movegen.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

typedef struct {
    Board board;
    Move move;
    Piece captured_piece;
    Square captured_square;
    uint8_t castling_before;
    Square enPassant_before;
    uint8_t halfMoves_before;
    uint16_t moveNumber_before;
    Color turn_before;
} MoveState;

static void save_state(const Board* board, MoveState* state) {
    if (!board || !state) return;
    board_clone(board, &state->board);
    state->castling_before = board->castling;
    state->enPassant_before = board->enPassant;
    state->halfMoves_before = board->halfMoves;
    state->moveNumber_before = board->moveNumber;
    state->turn_before = board->turn;
}

static void restore_state(Board* board, const MoveState* state) {
    if (!board || !state) return;
    board_clone(&state->board, board);
}

static Piece make_move_state(Board* board, Move move, MoveState* state) {
    Square from = move_from(move);
    Square to = move_to(move);
    
    save_state(board, state);
    
    Piece moving = board->squares[from];
    Piece captured = board->squares[to];
    
    /* Handle en passant capture */
    if (piece_type(moving) == PAWN && board->enPassant != SQUARE_NB && to == board->enPassant) {
        Square capturedPawnSq = (board->turn == ColorWhite) ? to + 8 : to - 8;
        captured = board->squares[capturedPawnSq];
    }
    
    state->captured_piece = captured;
    state->captured_square = to;
    
    /* Handle en passant capture: remove pawn at different square */
    if (piece_type(moving) == PAWN && board->enPassant != SQUARE_NB && to == board->enPassant) {
        Square capturedPawnSq = (board->turn == ColorWhite) ? to + 8 : to - 8;
        state->captured_square = capturedPawnSq;
        board->byType[PAWN] &= ~(1ULL << capturedPawnSq);
        board->byColor[opposite_color(board->turn)] &= ~(1ULL << capturedPawnSq);
        board->squares[capturedPawnSq] = NO_PIECE;
    }
    
    if (captured != NO_PIECE) {
        board->byType[piece_type(captured)] &= ~(1ULL << to);
        board->byColor[piece_color(captured)] &= ~(1ULL << to);
        board->squares[to] = NO_PIECE;
        
        if (piece_type(captured) == KING) {
            board->byType[KING] &= ~(1ULL << to);
            board->byColor[piece_color(captured)] &= ~(1ULL << to);
        }
    }
    
    board->squares[from] = NO_PIECE;
    board->byType[piece_type(moving)] &= ~(1ULL << from);
    board->byColor[piece_color(moving)] &= ~(1ULL << from);
    
    Piece promoted_piece = NO_PIECE;
    if (move_promotion(move)) {
        promoted_piece = make_piece(board->turn, move_promotion(move));
        board->squares[to] = promoted_piece;
        board->byType[move_promotion(move)] |= (1ULL << to);
        board->byColor[piece_color(moving)] |= (1ULL << to);
    } else {
        board->squares[to] = moving;
        board->byType[piece_type(moving)] |= (1ULL << to);
        board->byColor[piece_color(moving)] |= (1ULL << to);
    }
    
    board->castling = 0;
    if (piece_type(moving) == KING) {
        if (piece_color(moving) == ColorWhite) {
            board->castling &= ~(CASTLING_WK | CASTLING_WQ);
        } else {
            board->castling &= ~(CASTLING_BK | CASTLING_BQ);
        }
        if (moving == W_KING && from == SQUARE_E1 && to == SQUARE_G1) {
            board->squares[SQUARE_F1] = board->squares[SQUARE_H1];
            board->squares[SQUARE_H1] = NO_PIECE;
            board->byType[ROOK] &= ~(1ULL << SQUARE_H1);
            board->byType[ROOK] |= (1ULL << SQUARE_F1);
            board->byColor[ColorWhite] &= ~(1ULL << SQUARE_H1);
            board->byColor[ColorWhite] |= (1ULL << SQUARE_F1);
        } else if (moving == W_KING && from == SQUARE_E1 && to == SQUARE_C1) {
            board->squares[SQUARE_D1] = board->squares[SQUARE_A1];
            board->squares[SQUARE_A1] = NO_PIECE;
            board->byType[ROOK] &= ~(1ULL << SQUARE_A1);
            board->byType[ROOK] |= (1ULL << SQUARE_D1);
            board->byColor[ColorWhite] &= ~(1ULL << SQUARE_A1);
            board->byColor[ColorWhite] |= (1ULL << SQUARE_D1);
        } else if (moving == B_KING && from == SQUARE_E8 && to == SQUARE_G8) {
            board->squares[SQUARE_F8] = board->squares[SQUARE_H8];
            board->squares[SQUARE_H8] = NO_PIECE;
            board->byType[ROOK] &= ~(1ULL << SQUARE_H8);
            board->byType[ROOK] |= (1ULL << SQUARE_F8);
            board->byColor[ColorBlack] &= ~(1ULL << SQUARE_H8);
            board->byColor[ColorBlack] |= (1ULL << SQUARE_F8);
        } else if (moving == B_KING && from == SQUARE_E8 && to == SQUARE_C8) {
            board->squares[SQUARE_D8] = board->squares[SQUARE_A8];
            board->squares[SQUARE_A8] = NO_PIECE;
            board->byType[ROOK] &= ~(1ULL << SQUARE_A8);
            board->byType[ROOK] |= (1ULL << SQUARE_D8);
            board->byColor[ColorBlack] &= ~(1ULL << SQUARE_A8);
            board->byColor[ColorBlack] |= (1ULL << SQUARE_D8);
        }
    }
    
    if (piece_type(moving) == ROOK) {
        if (from == SQUARE_A1) board->castling &= ~CASTLING_WQ;
        if (from == SQUARE_H1) board->castling &= ~CASTLING_WK;
        if (from == SQUARE_A8) board->castling &= ~CASTLING_BQ;
        if (from == SQUARE_H8) board->castling &= ~CASTLING_BK;
    }
    
    if (captured != NO_PIECE && piece_type(captured) == ROOK) {
        if (to == SQUARE_A1) board->castling &= ~CASTLING_WQ;
        if (to == SQUARE_H1) board->castling &= ~CASTLING_WK;
        if (to == SQUARE_A8) board->castling &= ~CASTLING_BQ;
        if (to == SQUARE_H8) board->castling &= ~CASTLING_BK;
    }
    
    board->enPassant = SQUARE_NB;
    if (piece_type(moving) == PAWN) {
        int from_rank = square_rank(from);
        int to_rank = square_rank(to);
        if ((to_rank == 3 && from_rank == 1) || (to_rank == 4 && from_rank == 6)) {
            if ((to == SQUARE_D3 && from == SQUARE_D2) || (to == SQUARE_E3 && from == SQUARE_E2) ||
                (to == SQUARE_F3 && from == SQUARE_F2) || (to == SQUARE_G3 && from == SQUARE_G2) ||
                (to == SQUARE_D3 && from == SQUARE_D2) || (to == SQUARE_D7 && from == SQUARE_D6) ||
                (to == SQUARE_E7 && from == SQUARE_E6) || (to == SQUARE_F7 && from == SQUARE_F6) ||
                (to == SQUARE_G7 && from == SQUARE_G6)) {
                board->enPassant = (to_rank == 3) ? (to + 8) : (to - 8);
            }
        }
    }
    
    board->halfMoves++;
    if (piece_type(moving) == PAWN || captured != NO_PIECE) {
        board->halfMoves = 0;
    }
    
    state->board.turn = opposite_color(state->board.turn);
    if (state->board.turn == ColorWhite) {
        state->board.moveNumber++;
    }
    
    return captured;
}

static void unmake_move_state(Board* board, MoveState* state) {
    if (!board || !state) return;
    restore_state(board, state);
}

static bool is_move_legal(const Board* board, Move move) {
Square ourKingColor = board->turn;
    Square ourKing = SQUARE_NB;
    for (int i = 0; i < BOARD_SIZE; i++) {
        if (board->squares[i] == make_piece(ourKingColor, KING)) {
            ourKing = i;
            break;
        }
    }
    
    if (ourKing == SQUARE_NB) return false;
    
   MoveState state;
    make_move_state((Board*)board, move, &state);
    
   /* After make_move_state, state.board.turn has flipped.
     * The opponent is now state.board.turn.
     * We need to check if opponent's pieces attack our king. */
    Square kingSq = ourKing;
    
    /* If king moved, update king square */
    Square from = move_from(move);
    Square to = move_to(move);
    if (from == ourKing) {
        kingSq = to;
    }
    
    /* Opponent color = current turn in state.board */
    Color opponentColor = state.board.turn;
    Bitboard occ = state.board.byColor[ColorWhite] | state.board.byColor[ColorBlack];
    
    Bitboard pawnAttacks = pawn_attacks(opponentColor, kingSq);
    if (pawnAttacks & state.board.byType[PAWN] & state.board.byColor[opponentColor]) {
        unmake_move_state((Board*)board, &state);
        return false;
    }
    
    if (square_attacks_king(kingSq) & state.board.byType[KING] & state.board.byColor[opponentColor]) {
        unmake_move_state((Board*)board, &state);
        return false;
    }
    
    if (square_attacks_knight(kingSq) & state.board.byType[KNIGHT] & state.board.byColor[opponentColor]) {
        unmake_move_state((Board*)board, &state);
        return false;
    }
    
    if (bishop_attacks(kingSq, occ) & state.board.byType[BISHOP] & state.board.byColor[opponentColor]) {
        unmake_move_state((Board*)board, &state);
        return false;
    }
    
    if (bishop_attacks(kingSq, occ) & state.board.byType[QUEEN] & state.board.byColor[opponentColor]) {
        unmake_move_state((Board*)board, &state);
        return false;
    }
    
    if (rook_attacks(kingSq, occ) & state.board.byType[ROOK] & state.board.byColor[opponentColor]) {
        unmake_move_state((Board*)board, &state);
        return false;
    }
    
    if (rook_attacks(kingSq, occ) & state.board.byType[QUEEN] & state.board.byColor[opponentColor]) {
        unmake_move_state((Board*)board, &state);
        return false;
    }
    
    unmake_move_state((Board*)board, &state);
    return true;
}

static uint64_t perft_recursive(const Board* board, int depth, int print_moves) {
    if (depth == 0) {
        return 1;
    }
    
    MoveList moves;
    generate_pseudolegal_moves(board, &moves);
    
    uint64_t nodes = 0;
    
    for (int i = 0; i < moves.count; i++) {
        Move move = moves.moves[i];
        
        if (!is_move_legal(board, move)) {
            continue;
        }
        
        MoveState state;
        
        make_move_state((Board*)board, move, &state);
        
        uint64_t newNodes = perft_recursive((Board*)board, depth - 1, 0);
        
        unmake_move_state((Board*)board, &state);

        if (depth == 0 && print_moves) {
            char fromStr[3], toStr[3];
            const char* files = "abcdefgh";
            const char* ranks = "12345678";
            fromStr[0] = files[move_from(move) & 7];
            fromStr[1] = ranks[(move_from(move) >> 3) & 7];
            fromStr[2] = '\0';
            toStr[0] = files[move_to(move) & 7];
            toStr[1] = ranks[(move_to(move) >> 3) & 7];
            toStr[2] = '\0';
            printf("%s%s %lu\n", fromStr, toStr, (unsigned long)newNodes);
        }
        
        nodes += newNodes;
    }
    
    return nodes;
}

uint64_t perft_depth(const Board* board, int depth) {
    return perft_recursive(board, depth, 0);
}

uint64_t perft_divide(const Board* board, int depth) {
    return perft_recursive(board, depth, 1);
}

uint64_t perft(const Board* board, int depth, int print_moves) {
    if (print_moves) {
        return perft_divide(board, depth);
    }
    return perft_depth(board, depth);
}
