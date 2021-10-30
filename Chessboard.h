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

#include <memory>

#include "OSG.h"

using Position = std::tuple<int, int>;
using Bounds = std::tuple<float, float, float, float>;

struct Point
{
    double x, y;
    Point() : x(0.), y(0.) {}
    Point(double x_, double y_) : x(x_), y(y_) {}
    Point(const Point &source)
    {
        x = source.x;
        y = source.y;
    }
};

class Chessboard : public osg::Referenced
{
public:
    typedef enum
    {
        Black = 1,
        White
    } Side;

public:
    class Piece
    {
    public:
        enum class Rank : std::uint32_t
        {
            Empty,
            Rook,
            Knight,
            Bishop,
            King,
            Queen,
            Pawn
        };

        Piece();
        Piece(Rank rank_, Side side_, float facing_ = 0.f);
        Piece(const Piece &source);
        Piece &operator=(const Piece &source);

        Rank get_rank() const
        {
            return rank;
        }
        void set_rank(Rank rank_)
        {
            rank = rank_;
        }

        Side get_side() const
        {
            return side;
        }
        void set_side(Side side_)
        {
            side = side_;
        }

        void clear()
        {
            rank = Rank::Empty;
            side = White;
        }

        NodePtr get_mesh();
        void load_mesh(const std::string &id);

        bool is_empty() const
        {
            return rank == Rank::Empty;
        }

        void set_facing(float facing_)
        {
            facing = facing_;
        }
        float get_facing() const
        {
            return facing;
        }

        Position get_position() const
        {
            return Position(row, col);
        }

        bool is_captured() const
        {
            return captured;
        }
        void capture();

        void set_name(const std::string &name_)
        {
            name = name_;
        }
        std::string get_name() const
        {
            return name;
        }

        bool has_moved() const
        {
            return !first_move;
        }
        void move_to(int row_, int col_);

        void checked(bool checked_ = true)
        {
            in_check = checked_;
        }
        bool is_checked() const
        {
            return in_check;
        }

    private:
        Rank rank{Rank::Empty};
        Side side{White};

        float facing{0.f};

        int row{0};
        int col{0};

        bool captured{false};
        bool first_move{true};
        bool in_check{false};

        std::string name;
    };

    class Cell
    {
    public:
        enum class Type
        {
            Invalid,
            Board,      // a cell on the chessboard itself
            Holding     // a "holding" cell for captured pieces
        };

    public:
        Type type;
        Point center;
        Piece piece;

        int row;
        int col;

    public:
        Cell() {}
        Cell(Point center_) : type(Type::Invalid), center(center_) {}

        void clear()
        {
            piece.clear();
        }

        bool has_piece() const
        {
            return !piece.is_empty();
        }
        Piece get_piece()
        {
            return piece;
        }

        Point get_center() const
        {
            return center;
        }
        Position get_position() const
        {
            return Position(row, col);
        }
        Bounds get_bounds();
    };

public:
    Chessboard();
    virtual ~Chessboard() {}

    void reset();

    NodePtr get_board_mesh();
    NodePtr get_move_marker_mesh();
    NodePtr get_capture_marker_mesh();
    NodePtr get_attack_marker_mesh();

    Cell &operator()(int row, int col)
    {
        return board[row][col];
    }

    bool is_piece(const std::string &node_id);
    Cell &find_piece(const std::string &name);

    Side local_side() const
    {
        return this_side;
    }

    void clear_selection()
    {
        selected = Position(-1, -1);
    }
    bool select(Cell &cell);
    bool select(Piece &piece);
    Position get_selected() const
    {
        return selected;
    }

    bool move_selected_to(int row, int col);
    bool move_to(const Piece &piece, int row, int col);

    ListStringList valid_paths(int row, int col);
    ListStringList valid_paths(Cell &cell);
    ListStringList valid_paths(Piece &cell);

protected: // data members
    Cell board[8][8];

    int white_capture_index{0};
    Cell white_capture[16];

    int black_capture_index{0};
    Cell black_capture[16];

    Side this_side{White};

    Position selected;

    static MeshMap mesh_map;
    static NodePtr board_mesh;
    static NodePtr move_marker_mesh;
    static NodePtr capture_marker_mesh;
    static NodePtr attack_marker_mesh;

protected: // methods
    ListStringList calc_valid_paths(int row, int col);
    ListStringList calc_pawn_moves(int row, int col);
    ListStringList calc_knight_moves(int row, int col);
    ListStringList calc_rook_moves(int row, int col);
    ListStringList calc_bishop_moves(int row, int col);
    ListStringList calc_king_moves(int row, int col);
    ListStringList calc_queen_moves(int row, int col);
};

using ChessboardPtr = osg::ref_ptr<Chessboard>;
