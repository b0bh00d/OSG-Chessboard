#------------------------------------------------------------------------------
# MIT License
#
# Copyright (c) 2020 Bob Hood
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.
#------------------------------------------------------------------------------

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <tuple>
#include <cassert>

#include "Game.h"
#include "Chessboard.h" // includes OSG.h

#include <stdio.h>
#include <sys/stat.h> // for stat()

static const std::string side_name[] = {"", "Black", "White"};
static const std::string rank_name[] = {"", "Rook", "Knight", "Bishop", "King", "Queen", "Pawn"};

static Chessboard::Piece::Rank white_major_type[] = {
    Chessboard::Piece::Rank::Rook,  Chessboard::Piece::Rank::Knight, Chessboard::Piece::Rank::Bishop, Chessboard::Piece::Rank::King,
    Chessboard::Piece::Rank::Queen, Chessboard::Piece::Rank::Bishop, Chessboard::Piece::Rank::Knight, Chessboard::Piece::Rank::Rook};
static const char *white_major_name[] = {"WKR", "WKK", "WKB", "WKING", "WQUEEN", "WQB", "WQK", "WQR"};
static const char *white_minor_name[] = {"WP1", "WP2", "WP3", "WP4", "WP5", "WP6", "WP7", "WP8"};

static Chessboard::Piece::Rank black_major_type[] = {
    Chessboard::Piece::Rank::Rook, Chessboard::Piece::Rank::Knight, Chessboard::Piece::Rank::Bishop, Chessboard::Piece::Rank::Queen,
    Chessboard::Piece::Rank::King, Chessboard::Piece::Rank::Bishop, Chessboard::Piece::Rank::Knight, Chessboard::Piece::Rank::Rook};
static const char *black_major_name[] = {"BQR", "BQK", "BQB", "BQUEEN", "BKING", "BKB", "BKK", "BKR"};
static const char *black_minor_name[] = {"BP1", "BP2", "BP3", "BP4", "BP5", "BP6", "BP7", "BP8"};

static const char *board_id = "Chess.Board";

MeshMap Chessboard::mesh_map;
NodePtr Chessboard::board_mesh;
NodePtr Chessboard::move_marker_mesh;
NodePtr Chessboard::capture_marker_mesh;
NodePtr Chessboard::attack_marker_mesh;

Chessboard::Piece::Piece() {}

Chessboard::Piece::Piece(Rank rank_, Side side_, float facing_) : rank(rank_), side(side_), facing(facing_) {}

Chessboard::Piece::Piece(const Piece &source)
{
    *this = source;
}

Chessboard::Piece &Chessboard::Piece::operator=(const Chessboard::Piece &source)
{
    rank = source.rank;
    side = source.side;
    facing = source.facing;
    row = source.row;
    col = source.col;
    name = source.name;

    first_move = source.first_move;
    captured = source.captured;
    in_check = source.in_check;

    return *this;
}

void Chessboard::Piece::load_mesh(const std::string &id)
{
    auto iter = mesh_map.find(id);
    if (iter == mesh_map.end())
    {
        NodePtr mesh;

        // load it

        std::string content_path = "Objects";
        std::string piece_path = content_path + "/" + side_name[side] + "/" + rank_name[static_cast<std::uint32_t>(rank)];
        std::string piece_osg = piece_path + ".osg";
        std::string piece_lwo = piece_path + ".lwo";

        struct stat osg_st, lwo_st;

        auto osg_exists = (stat(piece_osg.c_str(), &osg_st) == 0);
        auto lwo_exists = (stat(piece_lwo.c_str(), &lwo_st) == 0);

        if (osg_exists && lwo_exists)
        {
            // if LWO is newer, cause it to be re-loaded and re-saved

            if (lwo_st.st_mtime <= osg_st.st_mtime)
                lwo_exists = false;
        }

        if (lwo_exists)
        {
            // load in the LWO file
            mesh = osgDB::readNodeFile(piece_lwo);
            if (mesh.valid())
            {
                // save it as OSG for later loading
                auto result = osgDB::writeNodeFile(*(mesh.get()), piece_osg.c_str());
                if (!result)
                    osg::notify(osg::FATAL) << "Failed in osgDB::writeNodeFile()." << std::endl;
            }
        }
        else // only the OSG format exists
            mesh = osgDB::readNodeFile(piece_osg);

        if (mesh.valid())
        {
            mesh_map[id] = mesh;
            iter = mesh_map.find(id);
        }
    }

    if (iter != mesh_map.end())
    {
        iter->second->setName(id.c_str());
        // std::string name = iter->second->getName();
    }
}

NodePtr Chessboard::Piece::get_mesh()
{
    const auto iter = mesh_map.find(name);
    if (iter == mesh_map.end())
        return NodePtr();
    return iter->second.get();
}

void Chessboard::Piece::capture()
{
    captured = true;
    row = -1;
    col = -1;
}

void Chessboard::Piece::move_to(int row_, int col_)
{
    // we don't validate the move; we expect the caller has ensured that
    // this move is valid for our rank
    row = row_;
    col = col_;
    first_move = false;
}

Bounds Chessboard::Cell::get_bounds()
{
    return Bounds(center.x - 0.025, center.y - 0.025, center.x + 0.025, center.y + 0.025);
}

