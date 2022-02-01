//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PATHCANVASVISUALIZERBASE_H
#define __INET_PATHCANVASVISUALIZERBASE_H

#include "inet/common/figures/LabeledPolylineFigure.h"
#include "inet/common/geometry/common/CanvasProjection.h"
#include "inet/visualizer/base/PathVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API PathCanvasVisualizerBase : public PathVisualizerBase
{
  protected:
    class INET_API PathCanvasVisualization : public PathVisualization {
      public:
        LabeledPolylineFigure *figure = nullptr;

      public:
        PathCanvasVisualization(const char *label, const std::vector<int>& path, LabeledPolylineFigure *figure);
        virtual ~PathCanvasVisualization();
    };

  protected:
    double zIndex = NaN;
    const CanvasProjection *canvasProjection = nullptr;
    cGroupFigure *pathGroup = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;

    virtual const PathVisualization *createPathVisualization(const char *label, const std::vector<int>& path, cPacket *packet) const override;
    virtual void addPathVisualization(const PathVisualization *pathVisualization) override;
    virtual void removePathVisualization(const PathVisualization *pathVisualization) override;
    virtual void setAlpha(const PathVisualization *pathVisualization, double alpha) const override;
    virtual void refreshPathVisualization(const PathVisualization *pathVisualization, cPacket *packet) override;
};

} // namespace visualizer

} // namespace inet

#endif

