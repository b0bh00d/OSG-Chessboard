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

#include <osgViewer/Viewer>
#include <osgGA/TrackballManipulator>
#include <osgGA/TerrainManipulator>
#include <osgGA/UFOManipulator>
#include <osg/NodeCallback>
#include <osg/Camera>
#include <osg/Group>
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/Registry>
#include <osg/Notify>
#include <osg/Vec3>
#include <osg/Quat>

#include "Types.h"

using NodePtr = osg::ref_ptr< osg::Node >;
using GroupPtr = osg::ref_ptr< osg::Group >;

using MeshMap = std::map< std::string, NodePtr >;
using MeshMapIter = MeshMap::iterator;
using MeshMapConstIter = MeshMap::const_iterator;