Chessboard::Chessboard()
{
    std::string content_path = "Objects";

    auto load_mesh = [](NodePtr &node, const std::string &content_path, const std::string &base_name) {
        if (!node.valid())
        {
            // see if an OSG-format mesh file exists
            std::string osg_name = content_path + "/" + base_name + ".osg";
            node = osgDB::readNodeFile(osg_name);
            if (!node.valid())
            {
                // load in the LWO file
                std::string lwo_name = content_path + "/" + base_name + ".lwo";
                node = osgDB::readNodeFile(lwo_name);
                if (node.valid())
                {
                    // save it as OSG for later loading
                    auto result = osgDB::writeNodeFile(*(node.get()), osg_name.c_str());
                    if (!result)
                        osg::notify(osg::FATAL) << "Failed in osgDB::writeNodeFile()." << std::endl;
                }
            }
        }
    };

    load_mesh(board_mesh, content_path, "Board");
    load_mesh(move_marker_mesh, content_path, "MoveMarker");
    load_mesh(capture_marker_mesh, content_path, "CaptureMarker");
    load_mesh(attack_marker_mesh, content_path, "AttackMarker");

    // map each board cell to a world position

    //                 X       Y
    //                COL     ROW
    Point first_cell(-0.175, 0.175); // the 1A cell center
    for (auto row : Game::one_rank)
    {
        for (auto col : Game::one_rank)
        {
            board[row][col].type = Cell::Type::Board;
            board[row][col].center.x = first_cell.x + (col * 0.05);
            board[row][col].center.y = first_cell.y - (row * 0.05);
            board[row][col].row = row;
            board[row][col].col = col;
        }
    }

    // establish the capture areas

    Point white_p(board[0][0].center.x, board[0][0].center.y + 0.1);
    auto index = 0;
    for (auto i : Game::one_side)
    {
        if (i && (i % 8) == 0)
        {
            white_p.x = board[0][0].center.x;
            white_p.y += 0.05;
        }

        white_capture[index].center.x = white_p.x;
        white_p.x += 0.05;
        white_capture[index].center.y = white_p.y;
        ++index;
    }

    Point black_p(board[7][0].center.x, board[7][0].center.y - 0.1);
    index = 0;
    for (auto i : Game::one_side)
    {
        if (i && (i % 8) == 0)
        {
            black_p.x = board[7][0].center.x;
            black_p.y -= 0.05;
        }

        black_capture[index].center.x = black_p.x;
        black_p.x += 0.05;
        black_capture[index].center.y = black_p.y;
        ++index;
    }

    reset();
}

 void Chessboard::reset()
{
    // clear the center of the board

    for (auto row : Game::center_ranks)
    {
        for (auto col : Game::one_rank)
            board[row][col].piece.clear();
    }

    // White

    for (auto row : Game::white_ranks)
    {
        for (auto col : Game::one_rank)
        {
            // rotate the piece around to face the enemy
            board[row][col].piece.set_facing(1.0f);

            if (!row)
            {
                board[row][col].piece.set_rank(white_major_type[col]);
                board[row][col].piece.set_name(white_major_name[col]);
            }
            else
            {
                board[row][col].piece.set_rank(Piece::Rank::Pawn);
                board[row][col].piece.set_name(white_minor_name[col]);
            }

            board[row][col].piece.set_side(White);
            board[row][col].piece.load_mesh(board[row][col].piece.get_name());
        }
    }

    // Black

    for (auto row : Game::black_ranks)
    {
        for (auto col : Game::one_rank)
        {
            if (row == 6)
            {
                board[row][col].piece.set_rank(Piece::Rank::Pawn);
                board[row][col].piece.set_name(black_minor_name[col]);
            }
            else
            {
                board[row][col].piece.set_rank(black_major_type[col]);
                board[row][col].piece.set_name(black_major_name[col]);
            }

            board[row][col].piece.set_side(Black);
            board[row][col].piece.load_mesh(board[row][col].piece.get_name());
        }
    }
}

NodePtr Chessboard::get_board_mesh()
{
    if (!board_mesh.valid())
        return NodePtr();
    return board_mesh.get();
}

NodePtr Chessboard::get_move_marker_mesh()
{
    if (!move_marker_mesh.valid())
        return NodePtr();
    return move_marker_mesh.get();
}

NodePtr Chessboard::get_capture_marker_mesh()
{
    if (!capture_marker_mesh.valid())
        return NodePtr();
    return capture_marker_mesh.get();
}

NodePtr Chessboard::get_attack_marker_mesh()
{
    if (!attack_marker_mesh.valid())
        return NodePtr();
    return attack_marker_mesh.get();
}

bool Chessboard::is_piece(const std::string &node_id)
{
    MeshMapConstIter iter;
    for (iter = mesh_map.begin(); iter != mesh_map.end(); ++iter)
    {
        auto mesh = &(*iter->second);
        if (mesh->getName() == node_id)
            return true;
    }

    return false;
}

Chessboard::Cell &Chessboard::find_piece(const std::string &name)
{
    for (auto row : Game::one_rank)
    {
        for (auto col : Game::one_rank)
        {
            if (!board[row][col].has_piece())
                continue;
            if (board[row][col].piece.get_name() == name)
                return board[row][col];
        }
    }

    // check the capture squares

    for (auto i : Game::one_side)
    {
        if (!black_capture[i].has_piece())
            continue;
        if (black_capture[i].piece.get_name() == name)
            return black_capture[i];
    }

    for (auto i : Game::one_side)
    {
        if (!white_capture[i].has_piece())
            continue;
        if (white_capture[i].piece.get_name() == name)
            return white_capture[i];
    }

    throw "Invalid piece information provided!";
}

