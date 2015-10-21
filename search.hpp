#pragma once
#include <vector>
#include <utility>
#include "board.hpp"
#include "play.hpp"

int score(const Board &);
struct Game {
  int val;
  State st;
  std::vector<pos> history;
  Game()
    : val(0), st(), history() {}
  Game(const State &st, const std::vector<pos> &his)
    : val(score(st.bd)), st(st), history(his) {}
  Game(const State &st)
    : val(score(st.bd)), st(st), history() {}
  ~Game() = default;
};
inline bool operator<(const Game &lhs, const Game &rhs) {
  return lhs.val > rhs.val;
}

std::vector<pos> find_answer(const State &);
std::vector<pos> random_walk(const State &);
std::vector<pos> beam_search(const State &);
std::vector<pos> astar_search(const State &);
