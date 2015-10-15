#pragma once
#include <boost/optional.hpp>
#include "board.hpp"

struct State {
  Board bd;
  int score;
  State(const Board &bd, const int score = 0)
    : bd(bd), score(score) {}
  State(Board &&bd, const int score = 0)
    : bd(std::move(bd)), score(score) {}
  State(const State &) = default;
  State(State &&) = default;
  State &operator=(const State &) = default;
  State &operator=(State &&) = default;
};

//bool is_movable(const Board &, int x, int y);
// (next_board, elapsed_time)
boost::optional<State> try_move(const State &, int x, int y);
//std::tuple<Board, int> move(const Board &, int x, int y);