bool Chessboard::select(Cell &cell)
{
    if (!board[cell.row][cell.col].has_piece())
        return false;

    selected = Position(cell.row, cell.col);

    return true;
}

bool Chessboard::select(Piece &piece)
{
    if (piece.is_captured())
        return false;

    selected = piece.get_position();

    return true;
}

bool Chessboard::move_selected_to(int row, int col)
{
    int selected_row, selected_col;
    std::tie(selected_row, selected_col) = selected;

    if (!board[selected_row][selected_col].has_piece())
        return false;

    if (board[row][col].has_piece())
    {
        // it's an opposing piece; this is an attack.
        // move the piece to my next available capture
        // spot

        if (this_side == White)
            white_capture[white_capture_index++].piece = board[row][col].piece;
        else
            black_capture[black_capture_index++].piece = board[row][col].piece;

        board[row][col].piece.clear();
    }

    board[row][col].piece = board[selected_row][selected_col].piece;
    board[row][col].piece.move_to(row, col);
    board[selected_row][selected_col].piece.clear();

    if (this_side == White)
        this_side = Black;
    else
        this_side = White;

    return true;
}

bool Chessboard::move_to(const Piece &/*piece*/, int /*row*/, int /*col*/)
{
    return false;
}

// given a cell, piece or board position, calculate a list of
// valid paths that the board piece at that location can take

ListStringList Chessboard::valid_paths(int row, int col)
{
    if (board[row][col].type != Cell::Type::Board || board[row][col].piece.get_rank() == Piece::Rank::Empty)
        return ListStringList();

    return calc_valid_paths(row, col);
}

ListStringList Chessboard::valid_paths(Cell &cell)
{
    int row, col;
    std::tie(row, col) = cell.get_position();

    if (board[row][col].type != Cell::Type::Board || board[row][col].piece.get_rank() == Piece::Rank::Empty)
        return ListStringList();

    return calc_valid_paths(row, col);
}

ListStringList Chessboard::valid_paths(Piece &piece)
{
    int row, col;
    std::tie(row, col) = piece.get_position();

    if (board[row][col].type != Cell::Type::Board || board[row][col].piece.get_rank() == Piece::Rank::Empty)
        return ListStringList();

    return calc_valid_paths(row, col);
}

ListStringList Chessboard::calc_valid_paths(int row, int col)
{
    auto rank = board[row][col].piece.get_rank();

    switch (rank)
    {
        case Piece::Rank::Rook:
            return calc_rook_moves(row, col);
        case Piece::Rank::Knight:
            return calc_knight_moves(row, col);
        case Piece::Rank::Bishop:
            return calc_bishop_moves(row, col);
        case Piece::Rank::King:
            return calc_king_moves(row, col);
        case Piece::Rank::Queen:
            return calc_queen_moves(row, col);
        case Piece::Rank::Pawn:
            return calc_pawn_moves(row, col);
        case Piece::Rank::Empty:
            break;
    }

    return ListStringList();
}

ListStringList Chessboard::calc_pawn_moves(int row, int col)
{
    // for pawns. white moves in increasing rows (1->8), while
    // black moves in decreasing (8->1).  also, the first move
    // may be two (unblocked) squares instead of one.

    auto piece = board[row][col].piece;
    auto side = piece.get_side();

    ListStringList paths;

    if (side == White)
    {
        // check forward progress
        auto first_square_clear = false;
        if (row < 7 && board[row + 1][col].piece.get_rank() == Piece::Rank::Empty)
        {
            first_square_clear = true;

            // we can move here
            std::stringstream square_name_stream;
            square_name_stream << "Marker.Move." << (row + 1) << "." << col;
            std::string square_name = square_name_stream.str();

            StringList target_squares;
            target_squares.push_back(square_name);
            paths.push_back(target_squares);
        }

        if (first_square_clear && !piece.has_moved() && board[row + 2][col].piece.get_rank() == Piece::Rank::Empty)
        {
            // we can move here
            std::stringstream square_name_stream;
            square_name_stream << "Marker.Move." << (row + 2) << "." << col;
            std::string square_name = square_name_stream.str();

            StringList target_squares;
            target_squares.push_back(square_name);
            paths.push_back(target_squares);
        }

        // check for attack possibilities to the right
        if (col > 0 && row < 7 && board[row + 1][col - 1].piece.get_rank() != Piece::Rank::Empty &&
            board[row + 1][col - 1].piece.get_side() == Black)
        {
            // we can move here
            std::stringstream square_name_stream;
            square_name_stream << "Marker.Attack." << (row + 1) << "." << (col - 1);
            std::string square_name = square_name_stream.str();

            StringList target_squares;
            target_squares.push_back(square_name);
            paths.push_back(target_squares);
        }

        // check for attack possibilities to the left
        if (col < 7 && row < 7 && board[row + 1][col + 1].piece.get_rank() != Piece::Rank::Empty &&
            board[row + 1][col + 1].piece.get_side() == Black)
        {
            // we can move here
            std::stringstream square_name_stream;
            square_name_stream << "Marker.Attack." << (row + 1) << "." << (col + 1);
            std::string square_name = square_name_stream.str();

            StringList target_squares;
            target_squares.push_back(square_name);
            paths.push_back(target_squares);
        }
    }
    else
    {
        // check forward progress
        if (row > 0 && board[row - 1][col].piece.get_rank() == Piece::Rank::Empty)
        {
            // we can move here
            std::stringstream square_name_stream;
            square_name_stream << "Marker.Move." << (row - 1) << "." << col;
            std::string square_name = square_name_stream.str();

            StringList target_squares;
            target_squares.push_back(square_name);
            paths.push_back(target_squares);
        }

        if (!piece.has_moved() && board[row - 2][col].piece.get_rank() == Piece::Rank::Empty)
        {
            // we can move here
            std::stringstream square_name_stream;
            square_name_stream << "Marker.Move." << (row - 2) << "." << col;
            std::string square_name = square_name_stream.str();

            StringList target_squares;
            target_squares.push_back(square_name);
            paths.push_back(target_squares);
        }

        // check for attack possibilities to the right
        if (col < 7 && row > 0 && board[row - 1][col + 1].piece.get_rank() != Piece::Rank::Empty &&
            board[row - 1][col + 1].piece.get_side() == White)
        {
            // we can move here
            std::stringstream square_name_stream;
            square_name_stream << "Marker.Attack." << (row - 1) << "." << (col + 1);
            std::string square_name = square_name_stream.str();

            StringList target_squares;
            target_squares.push_back(square_name);
            paths.push_back(target_squares);
        }

        // check for attack possibilities to the left
        if (col > 0 && row > 0 && board[row - 1][col - 1].piece.get_rank() != Piece::Rank::Empty &&
            board[row - 1][col - 1].piece.get_side() == White)
        {
            // we can move here
            std::stringstream square_name_stream;
            square_name_stream << "Marker.Attack." << (row - 1) << "." << (col - 1);
            std::string square_name = square_name_stream.str();

            StringList target_squares;
            target_squares.push_back(square_name);
            paths.push_back(target_squares);
        }
    }

    return paths;
}

