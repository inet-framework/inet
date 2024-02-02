//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ROUTINGTABLECANVASVISUALIZER_H
#define __INET_ROUTINGTABLECANVASVISUALIZER_H

#include "inet/common/figures/LabeledLineFigure.h"
#include "inet/common/geometry/common/CanvasProjection.h"
#include "inet/visualizer/base/RoutingTableVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API RoutingTableCanvasVisualizer : public RoutingTableVisualizerBase
{
  protected:
    class INET_API RouteCanvasVisualization : public RouteVisualization {
      public:
        LabeledLineFigure *figure = nullptr;

      public:
        RouteCanvasVisualization(LabeledLineFigure *figure, const Ipv4Route *route, int nodeModuleId, int nextHopModuleId);
        virtual ~RouteCanvasVisualization();
    };

    class INET_API MulticastRouteCanvasVisualization : public MulticastRouteVisualization {
      public:
        LabeledLineFigure *figure = nullptr;

      public:
        MulticastRouteCanvasVisualization(LabeledLineFigure *figure, const Ipv4MulticastRoute *route, int nodeModuleId, int nextHopModuleId);
        virtual ~MulticastRouteCanvasVisualization();
    };

  protected:
    double zIndex = NaN;
    const CanvasProjection *canvasProjection = nullptr;
    cGroupFigure *routeGroup = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;

    virtual const RouteVisualization *createRouteVisualization(Ipv4Route *route, cModule *node, cModule *nextHop) const override;
    virtual void addRouteVisualization(const RouteVisualization *routeVisualization) override;
    virtual void removeRouteVisualization(const RouteVisualization *routeVisualization) override;
    virtual void refreshRouteVisualization(const RouteVisualization *routeVisualization) const override;

    virtual const MulticastRouteVisualization *createMulticastRouteVisualization(Ipv4MulticastRoute *route, cModule *node, cModule *nextHop) const override;
    virtual void addMulticastRouteVisualization(const MulticastRouteVisualization *routeVisualization) override;
    virtual void removeMulticastRouteVisualization(const MulticastRouteVisualization *routeVisualization) override;
    virtual void refreshMulticastRouteVisualization(const MulticastRouteVisualization *routeVisualization) const override;
};

} // namespace visualizer

} // namespace inet

#endif

