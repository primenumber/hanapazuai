#include "board.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/optional.hpp>

std::pair<int, int> bloom_pos(const Seed &s) {
  int dd[] = {-1, 0, 1, 0, -1};
  return std::make_pair(s.x + dd[s.dir+1], s.y + dd[s.dir]);
}

int Seed::top() const {
  if (is_bloomed && dir == 0) return y-1;
  else return y;
}
int Seed::left() const {
  if (is_bloomed && dir == 3) return x-1;
  else return x;
}
int Seed::height() const {
  if (!is_bloomed) return 1;
  if (dir % 2 == 0) return 2; // ^ or v
  return 1;
}
int Seed::width() const {
  if (!is_bloomed) return 1;
  if (dir % 2 == 1) return 2; // < or >
  return 1;
}
int Unit::height() const {
  if (is_seed()) return seed.height();
  else return block.h;
}
int Unit::width() const {
  if (is_seed()) return seed.width();
  else return block.w;
}
int Unit::x() const {
  if (is_seed()) return seed.left();
  else return block.x;
}
int Unit::y() const {
  if (is_seed()) return seed.top();
  else return block.y;
}
void Unit::move_up() {
  if (is_seed()) --seed.y;
  else --block.y;
}
void Unit::move_right() {
  if (is_seed()) ++seed.x;
  else ++block.x;
}
void Unit::move_down() {
  if (is_seed()) ++seed.y;
  else ++block.y;
}
void Unit::move_left() {
  if (is_seed()) --seed.x;
  else --block.x;
}

bool operator==(const Seed &lhs, const Seed &rhs) {
  return memcmp(&lhs, &rhs, sizeof(Seed)) == 0;
}
bool operator==(const Block &lhs, const Block &rhs) {
  return memcmp(&lhs, &rhs, sizeof(Block)) == 0;
}
bool operator==(const Unit &lhs, const Unit &rhs) {
  if (lhs.type != rhs.type) return false;
  if (lhs.is_seed()) return lhs.seed == rhs.seed;
  return lhs.block == rhs.block;
}
bool operator<(const Unit &lhs, const Unit &rhs) {
  int lb = lhs.y() + lhs.height() - 1;
  int rb = rhs.y() + rhs.height() - 1;
  if (lb == rb) return lhs.x() < rhs.x();
  else return lb > rb;
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
boost::optional<Unit> get_unit(const Board &bd, int x, int y) {
  for (const Unit &u : bd.units) {
    if (is_hit(u, x, y)) return u;
  }
  return boost::none;
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
std::pair<Unit, Unit> swap_unit(const Unit &lunit, const Unit &runit) {
  Unit newl = runit, newr = lunit;
  newl.move_left();
  newr.move_right();
  return std::make_pair(newl, newr);
}
int drop(Board &bd) {
  std::sort(std::begin(bd.units), std::end(bd.units));
  int max_drop_h = 0;
  for (int i = 0; i < (int)bd.units.size(); ++i) {
    Unit nx_unit = bd.units[i];
    for (int j = 0;; ++j) {
      nx_unit.move_down();
      bool ok = true;
      if (is_conflict(bd.tb, nx_unit)) ok = false;
      else {
        for (int k = 0; k < i; ++k) {
          if (is_conflict(bd.units[k], nx_unit)) {
            ok = false;
            break;
          }
        }
      }
      if (!ok) {
        max_drop_h = std::max(max_drop_h, j);
        nx_unit.move_up();
        bd.units[i] = nx_unit;
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
      if (auto opt_unit = get_unit(bd, ts, u.x()+i)) {
        if (!can_push(bd, *opt_unit, dir)) return false;
      }
    }
    return true;
   }
   case 1: case 3:{
    int rs = u.x() + (dir == 1 ? u.width() : -1);
    for (int i = 0; i < u.height(); ++i) {
      if (bd.tb[u.y()+i][rs] != '.') return false;
      if (auto opt_unit = get_unit(bd, u.y()+i, rs)) {
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
      if (auto opt_unit = get_unit(bd, ts, u.x()+i)) {
        push(bd, *opt_unit, dir);
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
      if (auto opt_unit = get_unit(bd, u.y()+i, rs)) {
        push(bd, *opt_unit, dir);
      }
    }
    if (move_unit) {
      if (dir == 1) u.move_right();
      else u.move_left();
    }
    return;
   }
  }
}
bool is_ne_f(const b_ary &b, const Seed s) {
  int dd[] = {-1, 0, 1, 0, -1};
  for (int i = 0; i < 4; ++i) {
    int nx = s.x + dd[i+1];
    int ny = s.y + dd[i];
    if (char2clr(b[ny][nx]) == s.color) return true;
  }
  return false;
}

bool try_bloom(Board &bd, Unit &u) {
  if (!u.is_seed()) return false;
  Seed &s = u.seed;
  if (s.is_bloomed) return false;
  if (!is_ne_f(bd.tb, s)) return false;
  if (can_push(bd, u, s.dir)) {
    push(bd, u, s.dir, false);
    s.is_bloomed = true;
    return true;
  }
  if (can_push(bd, u, (u.seed.dir+2)%4)) {
    push(bd, u, (u.seed.dir+2)%4, false);
    u.seed.is_bloomed = true;
    return true;
  } else return false;
}
int bloom(Board &bd) {
  std::sort(std::begin(bd.units), std::end(bd.units));
  int num = 0;
  for (Unit &u : bd.units) {
    bool is_bloom = try_bloom(bd, u);
    if (is_bloom) ++num;
  }
  return num;
}
std::tuple<Board, int> fix(Board &&bd) {
  int elapsed = 10;
  while(true) {
    int max_drop_h = drop(bd);
    elapsed += max_drop_h * 2;
    int bloom_num = bloom(bd);
    elapsed += bloom_num * 35;
    if (max_drop_h == 0 && bloom_num == 0) break;
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

int char2dir(char c) {
  switch(c) {
    case 'U': return 0;
    case 'R': return 1;
    case 'D': return 2;
    case 'L': return 3;
    default: return -1;
  }
}
int char2clr(char c) {
  switch(c) {
    case 'R': return 0;
    case 'Y': return 1;
    case 'B': return 2;
    default: return -1;
  }
}
Seed seed_from_file(std::stringstream &ss) {
  std::string color, dir;
  int x,y;
  ss >> color >> dir >> y >> x;
  return Seed(char2clr(color[0]), char2dir(dir[0]), x, y, false);
}
Block block_from_file(std::stringstream &ss) {
  int x,y,w,h;
  ss >> y >> x >> h >> w;
  return Block(x, y, w, h);
}
std::vector<Unit> units_from_file(std::ifstream &ifs) {
  using std::string;
  std::vector<Unit> units;
  string line;
  while(std::getline(ifs, line)) {
    std::stringstream ss;
    ss << line;
    string type;
    ss >> type;
    if (type == "S") {
      units.emplace_back(seed_from_file(ss));
    } else {
      units.emplace_back(block_from_file(ss));
    }
  }
  return units;
}
b_ary table_from_file(std::ifstream &ifs) {
  b_ary table;
  int h,w;
  ifs>>h>>w;
  std::string line;
  std::getline(ifs, line);
  for (int i = 0; i < h; ++i) {
    std::getline(ifs, line);
    for (int j = 0; j < w; ++j) {
      table[i][j] = line[j];
    }
  }
  return table;
}

Board board_from_file(const std::string &filename) {
  std::ifstream ifs(filename);
  Board bd;
  bd.tb = table_from_file(ifs);
  std::string line;
  std::getline(ifs, line);
  bd.units = units_from_file(ifs);
  return bd;
}
