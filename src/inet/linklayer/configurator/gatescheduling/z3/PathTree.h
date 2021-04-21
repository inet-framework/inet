#ifndef __INET_Z3_PATHTREE_H
#define __INET_Z3_PATHTREE_H

#include <z3++.h>

#include "inet/linklayer/configurator/gatescheduling/z3/Defs.h"

namespace inet {

using namespace z3;

/**
 * [Class]: PathTree
 * [Usage]: Used to specify the path on publish subscribe
 * flows. It is basically a tree of path nodes with a few
 * simple and classic tree methods.
 *
 */
class INET_API PathTree {
  public:
    PathNode *root = nullptr;
    std::vector<PathNode *> *leaves = new std::vector<PathNode *>();

    /**
     * [Method]: addRoot
     * [Usage]: Adds a root node to the pathTree.
     * The user must give a device or switch to
     * be the root of the tree.
     *
     * @param node      Device of the root node of the pathTree
     * @return          A reference to the root
     */
    PathNode *addRoot(cObject *node)
    {
        root=new PathNode(node);
        root->setParent(nullptr);
        root->setChildren(std::vector<PathNode *>());
        return root;
    }


    /**
     * [Method]: changeRoot
     * [Usage]: Given a new PathNode object, make it the
     * new root of this pathTree. Old root becomes child
     * of new root.
     *
     * @param newRoot       New root of pathTree
     */
    void changeRoot(PathNode *newRoot)
    {
        PathNode *oldRoot=this->root;
        newRoot->setParent(nullptr);
        newRoot->addChild(oldRoot);
        oldRoot->setParent(newRoot);
        this->root=newRoot;
    }


    /**
     * [Method]: searchLeaves
     * [Usage]: Adds all leaves to the leaves std::vector starting
     * from the node given as a parameter. In the way it is
     * implemented, must be used only once.
     *
     * TODO [Priority: Low]: Renew list on every first call
     *
     * @param node      Starter node of the search
     */
    void searchLeaves(PathNode *node) {

        if(node->getChildren().size() == 0) {
            leaves->push_back(node);
            return;
        }

        for(PathNode *auxNode : node->getChildren()) {
            searchLeaves(auxNode);
        }

    }


    /**
     * [Method]: getLeaves
     * [Usage]: Returns an std::vector with all the nodes of the
     * pathTree.
     *
     * @return      std::vector with all leaves as PathNodes
     */
    std::vector<PathNode *> *getLeaves(){
        leaves->clear();

        searchLeaves(root);

        return leaves;
    }


    /*
     * GETTERS AND SETTERS
     */


    PathNode *getRoot() {
        return root;
    }

    void setRoot(PathNode *root) {
        this->root = root;
    }

};

}

#endif

