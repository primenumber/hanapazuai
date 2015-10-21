#include "search.hpp"
#include <iostream>
#include <vector>
#include <unordered_set>
#include <boost/optional.hpp>

using units_t = std::vector<Unit>;
using set_t = std::unordered_set<units_t>;

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
