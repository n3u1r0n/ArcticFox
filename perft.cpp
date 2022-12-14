#pragma once

#include <iostream>
#include "modules/time.cpp"
#include "movegen/movegen.cpp"
#include "base.cpp"
#include "board.cpp"

/**********************************************************************
 * 
 * Module for perft testing.
 * 
**********************************************************************/

struct perft_result_t {
  u64_t nodes;
  float time;
  float mnps;
};

// perft function optimized for speed
template<color_t color, movetype_t movetype=legal>
u64_t _perft(Board& board, int depth) {
  constexpr color_t opponent = opponent(color);
  if (depth == 0) return 1;
  if (depth == 1) return generate<color, movetype, u64_t>(board);
  if (depth == 2) {
    move_stack_t legal_moves = generate<color, legal, move_stack_t>(board);
    u64_t nodes = 0;
    for (move_t move : legal_moves) {
      board.make<color>(move);
      nodes += generate<opponent, movetype, u64_t>(board);
      board.unmake<color>();
    };
    return nodes;
  };
  move_stack_t legal_moves = generate<color, legal, move_stack_t>(board);
  u64_t nodes = 0;
  for (move_t move : legal_moves) {
    board.make<color>(move);
    nodes += _perft<opponent, movetype>(board, depth - 1);
    board.unmake<color>();
  };
  return nodes;
};

// perft function with printing
template<color_t color, movetype_t movetype=legal>
perft_result_t _perft(Board& board, int depth, bool print) {
  constexpr color_t opponent = opponent(color);
  u64_t start_time = nanoseconds();
  u64_t nodes = 0;
  move_stack_t legal_moves = generate<color, legal, move_stack_t>(board);
  move_stack_t moves = generate<color, movetype, move_stack_t>(board);
  for (move_t move : legal_moves) {
    u64_t local_nodes = 0;
    if (moves.contains(move) || depth != 1) {
      board.make<color>(move);
      local_nodes = _perft<opponent, movetype>(board, depth - 1);
      board.unmake<color>();
    };
    nodes += local_nodes;
    if (print)
      std::cout << move_to_string(move) << ": " << local_nodes << "\n";
  };
  u64_t end_time = nanoseconds();
  float time = (float)(end_time - start_time) * 1e-9;
  float mnps = (nodes / time) * 1e-6;
  if (print) {
    std::cout << "\n";
    std::cout << "Nodes searched: " << nodes << "\n";
    std::cout << "Total time: " << time << " s\n";
    std::cout << "MNps: " << mnps << "\n";
  };
  return perft_result_t{ nodes, time, mnps };
};

// wrapper function for perft
template<movetype_t movetype=legal>
perft_result_t perft(Board& board, int depth, bool print=true) {
  if (depth == 0) return perft_result_t{ 0, 0, 0 };
  if (board.turn == white) {
    return _perft<white, movetype>(board, depth, print);
  } else {
    return _perft<black, movetype>(board, depth, print);
  };
};