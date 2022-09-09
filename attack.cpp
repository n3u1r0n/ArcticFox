#ifdef __INTELLISENSE__
  #pragma diag_suppress 28
#endif
#ifndef __ATTACK__MODULE__
#define __ATTACK__MODULE__


#include <array>
#include "base/base.cpp"
#include "magic.cpp"


/*

  Module to generate piece attacks.

*/


// generate the bishop and rook attack rays at compile time
namespace attack::ray {
constexpr std::array<bitboard_t, 64> bishop = {[]() constexpr {
  std::array<bitboard_t, 64> bishop{0ULL};
  for (auto square : square::all) {
    bishop[square] = 0ULL;
    for (int i = 1; i <= square % 8 && i <= square / 8; i++)
      bishop[square] |= (bitboard(square) >> (9 * i));
    for (int i = 1; i <= square % 8 && i <= 7 - (square / 8); i++)
      bishop[square] |= (bitboard(square) << (7 * i));
    for (int i = 1; i <= 7 - (square % 8) && i <= 7 - (square / 8); i++)
      bishop[square] |= (bitboard(square) << (9 * i));
    for (int i = 1; i <= 7 - (square % 8) && i <= square / 8; i++)
      bishop[square] |= (bitboard(square) >> (7 * i));
  };
  return bishop;
}()};

constexpr std::array<bitboard_t, 64> rook = {[]() constexpr {
  std::array<bitboard_t, 64> rook{0ULL};
  for (auto square : square::all) {
    rook[square] = 0ULL;
    for (int i = 1; i <= square % 8; i++)
      rook[square] |= (bitboard(square) >> i);
    for (int i = 1; i <= 7 - (square % 8); i++)
      rook[square] |= (bitboard(square) << i);
    for (int i = 1; i <= square / 8; i++)
      rook[square] |= (bitboard(square) >> (8 * i));
    for (int i = 1; i <= 7 - (square / 8); i++)
      rook[square] |= (bitboard(square) << (8 * i));
  };
  return rook;
}()};
};


// generate the pawn knight and king attack tables at compile time
namespace attack::table {
constexpr std::array<std::array<bitboard_t, 2>, 64> pawn = {[]() constexpr {
  std::array<std::array<bitboard_t, 2>, 64> pawn{bitboard::none};
  for (auto square : square::all) {
    pawn[square][color::white] = ((bitboard(square) >> 9) & ~bitboard::file_h) | ((bitboard(square) >> 7) & ~bitboard::file_a);
    pawn[square][color::black] = ((bitboard(square) << 9) & ~bitboard::file_a) | ((bitboard(square) << 7) & ~bitboard::file_h);
  };
  return pawn;
}()};

constexpr std::array<bitboard_t, 64> knight= {[]() constexpr {
  std::array<bitboard_t, 64> knight{bitboard::none};
  for (auto square : square::all) {
    knight[square] = ((bitboard(square) >>  6) & ~bitboard::file_a & ~bitboard::file_b & ~bitboard::rank_1) |
                     ((bitboard(square) <<  6) & ~bitboard::file_g & ~bitboard::file_h & ~bitboard::rank_8) |
                     ((bitboard(square) >> 10) & ~bitboard::file_g & ~bitboard::file_h & ~bitboard::rank_1) |
                     ((bitboard(square) << 10) & ~bitboard::file_a & ~bitboard::file_b & ~bitboard::rank_8) |
                     ((bitboard(square) >> 15) & ~bitboard::file_a & ~bitboard::rank_1 & ~bitboard::rank_2) |
                     ((bitboard(square) << 15) & ~bitboard::file_h & ~bitboard::rank_7 & ~bitboard::rank_8) |
                     ((bitboard(square) >> 17) & ~bitboard::file_h & ~bitboard::rank_1 & ~bitboard::rank_2) |
                     ((bitboard(square) << 17) & ~bitboard::file_a & ~bitboard::rank_7 & ~bitboard::rank_8);
  };
  return knight;
}()};

constexpr std::array<bitboard_t, 64> king = {[]() constexpr {
  std::array<bitboard_t, 64> king{bitboard::none};
  for (auto square : square::all) {
    king[square] = ((bitboard(square) >> 1) & ~bitboard::file_h) |
                    ((bitboard(square) << 1) & ~bitboard::file_a) |
                    ((bitboard(square) >> 8) & ~bitboard::rank_1) |
                    ((bitboard(square) << 8) & ~bitboard::rank_8) |
                    ((bitboard(square) >> 7) & ~bitboard::file_a & ~bitboard::rank_1) |
                    ((bitboard(square) << 7) & ~bitboard::file_h & ~bitboard::rank_8) |
                    ((bitboard(square) >> 9) & ~bitboard::file_h & ~bitboard::rank_1) |
                    ((bitboard(square) << 9) & ~bitboard::file_a & ~bitboard::rank_8);
  };
  return king;
}()};
};


// access to all the piece attacks
namespace attack {
template <piece_t piece>
bitboard_t attack(square_t square, bitboard_t occupancy=bitboard::none) {
  if constexpr (piece == piece::white_pawn) {
    return table::pawn[square][color::white];
  } else if constexpr (piece == piece::black_pawn) {
    return table::pawn[square][color::black];
  } else if constexpr (piece == piece::knight || piece == piece::white_knight || piece == piece::black_knight) {
    return table::knight[square];
  } else if constexpr (piece == piece::bishop || piece == piece::white_bishop || piece == piece::black_bishop) {
    return magic::table[
      magic::bishop[square].offset +
      (((occupancy | magic::bishop[square].mask) * magic::bishop[square].magic_number) >> 55)
    ];
  } else if constexpr (piece == piece::rook || piece == piece::white_rook || piece == piece::black_rook) {
    return magic::table[
      magic::rook[square].offset +
      (((occupancy | magic::rook[square].mask) * magic::rook[square].magic_number) >> 52)
    ];
  } else if constexpr (piece == piece::queen || piece == piece::white_queen || piece == piece::black_queen) {
    return (
      magic::table[
        magic::bishop[square].offset +
        (((occupancy | magic::bishop[square].mask) * magic::bishop[square].magic_number) >> 55)
      ] |
      magic::table[
        magic::rook[square].offset +
        (((occupancy | magic::rook[square].mask) * magic::rook[square].magic_number) >> 52)
      ]
    );
  } else if constexpr (piece == piece::king || piece == piece::white_king || piece == piece::black_king) {
    return table::king[square];
  } else {
    return bitboard::none;
  };
};
};


#endif