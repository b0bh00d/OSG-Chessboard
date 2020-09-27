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

#include "Chessboard.h"
#include "Handlers.h"
#include "Visitors.h"

PickHandlerInterface::PickHandlerInterface() {}

bool PickHandlerInterface::handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa)
{
    auto viewer = dynamic_cast<osgViewer::Viewer *>(&aa);
    if (!viewer)
        return false;

    switch (ea.getEventType())
    {
        case osgGA::GUIEventAdapter::PUSH:
        case osgGA::GUIEventAdapter::MOVE:
            // record mouse location for the button press and move events.
            _mX = ea.getX();
            _mY = ea.getY();
            return false;

        case osgGA::GUIEventAdapter::RELEASE:
            // if the mouse hasn't moved since the last button press
            // or move event, perform a pick. (otherwise, the trackball
            // manipulator will handle it.)

            {
                bool x_eq = compare_f(_mX, ea.getX());
                bool y_eq = compare_f(_mY, ea.getY());

                if (x_eq && y_eq)
                {
                    if (pick(static_cast<double>(ea.getXnormalized()), static_cast<double>(ea.getYnormalized()), viewer))
                        return true;
                }
            }
            return false;

        default:
            break;
    }

    return false;
}

// perform a pick operation.
bool PickHandlerInterface::pick(const double x, const double y, osgViewer::Viewer *viewer)
{
    if (!viewer->getSceneData())
        return false; // nothing to pick

    auto w(.005), h(.005);
    auto picker = new osgUtil::PolytopeIntersector(osgUtil::Intersector::PROJECTION, x - w, y - h, x + w, y + h);
    osgUtil::IntersectionVisitor iv(picker);
    viewer->getCamera()->accept(iv);

    if (picker->containsIntersections())
        return process_pick(picker->getFirstIntersection().nodePath);
    else if (_selectedNode.valid())
    {
        _selectedNode->setUpdateCallback(nullptr);
        _selectedNode = nullptr;
    }

    return _selectedNode.valid();
}

SelectionHandler::SelectionHandler(ChessboardPtr board_, NodePtr sg_root_) :
    PickHandlerInterface(), board(board_), sg_root(sg_root_)
{}

bool SelectionHandler::process_pick(const osg::NodePath &nodePath)
{
    // clear any existing visible markers
    TurnOffMoveHighlights off_visitor;
    sg_root->accept(off_visitor);

    std::string node_id;
    auto tail = nodePath.size();
    while (tail--)
    {
        // find the first PositionAttitudeTransform in the
        // node path; this will be the node that identifies

        auto patt = dynamic_cast<osg::PositionAttitudeTransform *>(nodePath[tail]);
        if (patt == nullptr)
            continue;

        node_id = nodePath[tail]->getName();
        break;
    }

    if (node_id.size())
    {
        tail = nodePath.size() - 1;
        auto node = dynamic_cast<osg::Node *>(nodePath[tail]);
        if (node == nullptr)
            return false;

        // is it a chess piece?

        if (board->is_piece(node_id))
        {
            auto cell = board->find_piece(node_id);
            if (cell.type != Chessboard::Cell::Type::Board)
                return false; // they're trying to select a captured piece

            auto piece = cell.get_piece();
            if (piece.get_side() != board->local_side())
                return false; // trying to select opponent's piece

            board->clear_selection();
            board->select(cell);

            // enable these

            auto paths = board->valid_paths(cell);
            if (paths.size())
            {
                TurnOnMoveHighlights on_visitor(paths);
                sg_root->accept(on_visitor);
            }

            return true;
        }

        return false;
    }

    // is it a marker?

    node_id.clear();
    tail = nodePath.size();
    while (tail--)
    {
        // find the first PositionAttitudeTransform in the
        // node path; this will be the node that identifies

        auto switch_node = dynamic_cast<osg::Switch *>(nodePath[tail]);
        if (switch_node == nullptr)
            continue;

        node_id = nodePath[tail]->getName();
        break;
    }

    if (node_id.size())
    {
        // it's a move
        std::string square_str;

        if (node_id.substr(0, 12) == "Marker.Move.")
            square_str = node_id.substr(12, node_id.size());
        else if (node_id.substr(0, 14) == "Marker.Attack.")
            square_str = node_id.substr(14, node_id.size());

        auto index = square_str.find(".");
        std::string row_str = square_str.substr(0, index);
        std::string col_str = square_str.substr(index + 1, square_str.size());

        auto row = std::stoi(row_str);
        auto col = std::stoi(col_str);

        board->move_selected_to(row, col);

        return true;
    }

    return false;
}
