//
// Copyright (C) OpenSim Ltd.
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

#ifndef __INET_INTERFACETABLEOSGVISUALIZER_H
#define __INET_INTERFACETABLEOSGVISUALIZER_H

#include "inet/visualizer/base/InterfaceTableVisualizerBase.h"
#include "inet/visualizer/scene/NetworkNodeOsgVisualization.h"
#include "inet/visualizer/scene/NetworkNodeOsgVisualizer.h"

#ifdef WITH_OSG
#include <osg/Node>
#endif // ifdef WITH_OSG

namespace inet {

namespace visualizer {

class INET_API InterfaceTableOsgVisualizer : public InterfaceTableVisualizerBase
{
#ifdef WITH_OSG

  protected:
    class INET_API InterfaceOsgVisualization : public InterfaceVisualization {
      public:
        NetworkNodeOsgVisualization *networkNodeVisualization = nullptr;
        osg::Node *node = nullptr;

      public:
        InterfaceOsgVisualization(NetworkNodeOsgVisualization *networkNodeVisualization, osg::Node *node, int networkNodeId, int networkNodeGateId, int interfaceId);
    };

  protected:
    NetworkNodeOsgVisualizer *networkNodeVisualizer = nullptr;

  protected:
    virtual void initialize(int stage) override;

    virtual InterfaceVisualization *createInterfaceVisualization(cModule *networkNode, InterfaceEntry *interfaceEntry) override;
    virtual void addInterfaceVisualization(const InterfaceVisualization *interfaceVisualization) override;
    virtual void removeInterfaceVisualization(const InterfaceVisualization *interfaceVisualization) override;
    virtual void refreshInterfaceVisualization(const InterfaceVisualization *interfaceVisualization, const InterfaceEntry *interfaceEntry) override;

  public:
    virtual ~InterfaceTableOsgVisualizer();

#else // ifdef WITH_OSG

  protected:
    virtual void initialize(int stage) override {}

    virtual InterfaceVisualization *createInterfaceVisualization(cModule *networkNode, InterfaceEntry *interfaceEntry) override { return nullptr; }
    virtual void refreshInterfaceVisualization(const InterfaceVisualization *interfaceVisualization, const InterfaceEntry *interfaceEntry) override { }

#endif // ifdef WITH_OSG
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_INTERFACETABLEOSGVISUALIZER_H

