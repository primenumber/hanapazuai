#pragma once
#include <vector>
#include <utility>
#include "board.hpp"
#include "play.hpp"

std::vector<pos> find_answer(const State &);
std::vector<pos> random_walk(const State &);
std::vector<pos> beam_search(const State &);
std::vector<pos> astar_search(const State &);
