#include "search.hpp"
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <queue>
#include <unordered_set>
#include <boost/optional.hpp>

using units_t = std::vector<Unit>;
using set_t = std::unordered_set<units_t>;

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
  constexpr int MAX_BEAM = 300000;
  std::vector<std::vector<pos>> ans;
  int count = 0;
  while (!beam.empty()) {
    std::cerr << count << ' ' << beam.size() << std::endl;
    ++count;
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
    std::cerr << "beam: " << nexts.size() << std::endl;
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
