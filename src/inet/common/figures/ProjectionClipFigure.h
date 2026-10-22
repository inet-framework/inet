//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_PROJECTIONCLIPFIGURE_H
#define __INET_PROJECTIONCLIPFIGURE_H

#include "inet/common/geometry/common/CanvasProjection.h"

namespace inet {

/**
 * Groups figures that must stay inside the clip rectangle of a CanvasProjection,
 * e.g. inside the area of a limited (regional) geographic map, see
 * GeoMapCanvasVisualizer. It has no visual appearance.
 *
 * While the projection has a clip rectangle, the figure clips its children to it
 * (see cAbstractShapeFigure::setClipChildren()). While the projection has no clip
 * rectangle (e.g. in scenes without a geographic map), the figure does not clip.
 * Otherwise it behaves like a cGroupFigure; in particular, clipping does not affect
 * the stacking order of the children.
 *
 * The figure follows the clip rectangle in refreshDisplay(), which the GUI calls
 * before rendering the canvas. (The clip rectangle is only set up in the last
 * initialization stage, i.e. after the visualizers have created their figures.)
 */
class INET_API ProjectionClipFigure : public cRectangleFigure
{
  protected:
    const CanvasProjection *canvasProjection;

  public:
    ProjectionClipFigure(const char *name, const CanvasProjection *canvasProjection);

    virtual void refreshDisplay() override;
};

} // namespace inet

#endif

