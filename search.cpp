#include "search.hpp"
#include <iostream>
#include <boost/optional.hpp>

boost::optional<std::vector<pos>> dfs(const State &st, int rem) {
  if (st.bd.is_goal()) return std::vector<pos>();
  if (rem < st.score) return boost::none;
  for (auto next : next_states(st)) {
    State nx;
    pos p;
    std::tie(nx, p) = next;
    if (auto opt_ans = dfs(nx, rem)) {
      opt_ans->emplace_back(p);
      return *opt_ans;
    }
  }
  return boost::none;
}
std::vector<pos> find_answer(const State &st) {
  for (int i = 10; i <= 2000; i += 10) {
    std::cerr << i << std::endl;
    if (auto opt_ans = dfs(st, i)) {
      return *opt_ans;
    }
  }
  return std::vector<pos>();
}
