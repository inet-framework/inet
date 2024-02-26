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

    class INET_API MulticastRouteVisualization : public LineManager::ModuleLine {
      public:
        mutable int numRoutes = 1;
        const Ipv4MulticastRoute *route = nullptr;

      public:
        MulticastRouteVisualization(const Ipv4MulticastRoute *route, int nodeModuleId, int nextHopModuleId);
        virtual ~MulticastRouteVisualization() {}
    };

    class INET_API DirectiveResolver : public StringFormat::IDirectiveResolver {
      protected:
        const Ipv4Route *route = nullptr;

      public:
        DirectiveResolver(const Ipv4Route *route) : route(route) {}

        virtual std::string resolveDirective(char directive) const override;
    };

    class INET_API MulticastDirectiveResolver : public StringFormat::IDirectiveResolver {
      protected:
        const Ipv4MulticastRoute *route = nullptr;

      public:
        MulticastDirectiveResolver(const Ipv4MulticastRoute *route) : route(route) {}

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
    NetworkNodeFilter multicastSourceNodeFilter;
    cMatchExpression multicastSourceAddressFilter;
    cMatchExpression multicastGroupFilter;
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

    bool allRoutingTableVisualizationsAreInvalid = true;
    std::set<Ipv4RoutingTable *> invalidRoutingTableVisualizations;
    LineManager *lineManager = nullptr;

    // key is router ID, source module ID, destination module ID
    std::map<std::tuple<Ipv4Address, int, int>, const RouteVisualization *> routeVisualizations;

    // key is router ID, source module ID, destination module ID
    std::map<std::tuple<Ipv4Address, int, int>, const MulticastRouteVisualization *> multicastRouteVisualizations;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;
    virtual void preDelete(cComponent *root) override;
    virtual void refreshDisplay() const override;

    virtual void subscribe();
    virtual void unsubscribe();

    virtual const RouteVisualization *createRouteVisualization(Ipv4Route *route, cModule *node, cModule *nextHop) const = 0;
    virtual const RouteVisualization *getRouteVisualization(Ipv4Route *route, int nodeModuleId, int nextHopModuleId);
    virtual void addRouteVisualization(const RouteVisualization *routeVisualization);
    virtual void removeRouteVisualization(const RouteVisualization *routeVisualization);

    virtual const MulticastRouteVisualization *createMulticastRouteVisualization(Ipv4MulticastRoute *route, cModule *node, cModule *nextHop) const = 0;
    virtual const MulticastRouteVisualization *getMulticastRouteVisualization(Ipv4MulticastRoute *route, int nodeModuleId, int nextHopModuleId);
    virtual void addMulticastRouteVisualization(const MulticastRouteVisualization *routeVisualization);
    virtual void removeMulticastRouteVisualization(const MulticastRouteVisualization *routeVisualization);

    virtual std::vector<Ipv4Address> getDestinations();
    virtual std::vector<Ipv4Address> getMulticastSources();
    virtual std::vector<Ipv4Address> getMulticastGroups();

    virtual void addRouteVisualizations(cModule *node, Ipv4RoutingTable *routingTable);
    virtual void removeRouteVisualizations(cModule *node, Ipv4RoutingTable *routingTable);
    virtual void updateRouteVisualizations(cModule *node, Ipv4RoutingTable *routingTable);

    virtual void addAllRouteVisualizations();
    virtual void removeAllRouteVisualizations();
    virtual void updateAllRouteVisualizations();

    virtual void updateRouteVisualizations();

    virtual std::string getRouteVisualizationText(const Ipv4Route *route) const;
    virtual std::string getMulticastRouteVisualizationText(const Ipv4MulticastRoute *route) const;

    virtual void refreshRouteVisualization(const RouteVisualization *routeVisualization) const = 0;
    virtual void refreshMulticastRouteVisualization(const MulticastRouteVisualization *routeVisualization) const = 0;

  public:
    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *obj, cObject *details) override;
};

} // namespace visualizer

} // namespace inet

#endif

