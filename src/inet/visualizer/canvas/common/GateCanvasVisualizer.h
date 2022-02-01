//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_GATECANVASVISUALIZER_H
#define __INET_GATECANVASVISUALIZER_H

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/figures/GateFigure.h"
#include "inet/visualizer/base/GateVisualizerBase.h"
#include "inet/visualizer/canvas/scene/NetworkNodeCanvasVisualizer.h"

namespace inet {

namespace visualizer {

class INET_API GateCanvasVisualizer : public GateVisualizerBase
{
  protected:
    class INET_API GateCanvasVisualization : public GateVisualization {
      public:
        NetworkNodeCanvasVisualization *networkNodeVisualization = nullptr;
        GateFigure *figure = nullptr;

      public:
        GateCanvasVisualization(NetworkNodeCanvasVisualization *networkNodeVisualization, GateFigure *figure, queueing::IPacketGate *gate);
        virtual ~GateCanvasVisualization();
    };

  protected:
    // parameters
    double zIndex = NaN;
    ModuleRefByPar<NetworkNodeCanvasVisualizer> networkNodeVisualizer;

  protected:
    virtual void initialize(int stage) override;

    virtual GateVisualization *createGateVisualization(queueing::IPacketGate *gate) const override;
    virtual void addGateVisualization(const GateVisualization *gateVisualization) override;
    virtual void removeGateVisualization(const GateVisualization *gateVisualization) override;
    virtual void refreshGateVisualization(const GateVisualization *gateVisualization) const override;
};

} // namespace visualizer

} // namespace inet

#endif

