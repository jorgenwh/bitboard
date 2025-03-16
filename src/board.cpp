#include "board.h"

// Constructor initializes to start position
Board::Board() {
    init_startpos();
}

// Initialize standard chess starting position
void Board::init_startpos() {
    // Pawns
    pieces[WHITE][PAWN]   = 0x000000000000FF00ULL;
    pieces[BLACK][PAWN]   = 0x00FF000000000000ULL;

    // Knights
    pieces[WHITE][KNIGHT] = 0x0000000000000042ULL;
    pieces[BLACK][KNIGHT] = 0x4200000000000000ULL;

    // Bishops
    pieces[WHITE][BISHOP] = 0x0000000000000024ULL;
    pieces[BLACK][BISHOP] = 0x2400000000000000ULL;

    // Rooks
    pieces[WHITE][ROOK]   = 0x0000000000000081ULL;
    pieces[BLACK][ROOK]   = 0x8100000000000000ULL;

    // Queens
    pieces[WHITE][QUEEN]  = 0x0000000000000008ULL;
    pieces[BLACK][QUEEN]  = 0x0800000000000000ULL;

    // Kings
    pieces[WHITE][KING]   = 0x0000000000000010ULL;
    pieces[BLACK][KING]   = 0x1000000000000000ULL;

    side_to_move = WHITE;

    // All castling rights initially available
    castling_rights = 0b1111;

    // No en passant square at start
    en_passant_square = -1;
}

// Make a move on the board
bool Board::make_move(const Move &move) {
    Color us = side_to_move;
    Color them = (us == WHITE) ? BLACK : WHITE;

    Piece moved_piece = NO_PIECE;

    // Find moved piece
    for (int p = PAWN; p < PIECE_NB; ++p) {
        if (Bitboards::get_bit(pieces[us][p], move.from)) {
            moved_piece = static_cast<Piece>(p);
            Bitboards::clear_bit(pieces[us][moved_piece], move.from);
            break;
        }
    }

    // Handle en passant captures FIRST clearly:
    if (moved_piece == PAWN && move.to == en_passant_square) {
        // Explicitly calculate the captured pawn's square
        int ep_captured_square = move.to + ((us == WHITE) ? -8 : 8);
        Bitboards::clear_bit(pieces[them][PAWN], ep_captured_square);
    } else {
        // Regular captures explicitly handled
        for (int p = PAWN; p < PIECE_NB; ++p) {
            if (Bitboards::get_bit(pieces[them][p], move.to)) {
                Bitboards::clear_bit(pieces[them][p], move.to);
                break;
            }
        }
    }

    // Set moved piece clearly (handle promotions explicitly too)
    Piece final_piece = move.promotion == NO_PIECE ? moved_piece : move.promotion;
    Bitboards::set_bit(pieces[us][final_piece], move.to);

    // Clearly update castling rights
    update_castling_rights(move.from);
    update_castling_rights(move.to);

    // En passant square clearly updated:
    if (moved_piece == PAWN && abs(move.to - move.from) == 16) {
        en_passant_square = (move.from + move.to) / 2;
    } else {
        en_passant_square = -1;
    }

    side_to_move = them;
    return true;
}

// Return occupied squares by a specific color
Bitboard Board::occupied(Color c) const {
    Bitboard occ = EMPTY_BITBOARD;
    for (int p = PAWN; p < PIECE_NB; p++)
        occ |= pieces[c][p];
    return occ;
}

// Return occupied squares for both colors
Bitboard Board::occupied() const {
    return occupied(WHITE) | occupied(BLACK);
}

void Board::update_castling_rights(int from_square) {
    switch (from_square) {
        case 4:  // White king moves
            castling_rights &= ~0b0011; // Remove white kingside & queenside rights
            break;
        case 60: // Black king moves from e8
            castling_rights &= ~0b1100; // Remove black castling rights
            break;
        case 0: // White rook on a1 moves
            castling_rights &= ~0b0010; // Remove white queenside
            break;
        case 7: // White rook on h1 moves
            castling_rights &= ~0b0001; // Remove white kingside
            break;
        case 56: // Black rook on a8 moves
            castling_rights &= ~0b1000; // Remove black queenside
            break;
        case 63: // Black rook on h8 moves
            castling_rights &= ~0b0100; // Remove black kingside
            break;
    }
}

// Print board to console
void Board::print() const {
    const char piece_chars[COLOR_NB][PIECE_NB] = {
        { '.', 'P', 'N', 'B', 'R', 'Q', 'K' }, // WHITE pieces
        { '.', 'p', 'n', 'b', 'r', 'q', 'k' }  // BLACK pieces
    };

    for (int rank = 7; rank >= 0; rank--) {
        std::cout << (rank + 1) << " ";
        for (int file = 0; file < 8; file++) {
            int sq = rank * 8 + file;
            char piece_char = '.';

            // Find which piece is at this square (if any)
            for (int color = WHITE; color <= BLACK; color++) {
                for (int piece = PAWN; piece < PIECE_NB; piece++) {
                    if (Bitboards::get_bit(pieces[color][piece], sq)) {
                        piece_char = piece_chars[color][piece];
                    }
                }
            }
            std::cout << piece_char << " ";
        }
        std::cout << "\n";
    }
    std::cout << "  a b c d e f g h\n";
    std::cout << (side_to_move == WHITE ? "White" : "Black") << " to move.\n";
}
