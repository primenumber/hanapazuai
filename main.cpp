#include <ncurses.h>
#include "board.hpp"
#include "visualize.hpp"
#include "play.hpp"

int main(int argc, char **argv) {
  init();
  State state(board_from_file(argv[1]));
  while(true) {
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
  return 0;
}