ListStringList Chessboard::calc_knight_moves(int row, int col)
{
    // knight moves are about as easy as pawns.  at any given time,
    // a knight can move to one of eight targets, leaping over any
    // other pieces in its way.

    auto piece = board[row][col].piece;
    Piece::Rank target_rank;

    auto side = piece.get_side();
    Side opponent, target_side;
    if (side == White)
        opponent = Black;
    else
        opponent = White;

    ListStringList paths;

    // check forward...

    // up one...

    if (row < 7)
    {
        // ...over two

        if (col < 6)
        {
            target_rank = board[row + 1][col + 2].piece.get_rank();
            target_side = board[row + 1][col + 2].piece.get_side();

            if (target_rank == Piece::Rank::Empty || target_side == opponent)
            {
                // we can move here
                std::stringstream square_name_stream;
                square_name_stream << "Marker."
                                   << ((target_rank == Piece::Rank::Empty || target_side != opponent) ? "Move" : "Attack")
                                   << "." << (row + 1) << "." << (col + 2);
                std::string square_name = square_name_stream.str();

                StringList target_squares;
                target_squares.push_back(square_name);
                paths.push_back(target_squares);
            }
        }

        if (col > 1)
        {
            target_rank = board[row + 1][col - 2].piece.get_rank();
            target_side = board[row + 1][col - 2].piece.get_side();

            if (target_rank == Piece::Rank::Empty || target_side == opponent)
            {
                // we can move here
                std::stringstream square_name_stream;
                square_name_stream << "Marker."
                                   << ((target_rank == Piece::Rank::Empty || target_side != opponent) ? "Move" : "Attack")
                                   << "." << (row + 1) << "." << (col - 2);
                std::string square_name = square_name_stream.str();

                StringList target_squares;
                target_squares.push_back(square_name);
                paths.push_back(target_squares);
            }
        }
    }

    // up two...

    if (row < 6)
    {
        // ...over one

        if (col < 7)
        {
            target_rank = board[row + 2][col + 1].piece.get_rank();
            target_side = board[row + 2][col + 1].piece.get_side();

            if (target_rank == Piece::Rank::Empty || target_side == opponent)
            {
                // we can move here
                std::stringstream square_name_stream;
                square_name_stream << "Marker."
                                   << ((target_rank == Piece::Rank::Empty || target_side != opponent) ? "Move" : "Attack")
                                   << "." << (row + 2) << "." << (col + 1);
                std::string square_name = square_name_stream.str();

                StringList target_squares;
                target_squares.push_back(square_name);
                paths.push_back(target_squares);
            }
        }

        if (col > 0)
        {
            target_rank = board[row + 2][col - 1].piece.get_rank();
            target_side = board[row + 2][col - 1].piece.get_side();

            if (target_rank == Piece::Rank::Empty || target_side == opponent)
            {
                // we can move here
                std::stringstream square_name_stream;
                square_name_stream << "Marker."
                                   << ((target_rank == Piece::Rank::Empty || target_side != opponent) ? "Move" : "Attack")
                                   << "." << (row + 2) << "." << (col - 1);
                std::string square_name = square_name_stream.str();

                StringList target_squares;
                target_squares.push_back(square_name);
                paths.push_back(target_squares);
            }
        }
    }

    // check backward...

    // back one...

    if (row > 0)
    {
        // ...over two

        if (col < 6)
        {
            target_rank = board[row - 1][col + 2].piece.get_rank();
            target_side = board[row - 1][col + 2].piece.get_side();

            if (target_rank == Piece::Rank::Empty || target_side == opponent)
            {
                // we can move here
                std::stringstream square_name_stream;
                square_name_stream << "Marker."
                                   << ((target_rank == Piece::Rank::Empty || target_side != opponent) ? "Move" : "Attack")
                                   << "." << (row - 1) << "." << (col + 2);
                std::string square_name = square_name_stream.str();

                StringList target_squares;
                target_squares.push_back(square_name);
                paths.push_back(target_squares);
            }
        }

        if (col > 1)
        {
            target_rank = board[row - 1][col - 2].piece.get_rank();
            target_side = board[row - 1][col - 2].piece.get_side();

            if (target_rank == Piece::Rank::Empty || target_side == opponent)
            {
                // we can move here
                std::stringstream square_name_stream;
                square_name_stream << "Marker."
                                   << ((target_rank == Piece::Rank::Empty || target_side != opponent) ? "Move" : "Attack")
                                   << "." << (row - 1) << "." << (col - 2);
                std::string square_name = square_name_stream.str();

                StringList target_squares;
                target_squares.push_back(square_name);
                paths.push_back(target_squares);
            }
        }
    }

    // back two...

    if (row > 1)
    {
        // ...over one

        if (col < 7)
        {
            target_rank = board[row - 2][col + 1].piece.get_rank();
            target_side = board[row - 2][col + 1].piece.get_side();

            if (target_rank == Piece::Rank::Empty || target_side == opponent)
            {
                // we can move here
                std::stringstream square_name_stream;
                square_name_stream << "Marker."
                                   << ((target_rank == Piece::Rank::Empty || target_side != opponent) ? "Move" : "Attack")
                                   << "." << (row - 2) << "." << (col + 1);
                std::string square_name = square_name_stream.str();

                StringList target_squares;
                target_squares.push_back(square_name);
                paths.push_back(target_squares);
            }
        }

        if (col > 0)
        {
            target_rank = board[row - 2][col - 1].piece.get_rank();
            target_side = board[row - 2][col - 1].piece.get_side();

            if (target_rank == Piece::Rank::Empty || target_side == opponent)
            {
                // we can move here
                std::stringstream square_name_stream;
                square_name_stream << "Marker."
                                   << ((target_rank == Piece::Rank::Empty || target_side != opponent) ? "Move" : "Attack")
                                   << "." << (row - 2) << "." << (col - 1);
                std::string square_name = square_name_stream.str();

                StringList target_squares;
                target_squares.push_back(square_name);
                paths.push_back(target_squares);
            }
        }
    }

    return paths;
}

