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
        cGroupFigure *orientationFigure = nullptr; // 3D orientation cone indicator: base ellipse + two side lines
        cOvalFigure *orientationBaseFigure = nullptr; // (rotated) base disc; filled when the cone points away (child of orientationFigure)
        cLineFigure *orientationLineFigure[2] = {nullptr, nullptr}; // the two projected cone-side lines (children of orientationFigure)
        cLineFigure *velocityFigure = nullptr;
        TrailFigure *trailFigure = nullptr;
        // last true (unclipped) trail position; trail segments are clipped for display, so the chain
        // is continued from this stored position rather than from the last (clipped) figure's end
        cFigure::Point lastTrailPosition;
        bool hasLastTrailPosition = false;

      public:
        MobilityCanvasVisualization(cOvalFigure *positionFigure, cGroupFigure *orientationFigure, cLineFigure *velocityFigure, TrailFigure *trailFigure, IMobility *mobility);
        virtual ~MobilityCanvasVisualization();
    };

  protected:
    double zIndex = NaN;
    const CanvasProjection *canvasProjection = nullptr;
    // how the velocity arrow / orientation cone follow the (equirectangular) map distortion; parsed in initialize()
    CanvasProjection::DirectionProjection velocityProjection = CanvasProjection::DIRECTION_PROJECTED;
    CanvasProjection::DirectionProjection orientationProjection = CanvasProjection::DIRECTION_PROJECTED;

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;

    virtual MobilityVisualization *createMobilityVisualization(IMobility *mobility) override;
    virtual void addMobilityVisualization(const IMobility *mobility, MobilityVisualization *mobilityVisualization) override;
    virtual void removeMobilityVisualization(const MobilityVisualization *mobilityVisualization) override;
    virtual void extendMovementTrail(const IMobility *mobility, MobilityCanvasVisualization *visualization, cFigure::Point position) const;
};

} // namespace visualizer

} // namespace inet

#endif

