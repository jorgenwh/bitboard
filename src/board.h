#pragma once
#include "types.h"
#include "types.h"
#include <array>
#include <iostream>

struct Board {
    // Array of bitboards [color][piece] representing positions.
    Bitboard pieces[COLOR_NB][PIECE_NB];

    // Side to move (WHITE or BLACK)
    Color side_to_move;

    // Castling rights (bit flags: 1=White-K, 2=White-Q, 4=Black-K, 8=Black-Q)
    int castling_rights;

    // En passant square (-1 if none, otherwise 0-63)
    int en_passant_square;

    // Constructor
    Board();

    // Initialize board to standard chess starting position
    void init_startpos();

    // Make a move on the board
    bool make_move(const Move& move);

    // Get occupied squares for one color
    Bitboard occupied(Color c) const;

    // Get occupied squares (both colors)
    Bitboard occupied() const;

    // Update castling rights after a move
    void update_castling_rights(int from_square);

    // Print board to console
    void print() const;
};
