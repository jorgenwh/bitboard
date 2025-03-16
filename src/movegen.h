#pragma once

#include "board.h"
#include "types.h"
#include "util.h"

#include <vector>

namespace MoveGen {
    uint64_t perft(const Board& board, int depth);

    std::vector<Move> generate_legal_moves(const Board& board);

    // Individual move generators
    void generate_pawn_moves(const Board& board, std::vector<Move>& moves);
    void generate_knight_moves(const Board& board, std::vector<Move>& moves);
    void generate_bishop_moves(const Board& board, std::vector<Move>& moves);
    void generate_rook_moves(const Board& board, std::vector<Move>& moves);
    void generate_queen_moves(const Board& board, std::vector<Move>& moves);
    void generate_king_moves(const Board& board, std::vector<Move>& moves);
    void generate_castling_moves(const Board& board, std::vector<Move>& moves);

    bool is_square_attacked(const Board& board, int square, Color attacker);

    // To initialize knight attacks lookup table
    void init_knight_attacks();
    void init_king_attacks();

    extern Bitboard knight_attacks[64];
    extern Bitboard king_attacks[64];
}
