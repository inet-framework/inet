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

#ifndef __INET_TRACINGOBSTACLELOSSCANVASVISUALIZER_H
#define __INET_TRACINGOBSTACLELOSSCANVASVISUALIZER_H

#include "inet/common/figures/LabeledLineFigure.h"
#include "inet/common/geometry/common/CanvasProjection.h"
#include "inet/visualizer/base/TracingObstacleLossVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API TracingObstacleLossCanvasVisualizer : public TracingObstacleLossVisualizerBase
{
  protected:
    class INET_API ObstacleLossCanvasVisualization : public ObstacleLossVisualization {
      public:
        LabeledLineFigure *intersectionFigure = nullptr;
        cLineFigure *faceNormalFigure1 = nullptr;
        cLineFigure *faceNormalFigure2 = nullptr;

      public:
        ObstacleLossCanvasVisualization(LabeledLineFigure* intersectionFigure, cLineFigure* faceNormalFigure1, cLineFigure* faceNormalFigure2);
        virtual ~ObstacleLossCanvasVisualization();
    };

  protected:
    /** @name Parameters */
    //@{
    double zIndex = NaN;
    //@}

    /** @name Graphics */
    //@{
    /**
     * The 2D projection used on the canvas.
     */
    const CanvasProjection *canvasProjection = nullptr;
    /**
     * The layer figure that contains the figures representing the obstacle losses.
     */
    cGroupFigure *obstacleLossLayer = nullptr;
    //@}

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;

    virtual const ObstacleLossVisualization *createObstacleLossVisualization(const physicallayer::ITracingObstacleLoss::ObstaclePenetratedEvent *obstaclePenetratedEvent) const override;
    virtual void addObstacleLossVisualization(const ObstacleLossVisualization* obstacleLossVisualization) override;
    virtual void removeObstacleLossVisualization(const ObstacleLossVisualization* obstacleLossVisualization) override;
    virtual void setAlpha(const ObstacleLossVisualization *obstacleLossVisualization, double alpha) const override;

  public:
    virtual ~TracingObstacleLossCanvasVisualizer();
};

} // namespace visualizer

} // namespace inet

#endif // ifndef __INET_TRACINGOBSTACLELOSSCANVASVISUALIZER_H

