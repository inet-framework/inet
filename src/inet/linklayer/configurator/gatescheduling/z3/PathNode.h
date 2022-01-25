//
// Copyright (C) 2021 by original authors
//
// This file is copied from the following project with the explicit permission
// from the authors: https://github.com/ACassimiro/TSNsched
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#ifndef __INET_Z3_PATHNODE_H
#define __INET_Z3_PATHNODE_H

#include <z3++.h>

#include "inet/linklayer/configurator/gatescheduling/z3/Switch.h"

namespace inet {

using namespace z3;

class FlowFragment;

/**
 * Contains the data needed in each node of
 * a pathTree. Can reference a father, possesses an
 * device or switch, a list of children and a flow
 * fragment for each children in case of being a switch.
 *
 */
class INET_API PathNode : public cObject {
  public:
    PathNode *parent; // The parent of the current FlowNode
    cObject *node;
    std::vector<PathNode *> children; // The children of the current FlowNode
    std::vector<FlowFragment *> flowFragments;

    /**
     * Overloaded constructor method of the this class.
     * Receives an object that must be either a device or a switch.
     * In case of switch, creates a list of children and flowFragments.
     *
     * @param node      Device or a Switch that represents the node
     */
    PathNode (cObject *node)
    {
        if (dynamic_cast<Switch *>(node)) {
            this->node = node;
            children.clear();
            flowFragments.clear();
        } else if (dynamic_cast<Device *>(node)) {
            this->node = node;
            children.clear(); // NOTE: before port: children = nullptr;
        } else {
            //[TODO]: Throw error
        }

        children.clear();
    }

    /**
     * Adds a child to this node.
     *
     * @param node      Object representing the child device or switch
     * @return          A reference to the newly created node
     */
    PathNode *addChild(cObject *node)
    {
        PathNode *pathNode = new PathNode(node);
        pathNode->setParent(this);
        children.push_back(pathNode);

        return pathNode;
    }

    /*
     * GETTERS AND SETTERS:
     */

    PathNode *getParent() {
        return parent;
    }

    void setParent(PathNode *parent) {
        this->parent = parent;
    }

    cObject *getNode() {
        return node;
    }

    void setNode(cObject *node) {
        this->node = node;
    }

    std::vector<PathNode *> getChildren() {
        return children;
    }

    void setChildren(std::vector<PathNode *> children) {
        this->children = children;
    }

    int getChildIndex(PathNode *pathNode) {
        return std::find(children.begin(), children.end(), pathNode) - children.begin();
    }

    void addFlowFragment(FlowFragment *flowFragment) {
        this->flowFragments.push_back(flowFragment);
    }

    std::vector<FlowFragment *> getFlowFragments() {
        return flowFragments;
    }

    void setFlowFragment(std::vector<FlowFragment *>  flowFragments) {
        this->flowFragments = flowFragments;
    }
};

}

#endif

