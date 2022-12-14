#pragma once

#include <iostream>
#include "modules/time.cpp"
#include "base.cpp"
#include "board.cpp"
#include "evaluation.cpp"
#include "transposition.cpp"

/***********************************************************************
 * 
 * Module for the search algorithm.
 * 
***********************************************************************/

struct search_result_t {
  pv_t pv;
  score_t score;
};

// do quiescence search
template <color_t color>
score_t q_search(Board& board, int depth, score_t alpha, score_t beta, u64_t& nodes) {
  constexpr color_t opponent = opponent(color);
  if (depth == 0) {
    nodes++;
    return evaluate<color>(board);
  };
  score_t score = evaluate<color>(board);
  if (score >= beta) {
    nodes++;
    return beta;
  };
  if (alpha < score)
    alpha = score;
  move_stack_t moves = generate<color, check | capture, move_stack_t>(board);
  moves.sort(comparison);
  for (move_t move : moves) {
    board.make<color>(move);
    score = add_depth(q_search<opponent>(board, depth - 1, remove_depth(beta), remove_depth(alpha), nodes));
    board.unmake<color>();
    if (score >= beta)
      return beta;
    if (score > alpha)
      alpha = score;
  };
  return alpha;
};

// do search optimized for current color
template <color_t color>
search_result_t search(Board board, int depth, score_t alpha, score_t beta, pv_t old_pv, u64_t& tbhits, u64_t& nodes) {
  constexpr color_t opponent = opponent(color);
  if (depth == 0)
    return search_result_t {pv_t {}, q_search<color>(board, MAX_QSEARCH_DEPTH, alpha, beta, nodes)};
  if (board.position_existed()) {
    ++nodes;
    return search_result_t {pv_t {}, draw};
  };
  pv_t pv {};
  entry_t& entry = get_entry(board.zobrist.hash);
  if (entry.is_valid(board.zobrist.hash, depth)) {
    tbhits++;
    u8_t bound = entry.get_bound();
    score_t entry_score = entry.get_score();
    if (bound == exact_bound) {
      pv.push(entry.move);
      ++nodes;
      return search_result_t {pv, entry_score};
    } else if (bound == upper_bound && beta > entry_score) {
      beta = entry_score;
    } else if (bound == lower_bound && alpha < entry_score) {
      alpha = entry_score;
    };
    if (alpha >= beta) {
      ++nodes;
      return search_result_t {pv, entry_score};
    };
  };
  move_stack_t legal_moves = generate<color, legal, move_stack_t>(board);
  legal_moves.sort(reverse_comparison);
  if (old_pv.size() > 0) {
    move_t move = old_pv.pop();
    if (legal_moves.contains(move, reverse_comparison))
      legal_moves.push(move);
    else
      old_pv.clear();
  };
  if (legal_moves.contains(entry.move, reverse_comparison))
    legal_moves.push(entry.move);
  legal_moves.reverse();
  u8_t bound = upper_bound;
  for (move_t move : legal_moves) {
    board.make<color>(move);
    search_result_t search_result = search<opponent>(board, depth - 1, remove_depth(beta), remove_depth(alpha), old_pv, tbhits, nodes);
    board.unmake<color>();
    search_result.score = add_depth(search_result.score);
    if (search_result.score > alpha) {
      pv = search_result.pv.copy();
      pv.push(move);
      if (search_result.score >= beta) {
        entry.set(board.zobrist.hash, move, search_result.score, depth, lower_bound);
        return search_result_t {pv, beta};
      };
      alpha = search_result.score;
      bound = exact_bound;
    };
  };
  entry.set(board.zobrist.hash, pv[0], alpha, depth, bound);
  return search_result_t {pv, alpha};
};

// do search
search_result_t search(Board& board, int depth) {
  search_result_t search_result;
  pv_t pv {};
  for (int i = 1; i <= depth; i++) {
    u64_t tbhits = 0;
    u64_t nodes = 0;
    u64_t start_time = milliseconds();
    if (board.turn == white) {
      search_result = search<white>(board, i, -inf, inf, pv, tbhits, nodes);
    } else {
      search_result = search<black>(board, i, -inf, inf, pv, tbhits, nodes);
      search_result.score = -search_result.score;
    };
    u64_t end_time = milliseconds();
    pv = search_result.pv.copy();
    search_result.pv.reverse();
    u64_t nps = nodes / ((end_time - start_time) / 1000.0);
    std::cout << "info depth " << i
              << " score cp " << search_result.score
              << " time " << end_time - start_time
              << " tbhits " << tbhits
              << " nodes " << nodes
              << " nps " << nps
              << " string current bestmove " << move_to_string(search_result.pv[0]) << "\n";
  };
  return search_result;
};