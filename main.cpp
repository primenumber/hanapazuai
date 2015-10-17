#include <ctime>
#include <iostream>
#include <sstream>
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
  std::cerr << state.score << std::endl;
}

void solve(State state) {
  auto res = find_answer(state);
  for (pos &p : res) {
    std::cout << p.y << ' ' << p.x << std::endl;
  }
}

void simulate(State state) {
  init();
  std::string line;
  timespec ts;
  ts.tv_sec = 1;
  ts.tv_nsec = 0;
  while(std::getline(std::cin, line)) {
    visualize(state.bd);
    std::stringstream ss;
    ss << line;
    int x, y;
    ss >> y >> x;
    state = *try_move(state, x, y);
    nanosleep(&ts, nullptr);
  }
  visualize(state.bd);
  nanosleep(&ts, nullptr);
  finalize();
  std::cerr << "score: " << state.score << std::endl;
}

void calc(State state) {
  std::string line;
  while(std::getline(std::cin, line)) {
    std::stringstream ss;
    ss << line;
    int x, y;
    ss >> y >> x;
    state = *try_move(state, x, y);
  }
  if (state.bd.is_goal())
    std::cout << "score: " << state.score << std::endl;
  else
    std::cout << "error" << std::endl;
}

void random_walk_search(State state) {
  auto res = random_walk(state);
  for (pos &p : res) {
    std::cout << p.y << ' ' << p.x << std::endl;
  }
}

void beam(State state) {
  auto res = beam_search(state);
  for (pos &p : res) {
    std::cout << p.y << ' ' << p.x << std::endl;
  }
}

int main(int argc, char **argv) {
  if (argc < 3) {
    std::cerr << "usage: " << argv[0] << " COMMAND INPUT" << std::endl;
    std::cerr << "COMMAND:" << std::endl;
    std::cerr << "	play play game" << std::endl;
    std::cerr << "	solve run solver" << std::endl;
    std::cerr << "	simulate simulation" << std::endl;
    std::cerr << "	calc calculate score" << std::endl;
    std::cerr << "	rand random walk search" << std::endl;
    std::cerr << "	beam beam search" << std::endl;
    return 1;
  }
  State state(board_from_file(argv[2]));
  std::string command = argv[1];
  if (command == "play") {
    play(state);
  } else if (command == "solve") {
    solve(state);
  } else if (command == "simulate") {
    simulate(state);
  } else if (command == "calc") {
    calc(state);
  } else if (command == "random") {
    random_walk_search(state);
  } else if (command == "beam") {
    beam(state);
  } else {
    std::cerr << "unknown command: " << command << std::endl;
  }
  return 0;
}
