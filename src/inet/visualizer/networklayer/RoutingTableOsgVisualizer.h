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

#ifndef __INET_ROUTINGTABLEOSGVISUALIZER_H
#define __INET_ROUTINGTABLEOSGVISUALIZER_H

#include "inet/visualizer/base/RoutingTableVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API RoutingTableOsgVisualizer : public RoutingTableVisualizerBase
{
#ifdef WITH_OSG

  protected:
    class INET_API RouteOsgVisualization : public RouteVisualization {
      public:
        osg::Node *node = nullptr;

      public:
        RouteOsgVisualization(osg::Node *node, const Ipv4Route *route, int nodeModuleId, int nextHopModuleId);
        virtual ~RouteOsgVisualization();
    };

  protected:
    virtual void initialize(int stage) override;

    virtual const RouteVisualization *createRouteVisualization(Ipv4Route *route, cModule *node, cModule *nextHop) const override;
    virtual void addRouteVisualization(const RouteVisualization *routeVisualization) override;
    virtual void removeRouteVisualization(const RouteVisualization *routeVisualization) override;
    virtual void refreshRouteVisualization(const RouteVisualization *routeVisualization) const override;

#else // ifdef WITH_OSG

  protected:
    virtual void initialize(int stage) override {}

    virtual const RouteVisualization *createRouteVisualization(Ipv4Route *route, cModule *node, cModule *nextHop) const override { return nullptr; }
    virtual void refreshRouteVisualization(const RouteVisualization *routeVisualization) const override { }

#endif // ifdef WITH_OSG
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_ROUTINGTABLEOSGVISUALIZER_H

