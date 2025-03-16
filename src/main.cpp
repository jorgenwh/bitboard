// main.cpp
#include "board.h"
#include "movegen.h"
#include "types.h"
#include "bitboard.h"
#include <iostream>

std::string square_to_string(int square) {
    char file = 'a' + (square % 8);
    char rank = '1' + (square / 8);
    return std::string{file, rank};
}

std::string piece_to_string(Piece piece) {
    switch (piece) {
        case PAWN: return "Pawn";
        case KNIGHT: return "Knight";
        case BISHOP: return "Bishop";
        case ROOK: return "Rook";
        case QUEEN: return "Queen";
        case KING: return "King";
        default: return "No piece";
    }
}

int main() {
    MoveGen::init_knight_attacks();
    MoveGen::init_king_attacks();

    Board board;
    board.print();

    // Setup custom position for testing bishop at center (d4)
    for (int c = WHITE; c <= BLACK; ++c)
        for (int p = PAWN; p < PIECE_NB; ++p)
            board.pieces[c][p] = EMPTY_BITBOARD;

    board.update_castling_rights(4); // remove white castling rights
    board.update_castling_rights(60); // remove black castling rights

    // place rook on e1
    board.pieces[BLACK][ROOK] = 1ULL << 4;
    // place white king on d4
    board.pieces[WHITE][KING] = 1ULL << 27;
    board.print();

    auto moves = MoveGen::generate_moves(board);

    std::cout << "\nTotal moves generated: " << moves.size() << "\n";
    for (const auto &m : moves) {
        std::cout << "Move: "
                  << square_to_string(m.from) << " -> "
                  << square_to_string(m.to) << " : "
                    << piece_to_string(m.promotion) << "\n";
    }

    return 0;
}
