// movegen.cpp
#include "movegen.h"
#include "bitboard.h"
#include <iostream>

namespace MoveGen {

// Global lookup table for knight moves
Bitboard knight_attacks[64];

// Global lookup table for king moves
Bitboard king_attacks[64];

std::vector<Move> generate_moves(const Board &board) {
    std::vector<Move> pseudo_legal_moves;

    generate_pawn_moves(board, pseudo_legal_moves);
    generate_knight_moves(board, pseudo_legal_moves);
    generate_bishop_moves(board, pseudo_legal_moves);
    generate_rook_moves(board, pseudo_legal_moves);
    generate_queen_moves(board, pseudo_legal_moves);
    generate_king_moves(board, pseudo_legal_moves);
    generate_castling_moves(board, pseudo_legal_moves);

    std::vector<Move> legal_moves;

    for (const Move& move : pseudo_legal_moves) {
        Board board_copy = board;
        board_copy.make_move(move);

        // Get king's square after move
        Bitboard king_bb = board_copy.pieces[board.side_to_move][KING];
        int king_square = Bitboards::lsb(king_bb);

        if (!is_square_attacked(board_copy, king_square, board_copy.side_to_move)) {
            legal_moves.push_back(move);
        }
    }

    return legal_moves;
}

// ------------------- PAWN MOVES ----------------------
void generate_pawn_moves(const Board &board, std::vector<Move> &moves) {
    Color us = board.side_to_move;
    Color them = (us == WHITE) ? BLACK : WHITE;
    Bitboard pawns = board.pieces[us][PAWN];
    Bitboard enemy_pieces = board.occupied(them);
    Bitboard empty = ~board.occupied();

    int forward = (us == WHITE) ? 8 : -8;
    Bitboard promotion_rank = (us == WHITE) ? 0xFF00000000000000ULL : 0x00000000000000FFULL;

    // Single pawn pushes
    Bitboard single_pushes = (us == WHITE) ? (pawns << 8) & empty : (pawns >> 8) & empty;
    Bitboard temp = single_pushes;
    while (temp) {
        int to = Bitboards::lsb(temp);
        int from = to - forward;

        // Check promotions
        if ((1ULL << to) & promotion_rank) {
            moves.push_back({from, to, QUEEN});
            moves.push_back({from, to, ROOK});
            moves.push_back({from, to, BISHOP});
            moves.push_back({from, to, KNIGHT});
        } else {
            moves.push_back({from, to, NO_PIECE});
        }
        Bitboards::clear_bit(temp, to);
    }

    // Double pawn pushes (from original rank)
    Bitboard double_rank = (us == WHITE) ? 0x000000000000FF00ULL : 0x00FF000000000000ULL;
    Bitboard double_pushes = (us == WHITE) ? ((single_pushes & (double_rank << 8)) << 8) & empty
                                           : ((single_pushes & (double_rank >> 8)) >> 8) & empty;
    temp = double_pushes;
    while (temp) {
        int to = Bitboards::lsb(temp);
        int from = to - (forward * 2);
        moves.push_back({from, to, NO_PIECE});
        Bitboards::clear_bit(temp, to);
    }

    // Pawn captures
    Bitboard captures_left, captures_right;
    if (us == WHITE) {
        captures_left  = (pawns << 7) & enemy_pieces & 0x7F7F7F7F7F7F7F7FULL;  // no wrap-around
        captures_right = (pawns << 9) & enemy_pieces & 0xFEFEFEFEFEFEFEFEULL;
    } else {
        captures_left  = (pawns >> 9) & enemy_pieces & 0x7F7F7F7F7F7F7F7FULL;
        captures_right = (pawns >> 7) & enemy_pieces & 0xFEFEFEFEFEFEFEFEULL;
    }

    // Handle captures clearly (left)
    temp = captures_left;
    while (temp) {
        int to = Bitboards::lsb(temp);
        int from = to - ((us == WHITE) ? 7 : -9);

        if ((1ULL << to) & promotion_rank) {  // capture promotions
            moves.push_back({from, to, QUEEN});
            moves.push_back({from, to, ROOK});
            moves.push_back({from, to, BISHOP});
            moves.push_back({from, to, KNIGHT});
        } else {
            moves.push_back({from, to, NO_PIECE});
        }

        Bitboards::clear_bit(temp, to);
    }

    // Handle captures clearly (right)
    temp = captures_right;
    while (temp) {
        int to = Bitboards::lsb(temp);
        int from = to - ((us == WHITE) ? 9 : -7);

        if ((1ULL << to) & promotion_rank) {  // capture promotions
            moves.push_back({from, to, QUEEN});
            moves.push_back({from, to, ROOK});
            moves.push_back({from, to, BISHOP});
            moves.push_back({from, to, KNIGHT});
        } else {
            moves.push_back({from, to, NO_PIECE});
        }

        Bitboards::clear_bit(temp, to);
    }

    // --- En passant clearly ---
    if (board.en_passant_square != -1) {
        Bitboard pawns = board.pieces[us][PAWN];
        // Bitboard ep_square_bb = (1ULL << board.en_passant_square);

        // Possible pawns that can capture en passant
        // Bitboard potential_attackers = (us == WHITE) ?
        //     ((pawns << 7) & 0x7F7F7F7F7F7F7F7FULL) | ((pawns << 9) & 0xFEFEFEFEFEFEFEFEULL) :
        //     ((pawns >> 7) & 0xFEFEFEFEFEFEFEFEULL) | ((pawns >> 9) & 0x7F7F7F7F7F7F7F7FULL);

        if (Bitboards::get_bit(pawns, board.en_passant_square + ((us == WHITE) ? -7 : 9))) {
            moves.push_back({board.en_passant_square - ((us == WHITE) ? 7 : -9), board.en_passant_square, NO_PIECE});
        }

        if (Bitboards::get_bit(pawns, board.en_passant_square - ((us == WHITE) ? 9 : -7))) {
            moves.push_back({board.en_passant_square - ((us == WHITE) ? 9 : -7), board.en_passant_square, NO_PIECE});
        }
    }
}

// Precompute knight moves for every square
void init_knight_attacks() {
    for (int sq = 0; sq < 64; sq++) {
        Bitboard attacks = EMPTY_BITBOARD;
        Bitboard bit = (1ULL << sq);

        if ((bit << 17) & 0xFEFEFEFEFEFEFEFEULL) attacks |= (bit << 17);
        if ((bit << 15) & 0x7F7F7F7F7F7F7F7FULL) attacks |= (bit << 15);
        if ((bit << 10) & 0xFCFCFCFCFCFCFCFCULL) attacks |= (bit << 10);
        if ((bit << 15) & 0x7F7F7F7F7F7F7F7FULL) attacks |= (bit << 6);
        if ((bit >> 17) & 0x7F7F7F7F7F7F7F7FULL) attacks |= (bit >> 17);
        if ((bit >> 15) & 0xFEFEFEFEFEFEFEFEULL) attacks |= (bit >> 15);
        if ((bit >> 10) & 0x3F3F3F3F3F3F3F3FULL) attacks |= (bit >> 10);
        if ((bit >> 15) & 0xFCFCFCFCFCFCFCFCULL) attacks |= (bit >> 6);

        knight_attacks[sq] = attacks;
    }
}

// ------------------- KNIGHT MOVES ----------------------
void generate_knight_moves(const Board &board, std::vector<Move> &moves) {
    Color us = board.side_to_move;
    Bitboard knights = board.pieces[us][KNIGHT];
    Bitboard own_pieces = board.occupied(us);

    while (knights) {
        int from = Bitboards::lsb(knights);
        Bitboards::clear_bit(knights, from);

        Bitboard attacks = knight_attacks[from] & ~own_pieces;

        while (attacks) {
            int to = Bitboards::lsb(attacks);
            moves.push_back({from, to, NO_PIECE});
            Bitboards::clear_bit(attacks, to);
        }
    }
}

// ------------------- BISHOP MOVES ----------------------
void generate_bishop_moves(const Board &board, std::vector<Move> &moves) {
    Color us = board.side_to_move;
    Bitboard bishops = board.pieces[us][BISHOP];
    Bitboard own_pieces = board.occupied(us);
    Bitboard occupied = board.occupied();

    const int directions[4] = {9, 7, -7, -9};

    while (bishops) {
        int from = Bitboards::lsb(bishops);
        Bitboards::clear_bit(bishops, from);

        // int from_rank = from / 8;
        // int from_file = from % 8;

        for (int dir : directions) {
            int to = from;

            while (true) {
                int to_rank = to / 8;
                int to_file = to % 8;

                // Compute new file and rank explicitly
                int new_rank = to_rank + ((dir > 0) ? 1 : -1);
                int new_file = to_file + ((dir == 9 || dir == -7) ? 1 : -1);

                // Check clearly: if off-board, stop.
                if (new_rank < 0 || new_rank > 7 || new_file < 0 || new_file > 7)
                    break;

                to = new_rank * 8 + new_file;

                // Check clearly: own piece blocking
                if (Bitboards::get_bit(own_pieces, to))
                    break;

                moves.push_back({from, to, NO_PIECE});

                // Check clearly: enemy piece blocking (capture and stop)
                if (Bitboards::get_bit(occupied, to))
                    break;
            }
        }
    }
}

// ------------------- ROOK MOVES ----------------------
void generate_rook_moves(const Board &board, std::vector<Move> &moves) {
    Color us = board.side_to_move;
    Bitboard rooks = board.pieces[us][ROOK];
    Bitboard own_pieces = board.occupied(us);
    Bitboard occupied = board.occupied();

    const int directions[4] = {8, -8, 1, -1};  // North, South, East, West

    while (rooks) {
        int from = Bitboards::lsb(rooks);
        Bitboards::clear_bit(rooks, from);

        // int from_rank = from / 8;
        // int from_file = from % 8;

        for (int dir : directions) {
            int to = from;

            while (true) {
                int to_rank = to / 8;
                int to_file = to % 8;

                int new_rank = to_rank;
                int new_file = to_file;

                if (dir == 8) new_rank += 1;        // North
                else if (dir == -8) new_rank -= 1;  // South
                else if (dir == 1) new_file += 1;   // East
                else if (dir == -1) new_file -= 1;  // West

                // Explicit boundary check
                if (new_rank < 0 || new_rank > 7 || new_file < 0 || new_file > 7)
                    break;

                to = new_rank * 8 + new_file;

                if (Bitboards::get_bit(own_pieces, to))
                    break;

                moves.push_back({from, to, NO_PIECE});

                if (Bitboards::get_bit(occupied, to)) // Stop after capture
                    break;
            }
        }
    }
}

// ------------------- QUEEN MOVES ----------------------
void generate_queen_moves(const Board &board, std::vector<Move> &moves) {
    Color us = board.side_to_move;
    Bitboard queens = board.pieces[us][QUEEN];
    Bitboard own_pieces = board.occupied(us);
    Bitboard occupied = board.occupied();

    const int directions[8] = {8, -8, 1, -1, 9, 7, -7, -9};  // Rook + Bishop directions

    while (queens) {
        int from = Bitboards::lsb(queens);
        Bitboards::clear_bit(queens, from);

        for (int dir : directions) {
            int to = from;

            while (true) {
                int to_rank = to / 8;
                int to_file = to % 8;

                int new_rank = to_rank;
                int new_file = to_file;

                if (dir == 8) new_rank += 1;          // North
                else if (dir == -8) new_rank -= 1;    // South
                else if (dir == 1) new_file += 1;     // East
                else if (dir == -1) new_file -= 1;    // West
                else if (dir == 9) { new_rank += 1; new_file += 1; }  // NE
                else if (dir == 7) { new_rank += 1; new_file -= 1; }  // NW
                else if (dir == -7) { new_rank -= 1; new_file += 1; } // SE
                else if (dir == -9) { new_rank -= 1; new_file -= 1; } // SW

                // Clearly check boundaries
                if (new_rank < 0 || new_rank > 7 || new_file < 0 || new_file > 7)
                    break;

                to = new_rank * 8 + new_file;

                if (Bitboards::get_bit(own_pieces, to))
                    break;

                moves.push_back({from, to, NO_PIECE});

                if (Bitboards::get_bit(occupied, to))
                    break;
            }
        }
    }
}

void init_king_attacks() {
    for (int sq = 0; sq < 64; sq++) {
        Bitboard attacks = EMPTY_BITBOARD;
        Bitboard bit = (1ULL << sq);

        // Set attacks for king around the square
        if ((bit << 8) != 0)  attacks |= (bit << 8);   // North
        if ((bit >> 8) != 0)  attacks |= (bit >> 8);   // South
        if ((bit << 1) & 0xFEFEFEFEFEFEFEFEULL) attacks |= (bit << 1); // East
        if ((bit >> 1) & 0x7F7F7F7F7F7F7F7FULL) attacks |= (bit >> 1); // West

        if ((bit << 9) & 0xFEFEFEFEFEFEFEFEULL) attacks |= (bit << 9); // NE
        if ((bit << 7) & 0x7F7F7F7F7F7F7F7FULL) attacks |= (bit << 7); // NW
        if ((bit >> 7) & 0x7F7F7F7F7F7F7F7FULL) attacks |= (bit >> 7); // SE
        if ((bit >> 9) & 0xFEFEFEFEFEFEFEFEULL) attacks |= (bit >> 9); // SW

        king_attacks[sq] = attacks;
    }
}

// ------------------- KING MOVES ----------------------
void generate_king_moves(const Board &board, std::vector<Move> &moves) {
    Color us = board.side_to_move;
    Bitboard kings = board.pieces[us][KING];
    Bitboard own_pieces = board.occupied(us);

    int from = Bitboards::lsb(kings); // There's only one king
    Bitboard attacks = king_attacks[from] & ~own_pieces;

    while (attacks) {
        int to = Bitboards::lsb(attacks);
        moves.push_back({from, to, NO_PIECE});
        Bitboards::clear_bit(attacks, to);
    }
}

void generate_castling_moves(const Board &board, std::vector<Move> &moves) {
    Color us = board.side_to_move;
    Bitboard occupied = board.occupied();

    if (us == WHITE) {
        // White Kingside castling
        if ((board.castling_rights & 1) &&
            !(occupied & (1ULL << 5 | 1ULL << 6))) {
            moves.push_back({4, 6, NO_PIECE}); // e1->g1
        }
        // White Queenside castling
        if (board.castling_rights & 2) {
            if (!(occupied & (1ULL << 1 | 1ULL << 2 | 1ULL << 3))) {
                moves.push_back({4, 2, NO_PIECE}); // e1->c1
            }
        }
    } else { // BLACK
        // Black Kingside castling
        if ((board.castling_rights & 4) &&
            !(board.occupied() & (1ULL << 57 | 1ULL << 58 | 1ULL << 59))) {
            moves.push_back({60, 62, NO_PIECE}); // e8->g8
        }
        // Black Queenside castling
        if ((board.castling_rights & 8) && !(board.occupied() & (1ULL << 61 | 1ULL << 62))) {
            moves.push_back({60, 58, NO_PIECE}); // e8->c8
        }
    }
}

bool is_square_attacked(const Board& board, int square, Color attacker) {
    Bitboard occupied = board.occupied();

    // Pawn attacks
    Bitboard pawns = board.pieces[attacker][PAWN];
    Bitboard pawn_attacks = (attacker == WHITE)
        ? ((pawns << 7) & 0x7F7F7F7F7F7F7F7FULL) | ((pawns << 9) & 0xFEFEFEFEFEFEFEFEULL)
        : ((pawns >> 7) & 0xFEFEFEFEFEFEFEFEULL) | ((pawns >> 9) & 0x7F7F7F7F7F7F7F7FULL);

    if (pawn_attacks & (1ULL << square)) return true;

    // Knight attacks
    Bitboard knights = board.pieces[attacker][KNIGHT];
    if (knight_attacks[square] & knights) return true;

    // King attacks
    Bitboard king = board.pieces[attacker][KING];
    if (king_attacks[square] & king) return true;

    // Bishop/Queen attacks (diagonals)
    Bitboard bishopsQueens = board.pieces[attacker][BISHOP] | board.pieces[attacker][QUEEN];
    const int bishop_dirs[4] = {9, 7, -7, -9};

    for (int dir : bishop_dirs) {
        int to = square;
        while (true) {
            int to_rank = to / 8;
            int to_file = to % 8;

            int new_rank = to_rank + ((dir > 0) ? 1 : -1);
            int new_file = to_file + ((dir == 9 || dir == -7) ? 1 : -1);

            if (new_rank < 0 || new_rank > 7 || new_file < 0 || new_file > 7) break;

            to = new_rank * 8 + new_file;

            if (Bitboards::get_bit(bishopsQueens, to)) return true;
            if (Bitboards::get_bit(occupied, to)) break;
        }
    }

    // Rook/Queen attacks (straight lines)
    Bitboard rooksQueens = board.pieces[attacker][ROOK] | board.pieces[attacker][QUEEN];
    const int rook_dirs[4] = {8, -8, 1, -1};

    for (int dir : rook_dirs) {
        int to = square;
        while (true) {
            int to_rank = to / 8;
            int to_file = to % 8;

            int new_rank = to_rank;
            int new_file = to_file;

            if (dir == 8) new_rank += 1;
            else if (dir == -8) new_rank -= 1;
            else if (dir == 1) new_file += 1;
            else if (dir == -1) new_file -= 1;

            if (new_rank < 0 || new_rank > 7 || new_file < 0 || new_file > 7) break;

            to = new_rank * 8 + new_file;

            if (Bitboards::get_bit(rooksQueens, to)) return true;

            if (Bitboards::get_bit(occupied, to)) break;
        }
    }

    return false;  // Not attacked by any piece
}

}
