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
};

} // namespace visualizer

} // namespace inet

#endif

