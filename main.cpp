#include <ncurses.h>
#include "board.hpp"
#include "visualize.hpp"
#include "play.hpp"

int main(int argc, char **argv) {
  init();
  Board b = board_from_file(argv[1]);
  int score = 0;
  while(true) {
    visualize(b);
    move(12, 0);
    addstr("command> ");
    char cmd[1025];
    getstr(cmd);
    int x = cmd[0] - 'A';
    int y = cmd[1] - '0';
    if (auto opt_res = try_move(b, x, y)) {
      b = std::get<0>(*opt_res);
      score += std::get<1>(*opt_res);
      mvprintw(13, 0, "score: %d", score);
    } else {
      move(13, 0);
      addstr("cannot move");
    }
  }
  finalize();
  return 0;
}
