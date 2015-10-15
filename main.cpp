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

void solve(State state) {
  auto res = find_answer(state);
  for (pos &p : res) {
    std::cout << p.x << ' ' << p.y << std::endl;
  }
}

int main(int argc, char **argv) {
  if (argc < 3) {
    std::cerr << "usage: " << argv[0] << " COMMAND INPUT" << std::endl;
    std::cerr << "COMMAND:" << std::endl;
    std::cerr << "	play play game" << std::endl;
    std::cerr << "	solve run solver" << std::endl;
    return 1;
  }
  State state(board_from_file(argv[2]));
  std::string command = argv[1];
  if (command == "play") {
    play(state);
  } else if (command == "solve") {
    solve(state);
  }
  return 0;
}
