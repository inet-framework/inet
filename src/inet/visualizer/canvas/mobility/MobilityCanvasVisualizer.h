//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
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
        cLineFigure *velocityFigure = nullptr;
        TrailFigure *trailFigure = nullptr;

      public:
        MobilityCanvasVisualization(cOvalFigure *positionFigure, cPieSliceFigure *orientationFigure, cLineFigure *velocityFigure, TrailFigure *trailFigure, IMobility *mobility);
        virtual ~MobilityCanvasVisualization();
    };

  protected:
    double zIndex = NaN;
    const CanvasProjection *canvasProjection = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;

    virtual MobilityVisualization *createMobilityVisualization(IMobility *mobility) override;
    virtual void addMobilityVisualization(const IMobility *mobility, MobilityVisualization *mobilityVisualization) override;
    virtual void removeMobilityVisualization(const MobilityVisualization *mobilityVisualization) override;
    virtual void extendMovementTrail(const IMobility *mobility, TrailFigure *trailFigure, cFigure::Point position) const;
};

} // namespace visualizer

} // namespace inet

#endif

