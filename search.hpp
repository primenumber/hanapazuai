#pragma once
#include <vector>
#include <utility>
#include "board.hpp"
#include "play.hpp"

std::vector<pos> find_answer(const State &);
std::vector<pos> random_walk(const State &);