ListStringList Chessboard::calc_rook_moves(int row, int col)
{
    // rooks move in row-column order, any number of moves until
    // they meet opposition or the end of the board.
    // castling is also verified, which requires the king and the
    // king's rook to not yet have moved.

    auto piece = board[row][col].piece;

    auto side = piece.get_side();
    Side opponent;
    if (side == White)
        opponent = Black;
    else
        opponent = White;

    ListStringList paths;

    auto i = row + 1;

    StringList target_squares;

    while (i < 8)
    {
        if (board[i][col].piece.get_rank() == Piece::Rank::Empty)
        {
            // we can move here
            std::stringstream square_name_stream;
            square_name_stream << "Marker.Move." << i << "." << col;
            std::string square_name = square_name_stream.str();

            target_squares.push_back(square_name);
        }
        else
        {
            // a piece is in our way, so we stop here.  if it is
            // an opposing piece, we can move to this square (attack)

            if (board[i][col].piece.get_side() == opponent)
            {
                // we can move here
                std::stringstream square_name_stream;
                square_name_stream << "Marker.Attack." << i << "." << col;
                std::string square_name = square_name_stream.str();

                target_squares.push_back(square_name);
            }

            break;
        }

        ++i;
    }

    if (target_squares.size())
    {
        paths.push_back(target_squares);
        target_squares.clear();
    }

    i = row - 1;

    while (i >= 0)
    {
        if (board[i][col].piece.get_rank() == Piece::Rank::Empty)
        {
            // we can move here
            std::stringstream square_name_stream;
            square_name_stream << "Marker.Move." << i << "." << col;
            std::string square_name = square_name_stream.str();

            target_squares.push_back(square_name);
        }
        else
        {
            // a piece is in our way, so we stop here.  if it is
            // an opposing piece, we can move to this square (attack)

            if (board[i][col].piece.get_side() == opponent)
            {
                // we can move here
                std::stringstream square_name_stream;
                square_name_stream << "Marker.Attack." << i << "." << col;
                std::string square_name = square_name_stream.str();

                target_squares.push_back(square_name);
            }

            break;
        }

        --i;
    }

    if (target_squares.size())
    {
        paths.push_back(target_squares);
        target_squares.clear();
    }

    i = col + 1;

    while (i < 8)
    {
        if (board[row][i].piece.get_rank() == Piece::Rank::Empty)
        {
            // we can move here
            std::stringstream square_name_stream;
            square_name_stream << "Marker.Move." << row << "." << i;
            std::string square_name = square_name_stream.str();

            target_squares.push_back(square_name);
        }
        else
        {
            // a piece is in our way, so we stop here.  if it is
            // an opposing piece, we can move to this square (attack)

            if (board[row][i].piece.get_side() == opponent)
            {
                // we can move here
                std::stringstream square_name_stream;
                square_name_stream << "Marker.Attack." << row << "." << i;
                std::string square_name = square_name_stream.str();

                target_squares.push_back(square_name);
            }

            break;
        }

        ++i;
    }

    if (target_squares.size())
    {
        paths.push_back(target_squares);
        target_squares.clear();
    }

    i = col - 1;

    while (i >= 0)
    {
        if (board[row][i].piece.get_rank() == Piece::Rank::Empty)
        {
            // we can move here
            std::stringstream square_name_stream;
            square_name_stream << "Marker.Move." << row << "." << i;
            std::string square_name = square_name_stream.str();

            target_squares.push_back(square_name);
        }
        else
        {
            // a piece is in our way, so we stop here.  if it is
            // an opposing piece, we can move to this square (attack)

            if (board[row][i].piece.get_side() == opponent)
            {
                // we can move here
                std::stringstream square_name_stream;
                square_name_stream << "Marker.Attack." << row << "." << i;
                std::string square_name = square_name_stream.str();

                target_squares.push_back(square_name);
            }

            break;
        }

        --i;
    }

    if (target_squares.size())
    {
        paths.push_back(target_squares);
        target_squares.clear();
    }

    if (!piece.has_moved())
    {
        // check for castle by running along the column until we hit
        // the king, another piece, or the end of the board.  if we
        // hit the king, and the king has not yet moved, we have a
        // potential castle move available.

        bool found_the_king = false;

        // go left first

        i = col - 1;
        while (i >= 0)
        {
            if (board[row][i].piece.get_rank() != Piece::Rank::Empty)
            {
                if (board[row][i].piece.get_rank() == Piece::Rank::King && board[row][i].piece.get_side() == side &&
                    !board[row][i].piece.has_moved())
                {
                    found_the_king = true;

                    // we can move here
                    std::stringstream square_name_stream;
                    square_name_stream << "Marker.Move." << row << "." << i + 1;
                    std::string square_name = square_name_stream.str();

                    target_squares.push_back(square_name);
                }

                break;
            }

            --i;
        }

        if (!found_the_king)
        {
            // check right

            i = col + 1;
            while (i < 8)
            {
                if (board[row][i].piece.get_rank() != Piece::Rank::Empty)
                {
                    if (board[row][i].piece.get_rank() == Piece::Rank::King && board[row][i].piece.get_side() == side &&
                        !board[row][i].piece.has_moved())
                    {
                        // we can move here
                        std::stringstream square_name_stream;
                        square_name_stream << "Marker.Move." << row << "." << i - 1;
                        std::string square_name = square_name_stream.str();

                        target_squares.push_back(square_name);
                    }

                    break;
                }

                ++i;
            }
        }
    }

    if (target_squares.size())
    {
        paths.push_back(target_squares);
        target_squares.clear();
    }

    return paths;
}

