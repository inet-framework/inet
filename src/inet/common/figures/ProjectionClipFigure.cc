//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/figures/ProjectionClipFigure.h"

namespace inet {

ProjectionClipFigure::ProjectionClipFigure(const char *name, const CanvasProjection *canvasProjection) :
    cRectangleFigure(name), canvasProjection(canvasProjection)
{
    setOutlined(false);
    setFilled(false);
    refreshDisplay();
}

void ProjectionClipFigure::refreshDisplay()
{
    // the setters do nothing if the value is unchanged, so this is cheap
    bool hasClipRect = canvasProjection->hasClipRect();
    if (hasClipRect)
        setBounds(canvasProjection->getClipRect());
    setClipChildren(hasClipRect);
}

} // namespace inet

