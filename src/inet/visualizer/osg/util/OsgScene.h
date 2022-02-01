//
// Copyright (C) 2006-2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_OSGSCENE_H
#define __INET_OSGSCENE_H

#include "inet/common/INETDefs.h"

#include <osg/Group>
#include <osg/NodeVisitor>

namespace inet {

namespace osg {

using namespace ::osg;

template<typename T>
class INET_API FindNodesVisitor : public NodeVisitor
{
  protected:
    std::vector<T *> foundNodes;

  public:
    FindNodesVisitor() : NodeVisitor(NodeVisitor::TRAVERSE_ALL_CHILDREN) {}

    virtual void apply(Node& node) override;

    virtual const std::vector<T *>& getFoundNodes() const { return foundNodes; }
};

/**
 * This class is used for the root of the OSG nodes of all kinds of simulation
 * visualization. There may be other nodes in the OSG node hierarchy such as map
 * nodes used by OSG Earth.
 */
class INET_API SimulationScene : public Group
{
};

/**
 * This class is used for the topmost node in the OSG node hierarchy. The scene
 * of the OSG canvas is a TopLevelScene instance.
 */
class INET_API TopLevelScene : public Group
{
  protected:
    SimulationScene *simulationScene = nullptr;

  public:
    virtual SimulationScene *getSimulationScene();

    static SimulationScene *getSimulationScene(cModule *module);
};

} // namespace osg

} // namespace inet

#endif

