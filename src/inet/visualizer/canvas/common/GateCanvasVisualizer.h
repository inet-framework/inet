//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
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

