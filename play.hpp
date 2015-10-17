#pragma once
#include <boost/operators.hpp>
#include <boost/optional.hpp>
#include "board.hpp"

struct State {
  Board bd;
  int score;
  State() = default;
  State(const Board &bd, const int score = 0)
    : bd(bd), score(score) {}
  State(Board &&bd, const int score = 0)
    : bd(std::move(bd)), score(score) {}
  State(const State &) = default;
  State(State &&) = default;
  State &operator=(const State &) = default;
  State &operator=(State &&) = default;
};

struct pos : public boost::operators<pos> {
  int x, y;
  pos() = default;
  pos(int x, int y) : x(x), y(y) {}
  pos(const pos &) = default;
  pos &operator=(const pos &) = default;
};
inline bool operator==(const pos &lhs, const pos &rhs) {
  return lhs.x == rhs.x && lhs.y == rhs.y;
}
inline bool operator<(const pos &lhs, const pos &rhs) {
  return lhs.x == rhs.x ? lhs.y < rhs.y : lhs.x < rhs.x;
}

// (next_board, elapsed_time)
boost::optional<State> try_move(const State &, int x, int y);
std::vector<std::tuple<State, pos>> next_states(const State &);
State simulate(const State &, const std::vector<pos> &);
