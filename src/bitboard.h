#pragma once
#include "types.h"

namespace Bitboards {

void set_bit(Bitboard &bb, int square);
void clear_bit(Bitboard &bb, int square);
bool get_bit(Bitboard bb, int square);
int popcount(Bitboard bb);
int lsb(Bitboard bb); // Least significant bit index

} // namespace Bitboards
