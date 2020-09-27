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

#include "Visitors.h"

TurnOffMoveHighlights::TurnOffMoveHighlights() : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}

void TurnOffMoveHighlights::apply(osg::Node &node)
{
    auto name = node.getName();
    if (name.size() > 6 && name.substr(0, 7) == "Marker.")
    {
        auto switch_node = dynamic_cast<osg::Switch *>(&node);
        if (switch_node)
            switch_node->setAllChildrenOff();
    }

    traverse(node);
}

TurnOnMoveHighlights::TurnOnMoveHighlights(ListStringList targets_) :
    osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN), targets(targets_)
{}

 void TurnOnMoveHighlights::apply(osg::Node &node)
{
    auto name = node.getName();
    if (name.size() > 6 && name.substr(0, 7) == "Marker.")
    {
        // see if this name is in the targets list
        for (const auto &lsl_iter : targets)
        {
            for (const auto &sl_iter : lsl_iter)
            {
                if (sl_iter == name)
                {
                    auto switch_node = dynamic_cast<osg::Switch *>(&node);
                    if (switch_node)
                        switch_node->setAllChildrenOn();
                }
            }
        }
    }

    traverse(node);
}
