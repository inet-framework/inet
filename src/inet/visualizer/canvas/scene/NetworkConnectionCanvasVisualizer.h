//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_NETWORKCONNECTIONCANVASVISUALIZER_H
#define __INET_NETWORKCONNECTIONCANVASVISUALIZER_H

#include <vector>

#include "inet/common/geometry/common/CanvasProjection.h"
#include "inet/visualizer/base/NetworkConnectionVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API NetworkConnectionCanvasVisualizer : public NetworkConnectionVisualizerBase
{
  protected:
    double zIndex = NaN;
    const CanvasProjection *canvasProjection = nullptr;

    struct ConnectionVisualization {
        cLineFigure *figure = nullptr;
        const cModule *startNetworkNode = nullptr;
        const cModule *endNetworkNode = nullptr;
    };
    std::vector<ConnectionVisualization> connectionVisualizations;

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;

    void createNetworkConnectionVisualization(cModule *startNetworkNode, cModule *endNetworkNode) override;
};

} // namespace visualizer

} // namespace inet

#endif