ListStringList Chessboard::calc_bishop_moves(int row, int col)
{
    // bishops move diagonally until they meet opposition or the
    // end of the board.

    auto piece = board[row][col].piece;

    auto side = piece.get_side();
    Side opponent;
    if (side == White)
        opponent = Black;
    else
        opponent = White;

    ListStringList paths;

    StringList target_squares;

    auto i = row + 1;
    auto j = col + 1;

    while (i < 8 && j < 8)
    {
        if (board[i][j].piece.get_rank() == Piece::Rank::Empty)
        {
            // we can move here
            std::stringstream square_name_stream;
            square_name_stream << "Marker.Move." << i << "." << j;
            std::string square_name = square_name_stream.str();

            target_squares.push_back(square_name);
        }
        else
        {
            // a piece is in our way, so we stop here.  if it is
            // an opposing piece, we can move to this square (attack)

            if (board[i][j].piece.get_side() == opponent)
            {
                // we can move here
                std::stringstream square_name_stream;
                square_name_stream << "Marker.Attack." << i << "." << j;
                std::string square_name = square_name_stream.str();

                target_squares.push_back(square_name);
            }

            break;
        }

        i++;
        j++;
    }

    if (target_squares.size())
    {
        paths.push_back(target_squares);
        target_squares.clear();
    }

    i = row + 1;
    j = col - 1;

    while (i < 8 && j >= 0)
    {
        if (board[i][j].piece.get_rank() == Piece::Rank::Empty)
        {
            // we can move here
            std::stringstream square_name_stream;
            square_name_stream << "Marker.Move." << i << "." << j;
            std::string square_name = square_name_stream.str();

            target_squares.push_back(square_name);
        }
        else
        {
            // a piece is in our way, so we stop here.  if it is
            // an opposing piece, we can move to this square (attack)

            if (board[i][j].piece.get_side() == opponent)
            {
                // we can move here
                std::stringstream square_name_stream;
                square_name_stream << "Marker.Attack." << i << "." << j;
                std::string square_name = square_name_stream.str();

                target_squares.push_back(square_name);
            }

            break;
        }

        i++;
        --j;
    }

    if (target_squares.size())
    {
        paths.push_back(target_squares);
        target_squares.clear();
    }

    i = row - 1;
    j = col + 1;

    while (i >= 0 && j < 8)
    {
        if (board[i][j].piece.get_rank() == Piece::Rank::Empty)
        {
            // we can move here
            std::stringstream square_name_stream;
            square_name_stream << "Marker.Move." << i << "." << j;
            std::string square_name = square_name_stream.str();

            target_squares.push_back(square_name);
        }
        else
        {
            // a piece is in our way, so we stop here.  if it is
            // an opposing piece, we can move to this square (attack)

            if (board[i][j].piece.get_side() == opponent)
            {
                // we can move here
                std::stringstream square_name_stream;
                square_name_stream << "Marker.Attack." << i << "." << j;
                std::string square_name = square_name_stream.str();

                target_squares.push_back(square_name);
            }

            break;
        }

        --i;
        j++;
    }

    if (target_squares.size())
    {
        paths.push_back(target_squares);
        target_squares.clear();
    }

    i = row - 1;
    j = col - 1;

    while (i >= 0 && j >= 0)
    {
        if (board[i][j].piece.get_rank() == Piece::Rank::Empty)
        {
            // we can move here
            std::stringstream square_name_stream;
            square_name_stream << "Marker.Move." << i << "." << j;
            std::string square_name = square_name_stream.str();

            target_squares.push_back(square_name);
        }
        else
        {
            // a piece is in our way, so we stop here.  if it is
            // an opposing piece, we can move to this square (attack)

            if (board[i][j].piece.get_side() == opponent)
            {
                // we can move here
                std::stringstream square_name_stream;
                square_name_stream << "Marker.Attack." << i << "." << j;
                std::string square_name = square_name_stream.str();

                target_squares.push_back(square_name);
            }

            break;
        }

        --i;
        --j;
    }

    if (target_squares.size())
    {
        paths.push_back(target_squares);
        target_squares.clear();
    }

    return paths;
}

