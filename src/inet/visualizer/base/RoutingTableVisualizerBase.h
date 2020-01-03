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

#include "inet/common/StringFormat.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/networklayer/ipv4/Ipv4RoutingTable.h"
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
        mutable int numRoutes = 1;
        const Ipv4Route *route = nullptr;

      public:
        RouteVisualization(const Ipv4Route *route, int nodeModuleId, int nextHopModuleId);
        virtual ~RouteVisualization() {}
    };

    class DirectiveResolver : public StringFormat::IDirectiveResolver {
      protected:
        const Ipv4Route *route = nullptr;

      public:
        DirectiveResolver(const Ipv4Route *route) : route(route) { }

        virtual const char *resolveDirective(char directive) const override;
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

    std::map<std::tuple<const Ipv4Route *, int, int>, const RouteVisualization *> routeVisualizations;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;

    virtual void subscribe();
    virtual void unsubscribe();

    virtual const RouteVisualization *createRouteVisualization(Ipv4Route *route, cModule *node, cModule *nextHop) const = 0;
    virtual const RouteVisualization *getRouteVisualization(Ipv4Route *route, int nodeModuleId, int nextHopModuleId);
    virtual void addRouteVisualization(const RouteVisualization *routeVisualization);
    virtual void removeRouteVisualization(const RouteVisualization *routeVisualization);

    virtual std::vector<Ipv4Address> getDestinations();

    virtual void addRouteVisualizations(IIpv4RoutingTable *routingTable);
    virtual void removeRouteVisualizations(IIpv4RoutingTable *routingTable);
    virtual void removeAllRouteVisualizations();
    virtual void updateRouteVisualizations(IIpv4RoutingTable *routingTable);
    virtual void updateAllRouteVisualizations();

    virtual std::string getRouteVisualizationText(const Ipv4Route *route) const;
    virtual void refreshRouteVisualization(const RouteVisualization *routeVisualization) const = 0;

  public:
    virtual ~RoutingTableVisualizerBase();

    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *obj, cObject *details) override;
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_ROUTINGTABLEVISUALIZERBASE_H

