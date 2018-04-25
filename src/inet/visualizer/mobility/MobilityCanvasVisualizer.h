//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_MOBILITYCANVASVISUALIZER_H
#define __INET_MOBILITYCANVASVISUALIZER_H

#include "inet/common/figures/TrailFigure.h"
#include "inet/common/geometry/common/CanvasProjection.h"
#include "inet/visualizer/base/MobilityVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API MobilityCanvasVisualizer : public MobilityVisualizerBase
{
  protected:
    class INET_API MobilityCanvasVisualization : public MobilityVisualization {
      public:
        cOvalFigure *positionFigure = nullptr;
        cPieSliceFigure *orientationFigure = nullptr;
        cLineFigure *veloctiyFigure = nullptr;
        TrailFigure *trailFigure = nullptr;

      public:
        MobilityCanvasVisualization(cOvalFigure *positionFigure, cPieSliceFigure *orientationFigure, cLineFigure *veloctiyFigure, TrailFigure *trailFigure, IMobility *mobility);
    };

  protected:
    double zIndex = NaN;
    const CanvasProjection *canvasProjection = nullptr;
    std::map<const IMobility *, MobilityCanvasVisualization *> mobilityVisualizations;

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;

    virtual MobilityCanvasVisualization *getMobilityVisualization(const IMobility *mobility) const;
    virtual void setMobilityVisualization(const IMobility *mobility, MobilityCanvasVisualization *entry);
    virtual void removeMobilityVisualization(const IMobility *mobility);
    virtual MobilityCanvasVisualization* ensureMobilityVisualization(IMobility *mobility);
    virtual void extendMovementTrail(const IMobility *mobility, TrailFigure *trailFigure, cFigure::Point position) const;

  public:
    virtual ~MobilityCanvasVisualizer();

    virtual void receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details) override;
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_MOBILITYCANVASVISUALIZER_H

