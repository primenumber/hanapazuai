#include "search.hpp"
#include <iostream>
#include <unordered_map>
#include <boost/optional.hpp>

boost::optional<std::vector<pos>> dfs(const State &st, int rem,
    const Board &prev, std::unordered_map<Board, int> &memo) {
  if (st.bd.is_goal()) return std::vector<pos>();
  if (rem < st.score) return boost::none;
  auto itr = memo.find(st.bd);
  if (itr != std::end(memo) && itr->second <= st.score) return boost::none;
  memo[st.bd] = st.score;
  for (auto next : next_states(st)) {
    State nx;
    pos p;
    std::tie(nx, p) = next;
    if (nx.bd == prev) continue;
    if (auto opt_ans = dfs(nx, rem, st.bd, memo)) {
      opt_ans->emplace_back(p);
      return *opt_ans;
    }
  }
  return boost::none;
}
std::vector<pos> find_answer(const State &st) {
  for (int i = 30; i <= 2000; i += 30) {
    std::unordered_map<Board, int> memo;
    std::cerr << i << std::endl;
    if (auto opt_ans = dfs(st, i, Board(), memo)) {
      std::reverse(std::begin(*opt_ans), std::end(*opt_ans));
      return *opt_ans;
    }
  }
  return std::vector<pos>();
}
