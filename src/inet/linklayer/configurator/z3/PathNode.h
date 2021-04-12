#ifndef __INET_Z3_PATHNODE_H
#define __INET_Z3_PATHNODE_H

#include <z3++.h>

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * [Class]: PathNode
 * [Usage]: Contains the data needed in each node of
 * a pathTree. Can reference a father, possesses an
 * device or switch, a list of children and a flow
 * fragment for each children in case of being a switch.
 *
 */
class INET_API PathNode {

    PathNode parent; // The parent of the current FlowNode
    Object node;
    ArrayList<PathNode> children; // The children of the current FlowNode
    ArrayList<FlowFragment> flowFragments;


    /**
     * [Method]: PathNode
     * [Usage]: Overloaded constructor method of the this class.
     * Receives an object that must be either a device or a switch.
     * In case of switch, creates a list of children and flowFragments.
     *
     * @param node      Device or a Switch that represents the node
     */
    PathNode (Object node)
    {
        if((node instanceof TSNSwitch) || (node instanceof Switch)) {
            this.node = node;
            children = new ArrayList<PathNode>();
            flowFragments = new ArrayList<FlowFragment>();
        } else if (node instanceof Device) {
            this.node = node;
            children = null;
        } else {
            //[TODO]: Throw error
        }

        children  = new ArrayList<PathNode>();
    }

    /**
     * [Method]: addChild
     * [Usage]: Adds a child to this node.
     *
     * @param node      Object representing the child device or switch
     * @return          A reference to the newly created node
     */
    PathNode addChild(Object node)
    {
        PathNode pathNode = new PathNode(node);
        pathNode.setParent(this);
        children.add(pathNode);

        return pathNode;
    }

    /*
     * GETTERS AND SETTERS:
     */

    PathNode getParent() {
        return parent;
    }

    void setParent(PathNode parent) {
        this.parent = parent;
    }

    Object getNode() {
        return node;
    }

    void setNode(Object node) {
        this.node = node;
    }

    ArrayList<PathNode> getChildren() {
        return children;
    }

    void setChildren(ArrayList<PathNode> children) {
        this.children = children;
    }

    void addFlowFragment(FlowFragment flowFragment) {
        this.flowFragments.add(flowFragment);
    }

    ArrayList<FlowFragment> getFlowFragments() {
        return flowFragments;
    }

    void setFlowFragment(ArrayList<FlowFragment>  flowFragments) {
        this.flowFragments = flowFragments;
    }

};

}

#endif

