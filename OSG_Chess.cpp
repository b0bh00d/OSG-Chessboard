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
#include "Handlers.h"

int main(int, char **)
{
    auto game = GamePtr(new Game());

    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow(100, 100, 800, 600);

    auto root = game->get_root_node();
    if (!root.valid())
        osg::notify(osg::FATAL) << "Failed in createScene()." << std::endl;

    viewer.setSceneData(root.get());

    // add the pick handler
    viewer.addEventHandler(new SelectionHandler(game->get_board(), root));

    // Set the clear color to something other than chalky blue.

    viewer.getCamera()->setClearColor(osg::Vec4(1., 1., 1., 1.));

    osg::Matrix lookAt;
    lookAt.makeLookAt(osg::Vec3(0.5f, -0.5f, 1.f), game->centerScope, osg::Vec3(0.0f, 0.0f, 1.0f));

    viewer.getCamera()->setViewMatrix(lookAt);

    viewer.realize();

    while (!viewer.done())
    {
        // fire off the cull-and-draw traversals of the scene
        viewer.frame();
    }

    return 0;
}
