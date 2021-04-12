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
    public PathNode (Object node)
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
    public PathNode addChild(Object node)
    {
        PathNode pathNode = new PathNode(node);
        pathNode.setParent(this);
        children.add(pathNode);

        return pathNode;
    }

    /*
     * GETTERS AND SETTERS:
     */

    public PathNode getParent() {
        return parent;
    }

    public void setParent(PathNode parent) {
        this.parent = parent;
    }

    public Object getNode() {
        return node;
    }

    public void setNode(Object node) {
        this.node = node;
    }

    public ArrayList<PathNode> getChildren() {
        return children;
    }

    public void setChildren(ArrayList<PathNode> children) {
        this.children = children;
    }

    public void addFlowFragment(FlowFragment flowFragment) {
        this.flowFragments.add(flowFragment);
    }

    public ArrayList<FlowFragment> getFlowFragments() {
        return flowFragments;
    }

    public void setFlowFragment(ArrayList<FlowFragment>  flowFragments) {
        this.flowFragments = flowFragments;
    }

}

}

#endif

