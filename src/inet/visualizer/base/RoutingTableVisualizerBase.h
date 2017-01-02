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

#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include "inet/networklayer/ipv4/IPv4RoutingTable.h"
#include "inet/visualizer/base/VisualizerBase.h"
#include "inet/visualizer/util/LineManager.h"
#include "inet/visualizer/util/NetworkNodeFilter.h"

namespace inet {

namespace visualizer {

class INET_API RoutingTableVisualizerBase : public VisualizerBase, public cListener
{
  protected:
    class INET_API RouteVisualization : public LineManager::ModuleLine {
      public:
        const int nodeModuleId = -1;
        const int nextHopModuleId = -1;

      public:
        RouteVisualization(int nodeModuleId, int nextHopModuleId);
        virtual ~RouteVisualization() {}
    };

  protected:
    /** @name Parameters */
    //@{
    bool displayRoutingTables = false;
    NetworkNodeFilter destinationFilter;
    NetworkNodeFilter nodeFilter;
    cFigure::Color lineColor;
    cFigure::LineStyle lineStyle;
    double lineShift = NaN;
    const char *lineShiftMode = nullptr;
    double lineWidth = NaN;
    double lineContactSpacing = NaN;
    const char *lineContactMode = nullptr;
    //@}

    LineManager *lineManager = nullptr;

    std::map<std::pair<int, int>, const RouteVisualization *> routeVisualizations;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;

    virtual void subscribe();
    virtual void unsubscribe();

    virtual const RouteVisualization *createRouteVisualization(IPv4Route *route, cModule *node, cModule *nextHop) const = 0;
    virtual const RouteVisualization *getRouteVisualization(std::pair<int, int> route);
    virtual void addRouteVisualization(const RouteVisualization *routeVisualization);
    virtual void removeRouteVisualization(const RouteVisualization *routeVisualization);

    virtual std::vector<IPv4Address> getDestinations();

    virtual void addRouteVisualizations(IIPv4RoutingTable *routingTable);
    virtual void removeRouteVisualizations(IIPv4RoutingTable *routingTable);
    virtual void removeAllRouteVisualizations();
    virtual void updateRouteVisualizations(IIPv4RoutingTable *routingTable);
    virtual void updateAllRouteVisualizations();

  public:
    virtual ~RoutingTableVisualizerBase();

    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *obj, cObject *details) override;
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_ROUTINGTABLEVISUALIZERBASE_H

