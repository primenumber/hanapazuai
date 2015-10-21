#include "search.hpp"
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <queue>
#include <unordered_map>
#include <boost/optional.hpp>

using units_t = std::vector<Unit>;
using map_t = std::unordered_map<units_t, int>;

int inf_dist(const Board &bd) {
  std::array<std::vector<pos>, 3> flowers_pos;
  for (int i = 0; i < 10; ++i)
    for (int j = 0; j < 14; ++j)
      if (char2clr(bd.tb[i][j]) >= 0)
        flowers_pos[char2clr(bd.tb[i][j])].emplace_back(j, i);
  int max_vdist = 0, max_hdist = 0;
  int sum_hdist = 0;
  int max_v_lift = 0;
  bool v_flower = false, h_flower = false;
  for (const Unit &u : bd.units)
    if (u.is_seed() && !u.seed.is_bloomed) {
      int min_vdist = 10000, min_hdist = 10000;
      int min_lift = 1000;
      if (u.seed.dir % 2 == 1)
        h_flower = true;
      else
        v_flower = true;
      for (pos p : flowers_pos[u.seed.color]) {
        if (u.y() > p.y) min_lift = std::min(min_lift, (u.y() - p.y - 1) * 35);
        else if (p.y > u.y()) min_vdist = std::min(min_vdist, (p.y - u.y() - 1) * 2);
        else min_vdist = 0;
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
      max_v_lift = std::max(max_v_lift, std::min(min_vdist, min_lift));
      max_vdist = std::max(max_vdist, min_vdist);
      max_hdist = std::max(max_hdist, min_hdist);
      sum_hdist += min_hdist;
    }
  int hdist = h_flower ? (max_hdist + 35) : sum_hdist;
  int vdist = v_flower ? max_v_lift : max_vdist;
  return hdist + vdist;
}
using T = std::tuple<int, Game>;
using queue_t = std::priority_queue<T, std::vector<T>, std::greater<T>>;
std::mutex mtx, mtx_memo;
bool check_memo(map_t &memo, const units_t &u, int score) {
  std::unique_lock<std::mutex> lck(mtx_memo);
  auto itr = memo.find(u);
  if (itr == std::end(memo)) return false;
  return std::get<1>(*itr) < score;
}
bool check_and_set_memo(map_t &memo, const units_t &u, int score) {
  std::unique_lock<std::mutex> lck(mtx_memo);
  auto itr = memo.find(u);
  if (itr == std::end(memo)) {
    memo[u] = score;
    return true;
  } else if (std::get<1>(*itr) > score) {
    memo[u] = score;
    return true;
  } else {
    return false;
  }
}
void search_impl(map_t &memo, queue_t &que,
    std::vector<std::vector<pos>> &ans, int &minimum_score, int i) {
  while (ans.empty()) {
    int score;
    Game g;
    std::unique_lock<std::mutex> lock(mtx);
    if (que.empty()) break;
    std::tie(score, g) = que.top();
    que.pop();
    lock.unlock();
    if (minimum_score < score) {
      std::cerr << score << ' ' << i << std::endl;
      minimum_score = score;
    }
    if (check_memo(memo, g.st.bd.units, score)) continue;
    if (g.st.bd.is_goal()) {
      mtx.lock();
      ans.push_back(g.history);
      mtx.unlock();
      return;
    }
    for (const auto &next : next_states(g.st)) {
      State nx;
      pos p;
      std::tie(nx, p) = next;
      int nx_score = inf_dist(nx.bd) + nx.score;
      if (check_and_set_memo(memo, nx.bd.units, nx_score)) {
        auto his = g.history;
        his.push_back(p);
        mtx.lock();
        que.emplace(nx_score, Game(nx, his));
        mtx.unlock();
      }
    }
  }
}
std::vector<pos> astar_search(const State &st) {
  map_t memo;
  std::priority_queue<T, std::vector<T>, std::greater<T>> que;
  que.emplace(inf_dist(st.bd), Game(st));
  memo[st.bd.units] = inf_dist(st.bd);
  int minimum_score = inf_dist(st.bd);
  std::vector<std::vector<pos>> ans;
  std::vector<std::thread> th;
  for (int i = 0; i < std::thread::hardware_concurrency(); ++i) {
    th.emplace_back(search_impl, std::ref(memo), std::ref(que), std::ref(ans), std::ref(minimum_score), i);
    timespec ts;
    ts.tv_sec = 1;
    ts.tv_nsec = 0;
    nanosleep(&ts, nullptr);
  }
  for (auto &t : th) {
    t.join();
  }
  return ans.front();
}
