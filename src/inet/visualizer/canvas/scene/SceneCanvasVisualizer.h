//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_SCENECANVASVISUALIZER_H
#define __INET_SCENECANVASVISUALIZER_H

#include "inet/common/geometry/common/CanvasProjection.h"
#include "inet/visualizer/base/SceneVisualizerBase.h"

namespace inet {

namespace visualizer {

class INET_API SceneCanvasVisualizer : public SceneVisualizerBase
{
  protected:
    double zIndex = NaN;
    CanvasProjection *canvasProjection = nullptr;
    cGroupFigure *axisLayer = nullptr;

  protected:
    virtual void initialize(int stage) override;
    virtual void initializeAxis(double axisLength);
    virtual void handleParameterChange(const char *name) override;
    virtual void refreshAxis(double axisLength);

    virtual RotationMatrix parseViewAngle(const char *viewAngle, bool& invertY);
    virtual cFigure::Point parse2D(const char *text, bool invertY = false);
    virtual void displayDescription(const char *descriptionFigurePath);
};

} // namespace visualizer

} // namespace inet

#endif

