#include "play.hpp"
#include <cassert>

b_ary to_b_ary(const Board &bd) {
  b_ary table = bd.tb;
  for (int i = 0; i < (int)bd.units.size(); ++i) {
    const Unit &u = bd.units[i];
    for (int j = 0; j < u.height(); ++j) {
      for (int k = 0; k < u.width(); ++k) {
        table[u.y()+j][u.x()+k] = i;
      }
    }
  }
  return table;
}
bool is_hit(const Seed &s, int x, int y) {
  if (s.x == x && s.y == y) return true;
  int bx, by;
  std::tie(bx, by) = bloom_pos(s);
  return s.is_bloomed && bx == x && by == y;
}
bool is_hit(const Block &b, int x, int y) {
  return b.x <= x && x < b.x + b.w && b.y <= y && y < b.y + b.h;
}
bool is_hit(const Unit &u, int x, int y) {
  if (u.is_seed()) {
    return is_hit(u.seed, x, y);
  } else {
    return is_hit(u.block, x, y);
  }
}
boost::optional<const Unit &> get_unit(const Board &bd, int x, int y) {
  for (const Unit &u : bd.units) {
    if (is_hit(u, x, y)) return u;
  }
  return boost::none;
}
boost::optional<Unit &> get_unit(Board &bd, int x, int y) {
  for (Unit &u : bd.units) {
    if (is_hit(u, x, y)) return u;
  }
  return boost::none;
}
boost::optional<const Unit &> get_unit(
    const Board &bd, const b_ary &table, int x, int y) {
  if (table[y][x] < (char)bd.units.size())
    return bd.units[table[y][x]];
  else
    return boost::none;
}
boost::optional<Unit &> get_unit(
    Board &bd, const b_ary &table, int x, int y) {
  if (table[y][x] < (char)bd.units.size())
    return bd.units[table[y][x]];
  else
    return boost::none;
}
int flower_color(const Board &bd, int x, int y) {
  if (char2clr(bd.tb[y][x]) >= 0) return char2clr(bd.tb[y][x]);
  if (auto opt_unit = get_unit(bd, x, y)) {
    if (opt_unit->is_seed() && std::make_pair(x, y) == bloom_pos(opt_unit->seed)) {
      return opt_unit->seed.color;
    }
  }
  return -1;
}

bool is_conflict(const Unit &lunit, const Unit &runit) {
  for (int i = 0; i < lunit.height(); ++i)
    for (int j = 0; j < lunit.width(); ++j)
      if (is_hit(runit, lunit.x()+j, lunit.y()+i)) return true;
  return false;
}
bool is_conflict(const b_ary &table, const Unit &unit) {
  for (int i = 0; i < unit.height(); ++i)
    for (int j = 0; j < unit.width(); ++j)
      if (table.at(unit.y()+i).at(unit.x()+j) != '.') return true;
  return false;
}
bool is_conflict_table(const b_ary &table, const Unit &unit, const int id) {
  for (int i = 0; i < unit.height(); ++i)
    for (int j = 0; j < unit.width(); ++j) {
      char data = table.at(unit.y()+i).at(unit.x()+j);
      if (data != '.' && data != id) return true;
    }
  return false;
}
std::pair<Unit, Unit> swap_unit(const Unit &lunit, const Unit &runit) {
  Unit newl = runit, newr = lunit;
  newl.move_left();
  newr.move_right();
  return std::make_pair(newl, newr);
}
int drop(Board &bd) {
  std::sort(std::begin(bd.units), std::end(bd.units));
  int max_drop_h = 0;
  b_ary tb = to_b_ary(bd);
  for (int i = 0; i < (int)bd.units.size(); ++i) {
    Unit nx_unit = bd.units[i];
    for (int j = 0;; ++j) {
      nx_unit.move_down();
      if (is_conflict_table(tb, nx_unit, i)) {
        max_drop_h = std::max(max_drop_h, j);
        nx_unit.move_up();
        bd.units[i] = nx_unit;
        tb = to_b_ary(bd);
        break;
      }
    }
  }
  return max_drop_h;
}
bool can_push(Board &bd, const Unit &u, int dir) {
  switch (dir) {
   case 0: case 2: {
    int ts = u.y() + (dir == 0 ? -1 : u.height());
    for (int i = 0; i < u.width(); ++i) {
      if (bd.tb[ts][u.x()+i] != '.') return false;
      if (auto opt_unit = get_unit(bd, u.x()+i, ts)) {
        if (!can_push(bd, *opt_unit, dir)) return false;
      }
    }
    return true;
   }
   case 1: case 3:{
    int rs = u.x() + (dir == 1 ? u.width() : -1);
    for (int i = 0; i < u.height(); ++i) {
      if (bd.tb[u.y()+i][rs] != '.') return false;
      if (auto opt_unit = get_unit(bd, rs, u.y()+i)) {
        if (!can_push(bd, *opt_unit, dir)) return false;
      }
    }
    return true;
   }
   default: return false;
  }
}
void push(Board &bd, Unit &u, int dir, bool move_unit = true) {
  switch (dir) {
   case 0: case 2: {
    int ts = u.y() + (dir == 0 ? -1 : u.height());
    for (int i = 0; i < u.width(); ++i) {
      if (auto opt_unit = get_unit(bd, u.x()+i, ts)) {
        push(bd, std::ref(*opt_unit), dir);
      }
    }
    if (move_unit) {
      if (dir == 0) u.move_up();
      else u.move_down();
    }
    return;
   }
   case 1: case 3:{
    int rs = u.x() + (dir == 1 ? u.width() : -1);
    for (int i = 0; i < u.height(); ++i) {
      if (auto opt_unit = get_unit(bd, rs, u.y()+i)) {
        push(bd, std::ref(*opt_unit), dir);
      }
    }
    if (move_unit) {
      if (dir == 1) u.move_right();
      else u.move_left();
    }
    return;
   }
   default: assert(false);
  }
}
bool is_ne_f(const Board &bd, const Seed s) {
  int dd[] = {-1, 0, 1, 0, -1};
  for (int i = 0; i < 4; ++i) {
    int nx = s.x + dd[i+1];
    int ny = s.y + dd[i];
    if (flower_color(bd, nx, ny) == s.color) return true;
  }
  return false;
}

