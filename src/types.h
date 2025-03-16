#pragma once
#include <cstdint>

using Bitboard = uint64_t;

enum Piece {
    NO_PIECE,
    PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING,
    PIECE_NB
};

struct Move {
    int from;
    int to;
    Piece promotion;  // NO_PIECE if not promotion
};

enum Color {
    WHITE, BLACK, COLOR_NB
};

constexpr Bitboard EMPTY_BITBOARD = 0ULL;
constexpr Bitboard FULL_BITBOARD  = ~0ULL;
