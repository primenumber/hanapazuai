#pragma once
#include <cstring>
#include <algorithm>
#include <array>
#include <vector>
#include <string>
#include <tuple>
#include <boost/operators.hpp>

template <typename T>
using line = std::array<T, 14>;
template <typename T>
using table_t = std::array<line<T>, 10>;
using b_ary = table_t<char>;

// URDL -> 0123
int char2dir(char);
int char2clr(char);

struct Seed : public boost::operators<Seed> {
  uint16_t color : 2;
  uint16_t dir : 2;
  uint16_t x : 4;
  uint16_t y : 4;
  uint16_t is_bloomed : 1;
  Seed() = default;
  Seed(int color, int dir, int x, int y, bool is_bloomed)
    : color(color), dir(dir), x(x), y(y), is_bloomed(is_bloomed) {}
  int top() const;
  int left() const;
  int height() const;
  int width() const;
};
std::pair<int, int> bloom_pos(const Seed &);
bool operator==(const Seed &, const Seed &);

struct Block : public boost::operators<Block> {
  uint16_t x : 4;
  uint16_t y : 4;
  uint16_t w : 4;
  uint16_t h : 4;
  Block() = default;
  Block(int x, int y, int w, int h)
    : x(x), y(y), w(w), h(h) {}
};
bool operator==(const Block &, const Block &);

struct Unit : public boost::operators<Unit> {
  int type;
  union {
    Seed seed;
    Block block;
  };
  Unit() = default;
  Unit(const Seed &s) : type(0), seed(s) {}
  Unit(const Block &b) : type(1), block(b) {}
  Unit(const Unit &) = default;
  Unit &operator=(const Seed &s) { type = 0; seed = s; return *this; }
  Unit &operator=(const Block &b) { type = 1; block = b; return *this; }
  bool is_seed() const { return type == 0; }
  int x() const;
  int y() const;
  int height() const;
  int width() const;
  void move_up();
  void move_right();
  void move_down();
  void move_left();
};
bool operator==(const Unit &, const Unit &);
bool operator<(const Unit &, const Unit &);
bool bin_less(const Unit &, const Unit &);

struct Board {
  b_ary tb;
  std::vector<Unit> units;
  bool is_goal() const;
};
bool operator==(const Board &, const Board &);

Board board_from_file(const std::string & filename);

namespace std {

template <>
class hash<Block> {
 public:
  size_t operator()(const Block &b) const {
    return b.x+b.y*14+b.w*14*10+b.h*14*10*14;
  }
};
template <>
class hash<Seed> {
 public:
  size_t operator()(const Seed &s) const {
    return s.x+s.y*14+s.color*14*10+s.dir*14*10*3+s.is_bloomed*14*10*3*4;
  }
};
template <>
class hash<Unit> {
 public:
  size_t operator()(const Unit &u) const {
    if (u.is_seed()) return hash<Seed>()(u.seed);
    else return hash<Block>()(u.block);
  }
};
template <>
class hash<vector<Unit>> {
 public:
  size_t operator()(const vector<Unit> &units) const {
    size_t res = 0;
    vector<Unit> cp = units;
    sort(begin(cp), end(cp), bin_less);
    for (const Unit &u : cp) {
      res += hash<Unit>()(u);
      res *= 17;
    }
    return res;
  }
};
template <>
class hash<Board> {
 public:
  size_t operator()(const Board &b) const {
    return hash<vector<Unit>>()(b.units);
  }
};

} // namespace std
  
