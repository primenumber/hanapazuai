#include "search.hpp"
#include <iostream>
#include <unordered_map>
#include <boost/optional.hpp>

boost::optional<std::tuple<std::vector<pos>, int>> dfs(const State &st, int rem,
    const Board &prev, std::unordered_map<Board, int> &memo,
    std::unordered_map<Board, int> &all_memo) {
  if (rem < st.score) return boost::none;
  if (st.bd.is_goal()) return std::make_tuple(std::vector<pos>(), st.score);
  auto itr = all_memo.find(st.bd);
  if (itr != std::end(all_memo) && itr->second < st.score) return boost::none;
  itr = memo.find(st.bd);
  if (itr != std::end(memo) && itr->second <= st.score) return boost::none;
  all_memo[st.bd] = memo[st.bd] = st.score;
  int min_score = 100000000;
  std::vector<pos> res;
  for (auto next : next_states(st)) {
    State nx;
    pos p;
    std::tie(nx, p) = next;
    if (nx.bd == prev) continue;
    if (auto opt_ans = dfs(nx, rem, st.bd, memo, all_memo)) {
      if (std::get<1>(*opt_ans) < min_score) {
        std::get<0>(*opt_ans).emplace_back(p);
        res = std::get<0>(*opt_ans);
        min_score = std::get<1>(*opt_ans);
      }
    }
  }
  if (min_score < 100000000) {
    return std::make_tuple(res, min_score);
  } else {
    return boost::none;
  }
}
std::vector<pos> find_answer(const State &st) {
  std::unordered_map<Board, int> all_memo;
  for (int i = 30; i <= 2000; i += 30) {
    std::unordered_map<Board, int> memo;
    std::cerr << i << std::endl;
    if (auto opt_ans = dfs(st, i, Board(), memo, all_memo)) {
      auto res = std::get<0>(*opt_ans);
      std::reverse(std::begin(res), std::end(res));
      return res;
    }
  }
  return std::vector<pos>();
}
