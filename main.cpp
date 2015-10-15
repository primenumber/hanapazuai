#include <iostream>
#include <ncurses.h>
#include "board.hpp"
#include "visualize.hpp"
#include "play.hpp"
#include "search.hpp"

void play(State state) {
  init();
  while(!state.bd.is_goal()) {
    visualize(state.bd);
    move(12, 0);
    addstr("command> ");
    char cmd[1025];
    getstr(cmd);
    int x = cmd[0] - 'A';
    int y = cmd[1] - '0';
    if (auto opt_res = try_move(state, x, y)) {
      state = *opt_res;
      mvprintw(13, 0, "score: %d", state.score);
    } else {
      move(13, 0);
      addstr("cannot move");
    }
  }
  finalize();
}

void search(State state) {
  auto res = find_answer(state);
  for (pos &p : res) {
    std::cout << p.x << ' ' << p.y << std::endl;
  }
}

int main(int argc, char **argv) {
  State state(board_from_file(argv[1]));
  play(state);
  return 0;
}