ListStringList Chessboard::calc_king_moves(int row, int col)
{
    // the king can move any direction, but only one square at a time.

    auto piece = board[row][col].piece;

    auto side = piece.get_side();
    Side opponent;
    if (side == White)
        opponent = Black;
    else
        opponent = White;

    ListStringList paths;

    // the king cannot move if he is in check, unless the move gets him
    // out of check

    if (piece.is_checked())
        return paths;

    // check the eight squares around me

    int squares[][2] = {{row - 1, col}, {row - 1, col - 1}, {row - 1, col + 1}, {row, col - 1},
                        {row, col + 1}, {row + 1, col},     {row + 1, col - 1}, {row + 1, col + 1}};

    for (int i = 0; i < 8; i++)
    {
        auto r = squares[i][0];
        auto c = squares[i][1];

        if (r < 0 || r > 7 || c < 0 || c > 7)
            continue;

        if (board[r][c].piece.get_rank() == Piece::Rank::Empty)
        {
            // we can move here
            std::stringstream square_name_stream;
            square_name_stream << "Marker.Move." << r << "." << c;
            std::string square_name = square_name_stream.str();

            StringList target_squares;
            target_squares.push_back(square_name);
            paths.push_back(target_squares);
        }
        else
        {
            // a piece is in our way.  if it is an opposing piece, we
            // can move to this square (attack)

            if (board[r][c].piece.get_side() == opponent)
            {
                // we can move here
                std::stringstream square_name_stream;
                square_name_stream << "Marker.Attack." << r << "." << c;
                std::string square_name = square_name_stream.str();

                StringList target_squares;
                target_squares.push_back(square_name);
                paths.push_back(target_squares);
            }
        }
    }

    return paths;
}

