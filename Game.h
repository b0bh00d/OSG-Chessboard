#pragma once

//------------------------------------------------------------------------------
// MIT License
//
// Copyright (c) 2020 Bob Hood
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//------------------------------------------------------------------------------

#include "OSG.h"
#include "Chessboard.h"

class Game : public osg::Referenced
{
    ChessboardPtr chessboard;
    NodePtr sg_root;

    GroupPtr move_squares;
    GroupPtr attack_squares;

    void construct_move_squares(ChessboardPtr chessboard, GroupPtr &squares);
    void construct_capture_squares(ChessboardPtr chessboard, GroupPtr &squares);
    void construct_attack_markers(ChessboardPtr chessboard, GroupPtr &attack_group);
    NodePtr createScene();

public:
    static const std::vector<int> one_rank;     // indices for the columns in a single rank (0-7); used for looping
    static const std::vector<int> one_side;     // indices for the columns on a side (0-15); used for looping
    static const std::vector<int> white_ranks;  // indices for the ranks on the white side (0-1); used for looping
    static const std::vector<int> center_ranks; // indices for board center (2-6); used for looping
    static const std::vector<int> black_ranks;  // indices for the ranks on the black side (7-8); used for looping

    osg::Vec3 centerScope{0.0f, 0.0f, 0.0f};

public:
    Game();

    NodePtr get_root_node() const
    {
        return sg_root;
    }
    ChessboardPtr get_board() const
    {
        return chessboard;
    }
};

using GamePtr = osg::ref_ptr<Game>;
