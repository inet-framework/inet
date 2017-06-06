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

#include <tuple>
#include "inet/networklayer/contract/ipv4/IPv4Address.h"
#include "inet/networklayer/ipv4/IPv4RoutingTable.h"
#include "inet/visualizer/base/VisualizerBase.h"
#include "inet/visualizer/util/LineManager.h"
#include "inet/visualizer/util/NetworkNodeFilter.h"
#include "inet/visualizer/util/StringFormat.h"

namespace inet {

namespace visualizer {

class INET_API RoutingTableVisualizerBase : public VisualizerBase, public cListener
{
  protected:
    class INET_API RouteVisualization : public LineManager::ModuleLine {
      public:
        mutable int numRoutes = 1;
        const IPv4Route *route = nullptr;

      public:
        RouteVisualization(const IPv4Route *route, int nodeModuleId, int nextHopModuleId);
        virtual ~RouteVisualization() {}
    };

    class DirectiveResolver : public StringFormat::IDirectiveResolver {
      protected:
        const IPv4Route *route = nullptr;
        std::string result;

      public:
        DirectiveResolver(const IPv4Route *route) : route(route) { }

        virtual const char *resolveDirective(char directive) override;
    };

  protected:
    /** @name Parameters */
    //@{
    bool displayRoutingTables = false;
    bool displayRoutesIndividually = false;
    bool displayLabels = false;
    NetworkNodeFilter destinationFilter;
    NetworkNodeFilter nodeFilter;
    cFigure::Color lineColor;
    cFigure::LineStyle lineStyle;
    double lineShift = NaN;
    const char *lineShiftMode = nullptr;
    double lineWidth = NaN;
    double lineContactSpacing = NaN;
    const char *lineContactMode = nullptr;
    StringFormat labelFormat;
    cFigure::Font labelFont;
    cFigure::Color labelColor;
    //@}

    LineManager *lineManager = nullptr;

    std::map<std::tuple<const IPv4Route *, int, int>, const RouteVisualization *> routeVisualizations;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;

    virtual void subscribe();
    virtual void unsubscribe();

    virtual const RouteVisualization *createRouteVisualization(IPv4Route *route, cModule *node, cModule *nextHop) const = 0;
    virtual const RouteVisualization *getRouteVisualization(IPv4Route *route, int nodeModuleId, int nextHopModuleId);
    virtual void addRouteVisualization(const RouteVisualization *routeVisualization);
    virtual void removeRouteVisualization(const RouteVisualization *routeVisualization);

    virtual std::vector<IPv4Address> getDestinations();

    virtual void addRouteVisualizations(IIPv4RoutingTable *routingTable);
    virtual void removeRouteVisualizations(IIPv4RoutingTable *routingTable);
    virtual void removeAllRouteVisualizations();
    virtual void updateRouteVisualizations(IIPv4RoutingTable *routingTable);
    virtual void updateAllRouteVisualizations();

    virtual std::string getRouteVisualizationText(const IPv4Route *route) const;
    virtual void refreshRouteVisualization(const RouteVisualization *routeVisualization) const = 0;

  public:
    virtual ~RoutingTableVisualizerBase();

    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *obj, cObject *details) override;
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_ROUTINGTABLEVISUALIZERBASE_H

