#include "board.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

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
