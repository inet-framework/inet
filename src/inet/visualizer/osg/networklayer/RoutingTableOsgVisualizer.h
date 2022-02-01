//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ROUTINGTABLEOSGVISUALIZER_H
#define __INET_ROUTINGTABLEOSGVISUALIZER_H

#include <osg/ref_ptr>

#include "inet/visualizer/base/RoutingTableVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API RoutingTableOsgVisualizer : public RoutingTableVisualizerBase
{
  protected:
    class INET_API RouteOsgVisualization : public RouteVisualization {
      public:
        osg::ref_ptr<osg::Node> node;

      public:
        RouteOsgVisualization(osg::Node *node, const Ipv4Route *route, int nodeModuleId, int nextHopModuleId);
    };

  protected:
    virtual void initialize(int stage) override;

    virtual const RouteVisualization *createRouteVisualization(Ipv4Route *route, cModule *node, cModule *nextHop) const override;
    virtual void addRouteVisualization(const RouteVisualization *routeVisualization) override;
    virtual void removeRouteVisualization(const RouteVisualization *routeVisualization) override;
    virtual void refreshRouteVisualization(const RouteVisualization *routeVisualization) const override;
};

} // namespace visualizer

} // namespace inet

#endif