bool try_bloom(Board &bd, Unit &u) {
  if (!u.is_seed()) return false;
  Seed &s = u.seed;
  if (s.is_bloomed) return false;
  if (!is_ne_f(bd, s)) return false;
  if (can_push(bd, u, s.dir)) {
    push(bd, u, s.dir, false);
    s.is_bloomed = true;
    return true;
  }
  if (can_push(bd, u, (s.dir+2)%4)) {
    push(bd, u, (s.dir+2)%4);
    s.is_bloomed = true;
    return true;
  } else return false;
}
bool bloom(Board &bd) {
  std::sort(std::begin(bd.units), std::end(bd.units));
  bool bloomed = false;
  for (Unit &u : bd.units) {
    bool is_bloom = try_bloom(bd, u);
    if (is_bloom) bloomed = true;
  }
  return bloomed;
}
std::tuple<Board, int> fix(Board &&bd) {
  int elapsed = 10;
  while(true) {
    int max_drop_h = drop(bd);
    elapsed += max_drop_h * 2;
    bool bloomed = bloom(bd);
    elapsed += bloomed ? 35 : 0;
    if (max_drop_h == 0 && !bloomed) break;
  }
  return std::make_tuple(bd, elapsed);
}
boost::optional<std::tuple<Board, int>> try_swap(
    const Board &bd, const Unit &lunit, const Unit &runit) {
  if (lunit == runit) {
    return boost::none;
  }
  if (lunit.width() > 1 || runit.width() > 1) {
    return boost::none;
  }
  auto swapped = swap_unit(lunit, runit);
  if (is_conflict(bd.tb, swapped.first) || is_conflict(bd.tb, swapped.second)) {
    return boost::none;
  }
  Board res = bd;
  for (Unit &u : res.units) {
    if (u == lunit) {
      u = swapped.second;
      continue;
    } else if (u == runit) {
      u = swapped.first;
      continue;
    }
    if (is_conflict(swapped.first, u) || is_conflict(swapped.second, u)) {
      return boost::none;
    }
  }
  return fix(std::move(res));
}
boost::optional<std::tuple<Board, int>> try_move(
    const Board &bd, const Unit &unit, int dir) {
  Unit nx_unit = unit;
  if (dir == 1) nx_unit.move_right();
  else nx_unit.move_left();
  if (is_conflict(bd.tb, nx_unit)) {
    return boost::none;
  }
  Board res = bd;
  for (Unit &u : res.units) {
    if (u == unit) {
      u = nx_unit;
      continue;
    }
    if (is_conflict(nx_unit, u)) {
      return boost::none;
    }
  }
  return fix(std::move(res));
}
boost::optional<std::tuple<Board, int>> try_move(const Board &bd, int x, int y) {
  if (auto opt_unit = get_unit(bd, x, y)) {
    Unit lunit = *opt_unit;
    if (auto opt_runit = get_unit(bd, x+1, y)) {
      Unit runit = *opt_runit;
      return try_swap(bd, lunit, runit);
    } else {
      return try_move(bd, lunit, 1); // move right
    }
  } else if (auto opt_unit = get_unit(bd, x+1, y)) {
    return try_move(bd, *opt_unit, 3); // move left
  } else {
    return boost::none;
  }
}
boost::optional<State> try_move(const State &st, int x, int y) {
  if (auto res = try_move(st.bd, x, y)) {
    return State(std::get<0>(*res), std::get<1>(*res) + st.score);
  } else {
    return boost::none;
  }
}
std::vector<std::tuple<State, pos>> next_states(const State &st) {
  std::vector<std::tuple<State, pos>> res;
  for (int i = 0; i < 10; ++i) {
    for (int j = -1; j < 13; ++j) {
      if (auto opt_state = try_move(st, j, i)) {
        res.emplace_back(*std::move(opt_state), pos(j, i));
      }
    }
  }
  return res;
}
