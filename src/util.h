#pragma once

#include "types.h"

#include <string>

inline std::string square_to_string(int square) {
    char file = 'a' + (square % 8);
    char rank = '1' + (square / 8);
    return std::string{file, rank};
}

inline std::string piece_to_string(Piece piece) {
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
