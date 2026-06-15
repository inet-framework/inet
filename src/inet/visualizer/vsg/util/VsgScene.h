//
// Copyright (C) 2006-2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

//
// VulkanSceneGraph (VSG) port of OsgScene.h. This is the OSG-free 3D scene root
// used by the inet.visualizer.vsg (VisualizationVsg) visualizers, mirroring the
// OpenSceneGraph (inet::osg) version. See also OMNeT++'s qtenv/vsg backend.
//

#ifndef __INET_VSGSCENE_H
#define __INET_VSGSCENE_H

#include "inet/common/INETDefs.h"

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>
#include <vsg/nodes/Group.h>

namespace inet {

namespace vsg {

using namespace ::vsg;

/**
 * Collects all nodes of a given type T in the subgraph rooted at the visited node.
 * VSG counterpart of the OSG FindNodesVisitor (a vsg::Visitor instead of an
 * osg::NodeVisitor). Overriding apply(Node&) is sufficient: VSG's Visitor default
 * implementations chain the concrete node types up to apply(Node&), and we descend
 * explicitly via traverse().
 */
template<typename T>
class INET_API FindNodesVisitor : public ::vsg::Visitor
{
  protected:
    std::vector<T *> foundNodes;

  public:
    virtual void apply(::vsg::Node& node) override
    {
        if (auto result = dynamic_cast<T *>(&node))
            foundNodes.push_back(result);
        node.traverse(*this);
    }

    virtual const std::vector<T *>& getFoundNodes() const { return foundNodes; }
};

/**
 * This class is used for the root of the VSG nodes of all kinds of simulation
 * visualization. There may be other nodes in the VSG node hierarchy such as map
 * nodes used for geospatial visualization.
 */
class INET_API SimulationScene : public Inherit<Group, SimulationScene>
{
};

/**
 * This class is used for the topmost node in the VSG node hierarchy. The scene
 * root held by the cOsgCanvas (wrapped in an omnetpp::cScene3DNode) is a
 * TopLevelScene instance.
 */
class INET_API TopLevelScene : public Inherit<Group, TopLevelScene>
{
  protected:
    SimulationScene *simulationScene = nullptr;

  public:
    virtual SimulationScene *getSimulationScene();

    static SimulationScene *getSimulationScene(cModule *module);
};

} // namespace vsg

} // namespace inet

#endif
