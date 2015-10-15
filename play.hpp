#pragma once
#include <boost/optional.hpp>
#include "board.hpp"

bool is_movable(const Board &, int x, int y);
// (next_board, elapsed_time)
boost::optional<std::tuple<Board, int>> try_move(const Board &, int x, int y);
//std::tuple<Board, int> move(const Board &, int x, int y);
