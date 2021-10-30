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

#include "Callbacks.h"

 PositionPieceCallback::PositionPieceCallback(ChessboardPtr board_) : board(board_) {}

void PositionPieceCallback::operator()(osg::Node *node, osg::NodeVisitor *nv)
{
    if (nv->getVisitorType() != osg::NodeVisitor::UPDATE_VISITOR)
        return;

    auto node_id = node->getName();
    if (board->is_piece(node_id))
    {
        Chessboard::Cell &cell = board->find_piece(node_id);
        Chessboard::Piece piece = cell.get_piece();

        auto center = cell.get_center();

        auto patt = dynamic_cast<osg::PositionAttitudeTransform *>(node);
        if (patt != nullptr)
        {
            osg::Vec3d pos(center.x, center.y, 0.020);
            osg::Vec3d current_position = patt->getPosition();

            auto pos0_eq = compare_f<double>(current_position[0], pos[0]);
            auto pos1_eq = compare_f<double>(current_position[1], pos[1]);

            if (!pos0_eq || !pos1_eq)
                patt->setPosition(pos);
        }
    }

    traverse(node, nv);
}

EnableAttackMarkerCallback::EnableAttackMarkerCallback(ChessboardPtr board_) : board(board_) {}

void EnableAttackMarkerCallback::operator()(osg::Node *node, osg::NodeVisitor *nv)
{
    if (nv->getVisitorType() != osg::NodeVisitor::UPDATE_VISITOR)
        return;

    auto node_id = node->getName();
    if (node_id == "Switch.White.Attack.Marker")
    {
        osg::Switch *switch_node = dynamic_cast<osg::Switch *>(node);

        if (board->local_side() == Chessboard::Black)
        {
            // turn off the White maker and remove its rotation
            if (switch_node->getValue(0))
                switch_node->setValue(0, false);
        }
        else
        {
            if (!switch_node->getValue(0))
                switch_node->setValue(0, true);
        }
    }
    else if (node_id == "Switch.Black.Attack.Marker")
    {
        osg::Switch *switch_node = dynamic_cast<osg::Switch *>(node);

        if (board->local_side() == Chessboard::White)
        {
            // turn off the White maker and remove its rotation
            if (switch_node->getValue(0))
                switch_node->setValue(0, false);
        }
        else
        {
            if (!switch_node->getValue(0))
                switch_node->setValue(0, true);
        }
    }

    traverse(node, nv);
}

RotateAttackMarkerCallback::RotateAttackMarkerCallback() {}

void RotateAttackMarkerCallback::operator()(osg::Node *node, osg::NodeVisitor *nv)
{
    if (nv->getVisitorType() != osg::NodeVisitor::UPDATE_VISITOR)
        return;

    auto node_id = node->getName();
    if (node_id == "Rotate.White.Attack.Marker")
    {
        // rotate this marker

        auto patt = dynamic_cast<osg::PositionAttitudeTransform *>(node);
        auto att = patt->getAttitude();

        double angle;
        osg::Vec3d axis;
        att.getRotate(angle, axis);

        axis.set(0., 1., 0.);
        angle += 0.01;
        if (angle > (2 * M_PI))
            angle = 0.;

        osg::Quat new_att(angle, axis);
        patt->setAttitude(new_att);
    }
    else if (node_id == "Rotate.Black.Attack.Marker")
    {
        // rotate this marker

        auto patt = dynamic_cast<osg::PositionAttitudeTransform *>(node);
        auto att = patt->getAttitude();

        double angle;
        osg::Vec3d axis;
        att.getRotate(angle, axis);

        axis.set(0., 1., 0.);
        angle -= 0.01;
        if (angle < 0.)
            angle = (2 * M_PI);

        osg::Quat new_att(angle, axis);
        patt->setAttitude(new_att);
    }

    traverse(node, nv);
}
