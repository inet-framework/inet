//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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

    class INET_API DirectiveResolver : public StringFormat::IDirectiveResolver {
      protected:
        const Ipv4Route *route = nullptr;

      public:
        DirectiveResolver(const Ipv4Route *route) : route(route) {}

        virtual std::string resolveDirective(char directive) const override;
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

    // key is router ID, source module ID, destination module ID
    std::map<std::tuple<Ipv4Address, int, int>, const RouteVisualization *> routeVisualizations;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;
    virtual void preDelete(cComponent *root) override;

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
    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *obj, cObject *details) override;
};

} // namespace visualizer

} // namespace inet

#endif

