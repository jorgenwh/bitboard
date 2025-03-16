#include "bitboard.h"
#include <bit>

namespace Bitboards {

void set_bit(Bitboard &bb, int square) {
    bb |= (1ULL << square);
}

void clear_bit(Bitboard &bb, int square) {
    bb &= ~(1ULL << square);
}

bool get_bit(Bitboard bb, int square) {
    return (bb & (1ULL << square)) != 0;
}

int popcount(Bitboard bb) {
    return std::popcount(bb); // C++20 built-in, efficient
}

int lsb(Bitboard bb) {
    if (bb == 0ULL) return -1;
    return std::countr_zero(bb); // returns index of least significant bit set
}

} // namespace Bitboards
