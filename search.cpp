#include "search.hpp"
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <queue>
#include <unordered_map>
#include <unordered_set>
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
int inf_dist(const Board &);
int score(const units_t &units) {
  int count = 0;
  int pot = 0;
  for (auto &u : units) {
    if (u.is_seed()) {
      if (u.seed.is_bloomed)
        ++count;
      else
        pot += u.y();
    }
  }
  return count * 35 - pot * 10;
}
int score(const Board &bd) {
  return score(bd.units) - inf_dist(bd);
}
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
bool operator<(const Game &lhs, const Game &rhs) {
  return lhs.val > rhs.val;
}
std::mutex m, n, r;
void calc_next(
    int i, int thread_num,
    const std::vector<Game> &beam,
    std::vector<Game> &nexts,
    set_t &memo,
    std::vector<std::vector<pos>> &ans) {
  for (int j = i; j < (int)beam.size(); j += thread_num) {
    const Game &g = beam[j];
    for (auto next : next_states(g.st)) {
      State nx;
      pos p;
      std::tie(nx, p) = next;
      std::unique_lock<std::mutex> lock(m);
      if (memo.count(nx.bd.units) != 0) continue;
      memo.insert(nx.bd.units);
      lock.unlock();
      auto his = g.history;
      his.push_back(p);
      if (nx.bd.is_goal()) {
        std::unique_lock<std::mutex> gl(r);
        ans.emplace_back(his);
      }
      std::unique_lock<std::mutex> lock2(n);
      nexts.emplace_back(nx, his);
    }
  }
}
std::vector<pos> beam_search(const State &st) {
  set_t memo;
  std::vector<Game> beam(1, Game(st));
  constexpr int MAX_BEAM = 30000;
  std::vector<std::vector<pos>> ans;
  while (!beam.empty()) {
    std::vector<Game> nexts;
    std::vector<std::thread> th;
    int thread_num = std::thread::hardware_concurrency();
    if (thread_num > 1) {
      for (int i = 0; i < thread_num; ++i) {
        th.emplace_back(
            calc_next, i, thread_num, std::cref(beam), std::ref(nexts),
            std::ref(memo), std::ref(ans));
      }
      for (int i = 0; i < thread_num; ++i) th[i].join();
    } else {
      calc_next(0, 1, beam, nexts, memo, ans);
    }
    if (nexts.size() > MAX_BEAM) {
      std::nth_element(std::begin(nexts), std::begin(nexts) + MAX_BEAM, std::end(nexts));
      nexts.erase(std::begin(nexts) + MAX_BEAM, std::end(nexts));
    }
    std::swap(beam, nexts);
  }
  if (ans.empty()) {
    return std::vector<pos>();
  } else {
    std::vector<pos> res;
    int min_time = 1000000;
    for (const auto &vp : ans) {
      int score = simulate(st, vp).score;
      if (min_time > score) {
        res = vp;
        min_time = score;
      }
    }
    return res;
  }
}
int inf_dist(const Board &bd) {
  std::array<std::vector<pos>, 3> flowers_pos;
  for (int i = 0; i < 10; ++i)
    for (int j = 0; j < 14; ++j)
      if (char2clr(bd.tb[i][j]) >= 0)
        flowers_pos[char2clr(bd.tb[i][j])].emplace_back(j, i);
  int max_vdist = 0, max_hdist = 0;
  for (const Unit &u : bd.units)
    if (u.is_seed() && !u.seed.is_bloomed) {
      int min_vdist = 10000, min_hdist = 10000;
      for (pos p : flowers_pos[u.seed.color]) {
        min_vdist = std::min(min_vdist,
            std::max(0, std::max((u.y() - p.y - 1) * 35,
              (p.y - u.y() - 1) * 2)));
        min_hdist = std::min(min_hdist,
            std::max(0, (std::abs(u.x() - p.x) - 1) * 10));
      }
      for (const Unit &v : bd.units)
        if (v.is_seed() && v.seed.is_bloomed && v.seed.color == u.seed.color) {
          int vx, vy;
          std::tie(vx, vy) = bloom_pos(v.seed);
          min_vdist = std::min(min_vdist, std::max(0, (std::abs(u.y() - vy) - 1) * 2));
          min_hdist = std::min(min_hdist, std::max(0, (std::abs(u.x() - vx) - 1) * 10));
        }
      max_vdist = std::max(max_vdist, min_vdist);
      max_hdist = std::max(max_hdist, min_hdist);
    }
  return max_hdist + max_vdist;
}
std::vector<pos> astar_search(const State &st) {
  map_t memo;
  using T = std::tuple<int, Game>;
  std::priority_queue<T, std::vector<T>, std::greater<T>> que;
  que.emplace(inf_dist(st.bd), Game(st));
  memo[st.bd.units] = inf_dist(st.bd);
  int minimum_score = inf_dist(st.bd);
  while (!que.empty()) {
    int score;
    Game g;
    std::tie(score, g) = que.top();
    que.pop();
    if (minimum_score < score) {
      std::cerr << score << std::endl;
      minimum_score = score;
    }
    auto itr = memo.find(g.st.bd.units);
    if (score > std::get<1>(*itr)) continue;
    if (g.st.bd.is_goal()) return g.history;
    for (const auto &next : next_states(g.st)) {
      State nx;
      pos p;
      std::tie(nx, p) = next;
      int nx_score = inf_dist(nx.bd) + nx.score;
      auto itr = memo.find(nx.bd.units);
      if (itr == std::end(memo) || std::get<1>(*itr) > nx_score) {
        auto his = g.history;
        his.push_back(p);
        que.emplace(nx_score, Game(nx, his));
        memo[nx.bd.units] = nx_score;
      }
    }
  }
  return std::vector<pos>();
}
boost::optional<std::vector<pos>> random_walk_impl(
    const State &st, set_t &memo, std::mt19937 &mt, uint64_t &count) {
  if (st.bd.is_goal()) return std::vector<pos>();
  if (count % 10000 == 0) std::cerr << count << std::endl;
  ++count;
  memo.insert(st.bd.units);
  auto nexts = next_states(st);
  std::vector<std::tuple<State, pos>> coodinates;
  for (auto &next : nexts)
    if (memo.count(std::get<0>(next).bd.units) == 0)
      coodinates.push_back(next);
  if (coodinates.empty()) return boost::none;
  std::shuffle(std::begin(coodinates), std::end(coodinates), mt);
  for (auto next : coodinates) {
    if (auto opt_res = random_walk_impl(std::get<0>(next), memo, mt, count)) {
      opt_res->push_back(std::get<1>(next));
      return opt_res;
    }
  }
  return boost::none;
} 
std::vector<pos> random_walk(const State &st) {
  std::random_device rd;
  std::mt19937 mt(rd());
  set_t memo;
  uint64_t count = 0;
  if (auto opt_ans = random_walk_impl(st, memo, mt, count)) {
    return *opt_ans;
  } else return std::vector<pos>();
}
