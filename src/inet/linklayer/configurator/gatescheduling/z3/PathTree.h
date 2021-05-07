//
// Copyright (C) 2021 by original authors
//
// This file is copied from the following project with the explicit permission
// from the authors: https://github.com/ACassimiro/TSNsched
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

#ifndef __INET_Z3_PATHTREE_H
#define __INET_Z3_PATHTREE_H

#include <z3++.h>

#include "inet/linklayer/configurator/gatescheduling/z3/Defs.h"

namespace inet {

using namespace z3;

/**
 * [Class]: PathTree
 * Used to specify the path on publish subscribe
 * flows. It is basically a tree of path nodes with a few
 * simple and classic tree methods.
 *
 */
class INET_API PathTree {
  public:
    PathNode *root = nullptr;
    std::vector<PathNode *> *leaves = new std::vector<PathNode *>();

    /**
     * Adds a root node to the pathTree.
     * The user must give a device or switch to
     * be the root of the tree.
     *
     * @param node      Device of the root node of the pathTree
     * @return          A reference to the root
     */
    PathNode *addRoot(cObject *node)
    {
        root = new PathNode(node);
        root->setParent(nullptr);
        root->setChildren(std::vector<PathNode *>());
        return root;
    }

    /**
     * Given a new PathNode object, make it the
     * new root of this pathTree. Old root becomes child
     * of new root.
     *
     * @param newRoot       New root of pathTree
     */
    void changeRoot(PathNode *newRoot)
    {
        PathNode *oldRoot = root;
        newRoot->setParent(nullptr);
        newRoot->addChild(oldRoot);
        oldRoot->setParent(newRoot);
        this->root = newRoot;
    }

    /**
     * Adds all leaves to the leaves std::vector starting
     * from the node given as a parameter. In the way it is
     * implemented, must be used only once.
     *
     * TODO [Priority: Low]: Renew list on every first call
     *
     * @param node      Starter node of the search
     */
    void searchLeaves(PathNode *node) {
        if (node->getChildren().size() == 0)
            leaves->push_back(node);
        else
            for (PathNode *auxNode : node->getChildren())
                searchLeaves(auxNode);
    }

    /**
     * Returns an std::vector with all the nodes of the
     * pathTree.
     *
     * @return      std::vector with all leaves as PathNodes
     */
    std::vector<PathNode *> *getLeaves(){
        leaves->clear();
        searchLeaves(root);
        return leaves;
    }

    PathNode *getRoot() {
        return root;
    }

    void setRoot(PathNode *root) {
        this->root = root;
    }
};

}

#endif

