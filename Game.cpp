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

#include "Game.h"
#include "Callbacks.h"

// these may be overkill (because the board size will never change) but they improve code readability
const std::vector<int> Game::one_rank{0, 1, 2, 3, 4, 5, 6, 7};
const std::vector<int> Game::one_side{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
const std::vector<int> Game::white_ranks{0, 1};
const std::vector<int> Game::center_ranks{2, 3, 4, 6};
const std::vector<int> Game::black_ranks{6, 7};

Game::Game()
{
    sg_root = createScene();
}

void Game::construct_move_squares(ChessboardPtr chessboard, GroupPtr &squares)
{
    Chessboard &cb = *chessboard;

    for (auto row : Game::one_rank)
    {
        for (auto col : Game::one_rank)
        {
            auto cell = cb(row, col);
            auto center = cell.get_center();

            std::stringstream square_name_stream;
            square_name_stream << "Marker.Move." << row << "." << col;
            auto square_name = square_name_stream.str();

            osg::ref_ptr<osg::Switch> switch_node(new osg::Switch);
            switch_node->setNewChildDefaultValue(false);
            switch_node->setName(square_name.c_str());
            switch_node->setDataVariance(osg::Object::DYNAMIC);

            osg::Matrix square_matrix;
            square_matrix.makeTranslate(center.x, center.y, 0.021);

            osg::ref_ptr<osg::MatrixTransform> mt(new osg::MatrixTransform(square_matrix));
            mt->addChild(cb.get_move_marker_mesh());
            mt->setDataVariance(osg::Object::STATIC);

            switch_node->addChild(mt);

            squares->addChild(switch_node);
        }
    }
}

void Game::construct_capture_squares(ChessboardPtr chessboard, GroupPtr &squares)
{
    Chessboard &cb = *chessboard;

    for (auto row : Game::one_rank)
    {
        for (auto col : Game::one_rank)
        {
            auto cell = cb(row, col);
            auto center = cell.get_center();

            std::stringstream square_name_stream;
            square_name_stream << "Marker.Attack." << row << "." << col;
            auto square_name = square_name_stream.str();

            osg::ref_ptr<osg::Switch> switch_node(new osg::Switch);
            switch_node->setNewChildDefaultValue(false);
            switch_node->setName(square_name.c_str());
            switch_node->setDataVariance(osg::Object::DYNAMIC);

            osg::Matrix square_matrix;
            square_matrix.makeTranslate(center.x, center.y, 0.021);

            osg::ref_ptr<osg::MatrixTransform> mt(new osg::MatrixTransform(square_matrix));
            mt->addChild(cb.get_capture_marker_mesh());
            mt->setDataVariance(osg::Object::DYNAMIC);

            switch_node->addChild(mt);

            squares->addChild(switch_node);
        }
    }
}

void Game::construct_attack_markers(ChessboardPtr chessboard, GroupPtr &attack_group)
{
    Chessboard &cb = *chessboard;

    {
        osg::Vec3d pos(0.275, 0.175, 0.0);
        osg::Vec3d axis(0., 0., 0.);
        osg::Quat att(0, axis);

        osg::ref_ptr<osg::PositionAttitudeTransform> patt(new osg::PositionAttitudeTransform);
        patt->setPosition(pos);
        patt->setAttitude(att);
        patt->setDataVariance(osg::Object::DYNAMIC);
        patt->addChild(cb.get_attack_marker_mesh());
        patt->setUpdateCallback(new RotateAttackMarkerCallback());
        patt->setName("Rotate.White.Attack.Marker");

        osg::ref_ptr<osg::Switch> switch_node(new osg::Switch);
        switch_node->setNewChildDefaultValue(true);
        switch_node->setName("Switch.White.Attack.Marker");
        switch_node->addChild(patt.get());
        switch_node->setUpdateCallback(new EnableAttackMarkerCallback(chessboard));
        switch_node->setDataVariance(osg::Object::DYNAMIC);

        attack_group->addChild(switch_node.get());
    }

    {
        osg::Vec3d pos1(0., 0., 0.);
        osg::Vec3d axis1(0., 0., 0.);
        osg::Quat att1(0., axis1);

        osg::ref_ptr<osg::PositionAttitudeTransform> patt1(new osg::PositionAttitudeTransform);
        patt1->setPosition(pos1);
        patt1->setAttitude(att1);
        patt1->setDataVariance(osg::Object::DYNAMIC);
        patt1->addChild(cb.get_attack_marker_mesh());
        patt1->setUpdateCallback(new RotateAttackMarkerCallback());
        patt1->setName("Rotate.Black.Attack.Marker");

        osg::Vec3d pos2(-0.275, -0.175, 0.0);
        osg::Vec3d axis2(0., 0., 1.);
        osg::Quat att2(M_PI, axis2);

        osg::ref_ptr<osg::PositionAttitudeTransform> patt2(new osg::PositionAttitudeTransform);
        patt2->setPosition(pos2);
        patt2->setAttitude(att2);
        patt2->setDataVariance(osg::Object::STATIC);
        patt2->addChild(patt1.get());
        patt2->setName("Position.Black.Attack.Marker");

        osg::ref_ptr<osg::Switch> switch_node(new osg::Switch);
        switch_node->setNewChildDefaultValue(false);
        switch_node->setName("Switch.Black.Attack.Marker");
        switch_node->addChild(patt2.get());
        switch_node->setUpdateCallback(new EnableAttackMarkerCallback(chessboard));
        switch_node->setDataVariance(osg::Object::DYNAMIC);

        attack_group->addChild(switch_node.get());
    }
}

NodePtr Game::createScene()
{
    chessboard = ChessboardPtr(new Chessboard);

    GroupPtr root = new osg::Group;
    root->setName("Root");
    root->setDataVariance(osg::Object::STATIC);

    osg::Matrix board_matrix;
    board_matrix.makeTranslate(0., 0., 0.);

    osg::ref_ptr<osg::MatrixTransform> mt(new osg::MatrixTransform(board_matrix));
    mt->addChild(chessboard->get_board_mesh());
    mt->setDataVariance(osg::Object::STATIC);
    mt->setName("Chess.Board");

    GroupPtr move_squares(new osg::Group);
    move_squares->setName("Board.Move.Squares");
    move_squares->setDataVariance(osg::Object::DYNAMIC);
    construct_move_squares(chessboard, move_squares);

    mt->addChild(move_squares.get());

    GroupPtr capture_squares(new osg::Group);
    capture_squares->setName("Board.Capture.Squares");
    capture_squares->setDataVariance(osg::Object::DYNAMIC);
    construct_capture_squares(chessboard, capture_squares);

    mt->addChild(capture_squares.get());

    root->addChild(mt.get());

    GroupPtr attack_markers(new osg::Group);
    attack_markers->setName("Board.Attack.Markers");
    attack_markers->setDataVariance(osg::Object::DYNAMIC);
    construct_attack_markers(chessboard, attack_markers);

    root->addChild(attack_markers.get());

    for (auto row : Game::one_rank)
    {
        for (auto col : Game::one_rank)
        {
            Chessboard::Cell cell = (*chessboard)(row, col);
            if (!cell.has_piece())
                continue;

            auto position = cell.get_center();
            auto piece = cell.get_piece();
#if 0
             std::cout << "[" << row << "][" << col << "] " << piece.get_name();

             osg::Matrix translate_matrix;
             translate_matrix.makeTranslate( position.x, position.y, 0.020f );
             mt = new osg::MatrixTransform( translate_matrix );
             mt->setDataVariance( osg::Object::DYNAMIC );
             mt->addChild( piece.get_mesh() );
             root->addChild( mt.get() );
#endif
            osg::Vec3d pos(position.x, position.y, 0.020);
            osg::Vec3d axis(0., 0., 1.);
            osg::Quat att(M_PI * static_cast<double>(piece.get_facing()), axis);

            osg::ref_ptr<osg::PositionAttitudeTransform> patt(new osg::PositionAttitudeTransform);
            patt->setPosition(pos);
            patt->setAttitude(att);
            patt->setDataVariance(osg::Object::DYNAMIC);
            patt->addChild(piece.get_mesh());
            patt->setName(piece.get_name());
            patt->setUpdateCallback(new PositionPieceCallback(chessboard));

            root->addChild(patt.get());
        }
    }

    osg::ref_ptr<osg::Light> light(new osg::Light);
    light->setAmbient(osg::Vec4(.1f, .1f, .1f, 1.f));
    light->setDiffuse(osg::Vec4(.8f, .8f, .8f, 1.f));
    light->setSpecular(osg::Vec4(.8f, .8f, .8f, 1.f));
    light->setPosition(osg::Vec4(0.f, 0.f, 0.f, 1.f));
    light->setDirection(osg::Vec3(0.f, 0.f, -1.f));
    // light->setSpotCutoff( 180.f );

    // Create a MatrixTransform to position the Light.

    osg::Matrix m;
    m.makeTranslate(osg::Vec3(0.f, 0.f, 0.75f));

    osg::ref_ptr<osg::MatrixTransform> light_matrix(new osg::MatrixTransform);
    light_matrix->setMatrix(m);
    light_matrix->setName("LightMatrix");

    // Add the Light to a LightSource. Add the LightSource and
    // MatrixTransform to the scene graph.

    osg::ref_ptr<osg::LightSource> ls(new osg::LightSource);
    ls->setName("LightSource");
    light_matrix->addChild(ls.get());
    ls->setLight(light.get());

    root->addChild(light_matrix.get());

    return root.get();
}
