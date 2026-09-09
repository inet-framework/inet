//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_ROUTINGTABLEVSGVISUALIZER_H
#define __INET_ROUTINGTABLEVSGVISUALIZER_H

#include <vsg/core/ref_ptr.h>
#include <vsg/nodes/Group.h>

#include "inet/visualizer/base/RoutingTableVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API RoutingTableVsgVisualizer : public RoutingTableVisualizerBase
{
  protected:
    class INET_API RouteVsgVisualization : public RouteVisualization {
      public:
        ::vsg::ref_ptr<::vsg::Node> node;

      public:
        RouteVsgVisualization(::vsg::ref_ptr<::vsg::Node> node, const Ipv4Route *route, int nodeModuleId, int nextHopModuleId);
    };

    class INET_API MulticastRouteVsgVisualization : public MulticastRouteVisualization {
      public:
        ::vsg::ref_ptr<::vsg::Node> node;

      public:
        MulticastRouteVsgVisualization(::vsg::ref_ptr<::vsg::Node> node, const Ipv4MulticastRoute *route, int nodeModuleId, int nextHopModuleId);
    };

  protected:
    virtual void initialize(int stage) override;

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
