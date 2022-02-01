//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_NETWORKNODECANVASVISUALIZER_H
#define __INET_NETWORKNODECANVASVISUALIZER_H

#include "inet/common/geometry/common/CanvasProjection.h"
#include "inet/visualizer/base/NetworkNodeVisualizerBase.h"
#include "inet/visualizer/canvas/scene/NetworkNodeCanvasVisualization.h"

namespace inet {

namespace visualizer {

class INET_API NetworkNodeCanvasVisualizer : public NetworkNodeVisualizerBase
{
  protected:
    const CanvasProjection *canvasProjection = nullptr;
    double zIndex = NaN;
    std::map<int, NetworkNodeCanvasVisualization *> networkNodeVisualizations;

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;

    virtual NetworkNodeCanvasVisualization *createNetworkNodeVisualization(cModule *networkNode) const override;
    virtual void addNetworkNodeVisualization(NetworkNodeVisualization *networkNodeVisualization) override;
    virtual void removeNetworkNodeVisualization(NetworkNodeVisualization *networkNodeVisualization) override;
    virtual void destroyNetworkNodeVisualization(NetworkNodeVisualization *networkNodeVisualization) override { delete networkNodeVisualization; }

  public:
    virtual ~NetworkNodeCanvasVisualizer();
    virtual NetworkNodeCanvasVisualization *findNetworkNodeVisualization(const cModule *networkNode) const override;
    virtual NetworkNodeCanvasVisualization *getNetworkNodeVisualization(const cModule *networkNode) const override;
};

} // namespace visualizer

} // namespace inet

#endif

