#include "visualize.hpp"
#include <ncurses.h>

void init() {
  initscr();
  start_color();
  init_pair(1, COLOR_BLACK, COLOR_WHITE);
  init_pair(2, COLOR_WHITE, COLOR_RED);
  init_pair(3, COLOR_GREEN, COLOR_RED);
  init_pair(4, COLOR_WHITE, COLOR_YELLOW);
  init_pair(5, COLOR_GREEN, COLOR_YELLOW);
  init_pair(6, COLOR_WHITE, COLOR_BLUE);
  init_pair(7, COLOR_GREEN, COLOR_BLUE);
}

void finalize() {
  endwin();
}

void output_header() {
  move(0, 1);
  for (int i = 0; i < 14; ++i)
    addch('A'+i);
  for (int i = 0; i < 10; ++i) {
    move(i+1, 0);
    addch('0'+i);
  }
}

void output_table(const b_ary &table) {
  for (int i = 0; i < 10; ++i) {
    for (int j = 0; j < 14; ++j) {
      move(i+1, j+1);
      addch(table[i][j]);
    }
  }
}

void output_seed(const Seed &s) {
  char dirstr[] = {'^', '>', 'v', '<'};
  int colorcode = s.color * 2 + 2 + s.is_bloomed;
  attron(COLOR_PAIR(colorcode));
  move(s.y+1, s.x+1);
  addch(dirstr[s.dir]);
  if (s.is_bloomed) {
    int x, y;
    std::tie(x, y) = bloom_pos(s);
    move(y+1, x+1);
    addch('*');
  }
  attroff(COLOR_PAIR(colorcode));
}

void output_block(const Block &b, int num) {
  attron(COLOR_PAIR(1));
  for (int i = 0; i < b.h; ++i) {
    move(b.y+i+1, b.x+1);
    for (int j = 0; j < b.w; ++j) {
      addch('0'+num);
    }
  }
  attroff(COLOR_PAIR(1));
}

void output_units(const std::vector<Unit> &units) {
  int count = 0;
  for (const Unit &u : units) {
    if (u.is_seed()) {
      output_seed(u.seed);
    } else {
      output_block(u.block, count++);
    }
  }
}

void visualize(const Board &bd) {
  output_header();
  output_table(bd.tb);
  output_units(bd.units);
  refresh();
}
