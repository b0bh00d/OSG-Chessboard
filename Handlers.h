#pragma once

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

#include "OSG.h"
#include "Chessboard.h"

// PickHandlerInterface -- An interface class, based on GUIEventHandler,
// that implements picking.  Derived classes need to override the
// pure virtual process_pick() method.

class PickHandlerInterface : public osgGA::GUIEventHandler
{
public:
    PickHandlerInterface();
    bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa ) override;

protected:
    // store mouse xy location for button press & move events.
    float _mX{0.0f};
    float _mY{0.0f};

    // Remember the previous selection;
    osg::ref_ptr<osg::MatrixTransform> _selectedNode;

    // perform a pick operation.
    bool pick( const double x, const double y, osgViewer::Viewer* viewer );

    virtual bool process_pick(const osg::NodePath& nodePath) = 0;
};

class SelectionHandler : public PickHandlerInterface
{
public:
    SelectionHandler( ChessboardPtr board_, NodePtr sg_root_ );
    ~SelectionHandler() override {}

protected:  // data members
    ChessboardPtr   board;
    NodePtr         sg_root;

protected:  // methods
    bool process_pick( const osg::NodePath& nodePath ) override;
};
