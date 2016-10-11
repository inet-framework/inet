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

#ifndef __INET_ROUTINGTABLEVISUALIZERBASE_H
#define __INET_ROUTINGTABLEVISUALIZERBASE_H

#include "inet/common/PatternMatcher.h"
#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include "inet/networklayer/ipv4/IPv4RoutingTable.h"
#include "inet/visualizer/base/VisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API RoutingTableVisualizerBase : public VisualizerBase, public cListener
{
  protected:
    class INET_API RouteVisualization {
      public:
        const int nodeModuleId;
        const int nextHopModuleId;

      public:
        RouteVisualization(int nodeModuleId, int nextHopModuleId);
        virtual ~RouteVisualization() {}
    };

  protected:
    /** @name Parameters */
    //@{
    cModule *subscriptionModule = nullptr;
    inet::PatternMatcher destinationMatcher;
    cFigure::Color lineColor;
    double lineWidth = NaN;
    cFigure::LineStyle lineStyle;
    //@}

    std::map<std::pair<int, int>, const RouteVisualization *> routeVisualizations;

  protected:
    virtual void initialize(int stage) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *obj DETAILS_ARG) override;

    virtual void setPosition(cModule *node, const Coord& position) const = 0;

    virtual const RouteVisualization *createRouteVisualization(cModule *node, cModule *nextHop) const = 0;
    virtual const RouteVisualization *getRouteVisualization(std::pair<int, int> route);
    virtual void addRouteVisualization(std::pair<int, int> nodeAndNextHop, const RouteVisualization *routeVisualization);
    virtual void removeRouteVisualization(const RouteVisualization *routeVisualization);

    virtual std::vector<IPv4Address> getDestinations();

    virtual void addRoutes(IPv4RoutingTable *routingTable);
    virtual void removeRoutes(IPv4RoutingTable *routingTable);
    virtual void updateRoutes(IPv4RoutingTable *routingTable);

};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_ROUTINGTABLEVISUALIZERBASE_H

