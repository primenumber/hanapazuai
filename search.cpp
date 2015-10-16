#include "search.hpp"
#include <iostream>
#include <unordered_map>
#include <boost/optional.hpp>

enum class Type {
  NONE,
  GOAL,
  DFS
};

Type need_dfs(const State &st, int rem,
    std::unordered_map<Board, int> &memo,
    std::unordered_map<Board, int> &all_memo) {
  if (rem < st.score) return Type::NONE;
  if (st.bd.is_goal()) return Type::GOAL;
  auto itr = all_memo.find(st.bd);
  if (itr != std::end(all_memo) && itr->second < st.score) return Type::NONE;
  itr = memo.find(st.bd);
  if (itr != std::end(memo) && itr->second <= st.score) return Type::NONE;
  all_memo[st.bd] = memo[st.bd] = st.score;
  return Type::DFS;
}
boost::optional<std::tuple<std::vector<pos>, int>> dfs(const State &st, int rem,
    std::unordered_map<Board, int> &memo,
    std::unordered_map<Board, int> &all_memo) {
  switch(need_dfs(st, rem, memo, all_memo)) {
    case Type::NONE: return boost::none;
    case Type::GOAL: return std::make_tuple(std::vector<pos>(), st.score);
    default:;
  }
  int min_score = 100000000;
  std::vector<pos> res;
  for (auto next : next_states(st)) {
    State nx;
    pos p;
    std::tie(nx, p) = next;
    if (auto opt_ans = dfs(nx, rem, memo, all_memo)) {
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
    if (auto opt_ans = dfs(st, i, memo, all_memo)) {
      auto res = std::get<0>(*opt_ans);
      std::reverse(std::begin(res), std::end(res));
      return res;
    }
  }
  return std::vector<pos>();
}
