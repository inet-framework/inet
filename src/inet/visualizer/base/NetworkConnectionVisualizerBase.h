//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_NETWORKCONNECTIONVISUALIZERBASE_H
#define __INET_NETWORKCONNECTIONVISUALIZERBASE_H

#include "inet/visualizer/base/VisualizerBase.h"
#include "inet/visualizer/util/NetworkNodeFilter.h"

namespace inet {

namespace visualizer {

class INET_API NetworkConnectionVisualizerBase : public VisualizerBase, public cListener
{
  protected:
    NetworkNodeFilter nodeFilter;
    cFigure::Color lineColor;
    cFigure::LineStyle lineStyle = cFigure::LINE_SOLID;
    double lineWidth = NaN;

  protected:
    virtual void initialize(int stage) override;
    virtual void handleParameterChange(const char *name) override;

    virtual void createNetworkConnectionVisualization(cModule *startNetworkNode, cModule *endNetworkNode) = 0;
};

} // namespace visualizer

} // namespace inet

#endif

