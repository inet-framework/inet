//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_TRANSPORTCONNECTIONCANVASVISUALIZER_H
#define __INET_TRANSPORTCONNECTIONCANVASVISUALIZER_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/figures/LabeledIconFigure.h"
#include "inet/common/geometry/common/CanvasProjection.h"
#include "inet/visualizer/base/TransportConnectionVisualizerBase.h"
#include "inet/visualizer/canvas/scene/NetworkNodeCanvasVisualizer.h"

namespace inet {

namespace visualizer {

class INET_API TransportConnectionCanvasVisualizer : public TransportConnectionVisualizerBase
{
  protected:
    class INET_API TransportConnectionCanvasVisualization : public TransportConnectionVisualization {
      public:
        LabeledIconFigure *sourceFigure = nullptr;
        LabeledIconFigure *destinationFigure = nullptr;

      public:
        TransportConnectionCanvasVisualization(LabeledIconFigure *sourceFigure, LabeledIconFigure *destinationFigure, int sourceModuleId, int destinationModuleId, int count);
    };

  protected:
    double zIndex = NaN;
    const CanvasProjection *canvasProjection = nullptr;
    ModuleRefByPar<NetworkNodeCanvasVisualizer> networkNodeVisualizer;

  protected:
    virtual void initialize(int stage) override;

    virtual LabeledIconFigure *createConnectionEndFigure(tcp::TcpConnection *connectionVisualization) const;
    virtual const TransportConnectionVisualization *createConnectionVisualization(cModule *source, cModule *destination, tcp::TcpConnection *tcpConnection) const override;
    virtual void addConnectionVisualization(const TransportConnectionVisualization *connectionVisualization) override;
    virtual void removeConnectionVisualization(const TransportConnectionVisualization *connectionVisualization) override;
    virtual void setConnectionLabelsVisible(bool visible);
};

} // namespace visualizer

} // namespace inet

#endif

