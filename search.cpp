#include "search.hpp"
#include <iostream>
#include <unordered_map>
#include <boost/optional.hpp>

boost::optional<std::vector<pos>> dfs(const State &st, int rem, const Board &prev) {
  if (st.bd.is_goal()) return std::vector<pos>();
  if (rem < st.score) return boost::none;
  for (auto next : next_states(st)) {
    State nx;
    pos p;
    std::tie(nx, p) = next;
    if (nx.bd == prev) continue;
    if (auto opt_ans = dfs(nx, rem, st.bd)) {
      opt_ans->emplace_back(p);
      return *opt_ans;
    }
  }
  return boost::none;
}
std::vector<pos> find_answer(const State &st) {
  for (int i = 10; i <= 2000; i += 10) {
    std::cerr << i << std::endl;
    if (auto opt_ans = dfs(st, i, Board())) {
      std::reverse(std::begin(*opt_ans), std::end(*opt_ans));
      return *opt_ans;
    }
  }
  return std::vector<pos>();
}
