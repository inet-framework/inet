//
// Copyright (C) 2006-2015 Opensim Ltd
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_OSGSCENE_H
#define __INET_OSGSCENE_H

#include "inet/common/INETDefs.h"

#if defined(WITH_OSG) && defined(WITH_VISUALIZERS)
#include <osg/Group>
#include <osg/NodeVisitor>
#endif // ifdef WITH_OSG

namespace inet {

namespace osg {

#if defined(WITH_OSG) && defined(WITH_VISUALIZERS)

using namespace ::osg;

template<typename T>
class INET_API FindNodesVisitor : public NodeVisitor
{
  protected:
    std::vector<T*> foundNodes;

  public:
    FindNodesVisitor() : NodeVisitor(NodeVisitor::TRAVERSE_ALL_CHILDREN) {}

    virtual void apply(Node& node) override;

    virtual const std::vector<T *>& getFoundNodes() const { return foundNodes; };
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

#endif // ifdef WITH_OSG

} // namespace osg

} // namespace inet

#endif // ifndef __INET_OSGSCENE_H

