//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
        ObstacleLossCanvasVisualization(LabeledLineFigure *intersectionFigure, cLineFigure *faceNormalFigure1, cLineFigure *faceNormalFigure2);
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
    virtual void addObstacleLossVisualization(const ObstacleLossVisualization *obstacleLossVisualization) override;
    virtual void removeObstacleLossVisualization(const ObstacleLossVisualization *obstacleLossVisualization) override;
    virtual void setAlpha(const ObstacleLossVisualization *obstacleLossVisualization, double alpha) const override;
};

} // namespace visualizer

} // namespace inet

#endif

