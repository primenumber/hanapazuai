#pragma once
#include <cstring>
#include <array>
#include <vector>
#include <string>
#include <tuple>
#include <boost/optional.hpp>
#include <boost/operators.hpp>

using line = std::array<char, 14>;
using b_ary = std::array<line, 10>;

// URDL -> 0123
int char2dir(char);
int char2clr(char);

struct Seed : public boost::operators<Seed> {
  int color;
  int dir;
  int x;
  int y;
  bool is_bloomed;
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
  int x;
  int y;
  int w;
  int h;
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

struct Board {
  b_ary tb;
  std::vector<Unit> units;
};

bool is_movable(const Board &, int x, int y);
// (next_board, elapsed_time)
boost::optional<std::tuple<Board, int>> try_move(const Board &, int x, int y);
//std::tuple<Board, int> move(const Board &, int x, int y);

Board board_from_file(const std::string & filename);
