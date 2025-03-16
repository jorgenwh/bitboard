// main.cpp
#include "types.h"
#include "util.h"
#include "board.h"
#include "movegen.h"
#include "bitboard.h"

#include <iostream>

int main() {
    MoveGen::init_knight_attacks();
    MoveGen::init_king_attacks();

    Board board;
    board.init_startpos();

    for (int depth = 1; depth <= 4; depth++) {
        uint64_t nodes = MoveGen::perft(board, depth);

        std::cout << "Depth: " << depth << " Total nodes: " << nodes << "\n";
    }

    return 0;
}
