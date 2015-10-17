#include "search.hpp"
#include <iostream>
#include <unordered_map>
#include <boost/optional.hpp>

using units_t = std::vector<Unit>;
using map_t = std::unordered_map<units_t, int>;
using set_t = std::unordered_set<units_t>;
enum class Type {
  NONE,
  GOAL,
  DFS
};

table_t<bool> can_lift_pos(const b_ary &ba) {
  table_t<bool> res;
  for (int j = 0; j < 14; ++j) res[9][j] = false;
  for (int i = 8; i >= 0; --i) {
    res[i][0] = false;
    for (int j = 1; j < 14; ++j) {
      if (ba[i][j] == '.') {
        res[i][j] = res[i][j-1];
        res[i][j] |= (ba[i-1][j] == '.');
      } else res[i][j] = false;
    }
    for (int j = 12; j >= 0; --j) {
      if (ba[i][j] == '.') {
        res[i][j] |= res[i][j+1];
      }
    }
  }
  return res;
}
std::array<std::vector<Seed>, 3> flowers(const units_t &units) {
  std::array<std::vector<Seed>, 3> res;
  for (const Unit &u : units) {
    if (!u.is_seed()) continue;
    res[u.seed.color].emplace_back(u.seed);
  }
  return res;
}
std::array<int, 3> lowest_flower(const b_ary &tb) {
  std::array<int, 3> res = {0, 0, 0};
  for (int i = 0; i < 10; ++i)
    for (int j = 0; j < 14; ++j)
      if (char2clr(tb[i][j]) >= 0)
        res[char2clr(tb[i][j])] = i;
  return res;
}
bool can_goal(const Board &bd) {
  auto can_lift_pos_table = can_lift_pos(bd.tb);
  auto flower_ary = flowers(bd.units);
  std::array<int, 3> lowest_f_pos = lowest_flower(bd.tb);
  std::array<int, 3> potential_need = {0, 0, 0};
  std::array<int, 3> potential_have = {0, 0, 0};
  for (int i = 0; i < 3; ++i) {
    if (flower_ary[i].size() == 1) {
      Seed s = flower_ary[i].front();
      if (s.is_bloomed) continue;
      if (lowest_f_pos[i]+1 < s.y) {
        if (!can_lift_pos_table[s.y][s.x]) {
          return false;
        } else {
          potential_need[i] = std::max(potential_need[i], s.y - lowest_f_pos[i] - 1);
        }
      }
      for (int j = 1; j < 3; ++j)
        ++potential_have[(i+j)%3];
    } else {
      for (const Seed &s : flower_ary[i])
        if (!s.is_bloomed)
          for (int j = 1; j < 3; ++j)
            ++potential_have[(i+j)%3];
    }
  }
  for (int i = 0; i < 3; ++i)
    if (potential_need[i] > potential_have[i])
      return false;
  return true;
}
Type need_dfs(const State &st, int rem,
    map_t &memo,
    map_t &all_memo) {
  if (rem < st.score) return Type::NONE;
  if (st.bd.is_goal()) return Type::GOAL;
  if (!can_goal(st.bd)) return Type::NONE;
  const units_t &units = st.bd.units;
  auto itr = all_memo.find(units);
  if (itr != std::end(all_memo) && itr->second < st.score) return Type::NONE;
  itr = memo.find(units);
  if (itr != std::end(memo) && itr->second <= st.score) return Type::NONE;
  all_memo[units] = memo[units] = st.score;
  return Type::DFS;
}
boost::optional<std::tuple<std::vector<pos>, int>> dfs(const State &st, int rem,
    map_t &memo, map_t &all_memo) {
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
      int score;
      std::vector<pos> vec;
      std::tie(vec, score) = *std::move(opt_ans);
      if (score < min_score) {
        vec.emplace_back(p);
        res = vec;
        min_score = score;
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
  map_t all_memo;
  int old_nodes = 1;
  int diff = 30;
  for (int i = 30; i <= 2000; i += diff) {
    map_t memo;
    std::cerr << i << std::endl;
    if (auto opt_ans = dfs(st, i, memo, all_memo)) {
      auto res = std::get<0>(*opt_ans);
      std::reverse(std::begin(res), std::end(res));
      std::cerr << "node: " << memo.size() << std::endl;
      return res;
    }
    std::cerr << "node: " << memo.size() << std::endl;
    if (memo.size() / old_nodes < 2)
      diff += 30;
    old_nodes = memo.size();
  }
  return std::vector<pos>();
}