ListStringList Chessboard::calc_queen_moves(int row, int col)
{
    // the queen can move any direction, any number of squares
    // at a time, until another piece, or the end of the board,
    // is encountered

    auto piece = board[row][col].piece;

    auto side = piece.get_side();
    Side opponent;
    if (side == White)
        opponent = Black;
    else
        opponent = White;

    ListStringList paths;

    StringList target_squares;

    auto i = row + 1;
    auto j = col + 1;

    while (i < 8 && j < 8)
    {
        if (board[i][j].piece.get_rank() == Piece::Rank::Empty)
        {
            // we can move here
            std::stringstream square_name_stream;
            square_name_stream << "Marker.Move." << i << "." << j;
            std::string square_name = square_name_stream.str();

            target_squares.push_back(square_name);
        }
        else
        {
            // a piece is in our way, so we stop here.  if it is
            // an opposing piece, we can move to this square (attack)

            if (board[i][j].piece.get_side() == opponent)
            {
                // we can move here
                std::stringstream square_name_stream;
                square_name_stream << "Marker.Attack." << i << "." << j;
                std::string square_name = square_name_stream.str();

                target_squares.push_back(square_name);
            }

            break;
        }

        i++;
        j++;
    }

    if (target_squares.size())
    {
        paths.push_back(target_squares);
        target_squares.clear();
    }

    i = row + 1;

    while (i < 8)
    {
        if (board[i][col].piece.get_rank() == Piece::Rank::Empty)
        {
            // we can move here
            std::stringstream square_name_stream;
            square_name_stream << "Marker.Move." << i << "." << col;
            std::string square_name = square_name_stream.str();

            target_squares.push_back(square_name);
        }
        else
        {
            // a piece is in our way, so we stop here.  if it is
            // an opposing piece, we can move to this square (attack)

            if (board[i][col].piece.get_side() == opponent)
            {
                // we can move here
                std::stringstream square_name_stream;
                square_name_stream << "Marker.Attack." << i << "." << col;
                std::string square_name = square_name_stream.str();

                target_squares.push_back(square_name);
            }

            break;
        }

        ++i;
    }

    if (target_squares.size())
    {
        paths.push_back(target_squares);
        target_squares.clear();
    }

    i = row + 1;
    j = col - 1;

    while (i < 8 && j >= 0)
    {
        if (board[i][j].piece.get_rank() == Piece::Rank::Empty)
        {
            // we can move here
            std::stringstream square_name_stream;
            square_name_stream << "Marker.Move." << i << "." << j;
            std::string square_name = square_name_stream.str();

            target_squares.push_back(square_name);
        }
        else
        {
            // a piece is in our way, so we stop here.  if it is
            // an opposing piece, we can move to this square (attack)

            if (board[i][j].piece.get_side() == opponent)
            {
                // we can move here
                std::stringstream square_name_stream;
                square_name_stream << "Marker.Attack." << i << "." << j;
                std::string square_name = square_name_stream.str();

                target_squares.push_back(square_name);
            }

            break;
        }

        i++;
        --j;
    }

    if (target_squares.size())
    {
        paths.push_back(target_squares);
        target_squares.clear();
    }

    j = col - 1;

    while (j >= 0)
    {
        if (board[row][j].piece.get_rank() == Piece::Rank::Empty)
        {
            // we can move here
            std::stringstream square_name_stream;
            square_name_stream << "Marker.Move." << row << "." << j;
            std::string square_name = square_name_stream.str();

            target_squares.push_back(square_name);
        }
        else
        {
            // a piece is in our way, so we stop here.  if it is
            // an opposing piece, we can move to this square (attack)

            if (board[row][j].piece.get_side() == opponent)
            {
                // we can move here
                std::stringstream square_name_stream;
                square_name_stream << "Marker.Attack." << row << "." << j;
                std::string square_name = square_name_stream.str();

                target_squares.push_back(square_name);
            }

            break;
        }

        --j;
    }

    if (target_squares.size())
    {
        paths.push_back(target_squares);
        target_squares.clear();
    }

    j = col + 1;

    while (j < 8)
    {
        if (board[row][j].piece.get_rank() == Piece::Rank::Empty)
        {
            // we can move here
            std::stringstream square_name_stream;
            square_name_stream << "Marker.Move." << row << "." << j;
            std::string square_name = square_name_stream.str();

            target_squares.push_back(square_name);
        }
        else
        {
            // a piece is in our way, so we stop here.  if it is
            // an opposing piece, we can move to this square (attack)

            if (board[row][j].piece.get_side() == opponent)
            {
                // we can move here
                std::stringstream square_name_stream;
                square_name_stream << "Marker.Attack." << row << "." << j;
                std::string square_name = square_name_stream.str();

                target_squares.push_back(square_name);
            }

            break;
        }

        ++j;
    }

    if (target_squares.size())
    {
        paths.push_back(target_squares);
        target_squares.clear();
    }

    i = row - 1;
    j = col + 1;

    while (i >= 0 && j < 8)
    {
        if (board[i][j].piece.get_rank() == Piece::Rank::Empty)
        {
            // we can move here
            std::stringstream square_name_stream;
            square_name_stream << "Marker.Move." << i << "." << j;
            std::string square_name = square_name_stream.str();

            target_squares.push_back(square_name);
        }
        else
        {
            // a piece is in our way, so we stop here.  if it is
            // an opposing piece, we can move to this square (attack)

            if (board[i][j].piece.get_side() == opponent)
            {
                // we can move here
                std::stringstream square_name_stream;
                square_name_stream << "Marker.Attack." << i << "." << j;
                std::string square_name = square_name_stream.str();

                target_squares.push_back(square_name);
            }

            break;
        }

        --i;
        j++;
    }

    if (target_squares.size())
    {
        paths.push_back(target_squares);
        target_squares.clear();
    }

    i = row - 1;

    while (i >= 0)
    {
        if (board[i][col].piece.get_rank() == Piece::Rank::Empty)
        {
            // we can move here
            std::stringstream square_name_stream;
            square_name_stream << "Marker.Move." << i << "." << col;
            std::string square_name = square_name_stream.str();

            target_squares.push_back(square_name);
        }
        else
        {
            // a piece is in our way, so we stop here.  if it is
            // an opposing piece, we can move to this square (attack)

            if (board[i][col].piece.get_side() == opponent)
            {
                // we can move here
                std::stringstream square_name_stream;
                square_name_stream << "Marker.Attack." << i << "." << col;
                std::string square_name = square_name_stream.str();

                target_squares.push_back(square_name);
            }

            break;
        }

        --i;
    }

    if (target_squares.size())
    {
        paths.push_back(target_squares);
        target_squares.clear();
    }

    i = row - 1;
    j = col - 1;

    while (i >= 0 && j >= 0)
    {
        if (board[i][j].piece.get_rank() == Piece::Rank::Empty)
        {
            // we can move here
            std::stringstream square_name_stream;
            square_name_stream << "Marker.Move." << i << "." << j;
            std::string square_name = square_name_stream.str();

            target_squares.push_back(square_name);
        }
        else
        {
            // a piece is in our way, so we stop here.  if it is
            // an opposing piece, we can move to this square (attack)

            if (board[i][j].piece.get_side() == opponent)
            {
                // we can move here
                std::stringstream square_name_stream;
                square_name_stream << "Marker.Attack." << i << "." << j;
                std::string square_name = square_name_stream.str();

                target_squares.push_back(square_name);
            }

            break;
        }

        --i;
        --j;
    }

    if (target_squares.size())
    {
        paths.push_back(target_squares);
        target_squares.clear();
    }

    return paths;
}
