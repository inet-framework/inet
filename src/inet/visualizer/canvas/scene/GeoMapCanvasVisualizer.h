//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_GEOMAPCANVASVISUALIZER_H
#define __INET_GEOMAPCANVASVISUALIZER_H

#include "inet/visualizer/base/VisualizerBase.h"
#include "inet/common/geometry/common/EquirectangularProjection.h"

namespace inet {

namespace visualizer {

/**
 * Draws a 2D world map on the OMNeT++ canvas: a background map image (stretched to
 * the map area) overlaid with a latitude/longitude graticule, plus an optional
 * absolute UTC wall-clock in the lower-left corner. It also sets up the shared
 * canvas projection so that other canvas visualizers drawing scene coordinates
 * (node markers, movement trails, the ~GeoHorizonCanvasVisualizer footprints, ...)
 * line up with this map.
 *
 * The map image and the graticule are drawn together as one group at this
 * visualizer's z-index, with the graticule on top of the image. No per-node
 * markers are drawn; node visualization is left to the other visualizers.
 *
 * @see ~GeoHorizonCanvasVisualizer, ~MobilityCanvasVisualizer
 */
class INET_API GeoMapCanvasVisualizer : public VisualizerBase
{
  protected:
    /** @name Parameters */
    //@{
    bool displayMap = false;
    double zIndex = NaN;

    // map background and projection
    std::string mapImage;
    EquirectangularProjection projection; // the equirectangular map window (and the shared canvas projection it configures)
    bool adjustBackgroundBox = false;
    bool clipFigures = false; // clip other visualizers' figures to the [0..mapWidth] x [0..mapHeight] map area

    // graticule (latitude/longitude grid)
    bool displayGraticule = false;
    double graticuleLatSpacing = NaN;
    double graticuleLonSpacing = NaN;
    cFigure::Color graticuleLineColor;
    double graticuleLineWidth = NaN;
    //@}

    cGroupFigure *mapFigure = nullptr;
    cLabelFigure *clockFigure = nullptr; // wall-clock in the lower-left corner, null if no epoch is configured
    double clockBaseSeconds = 0; // absolute UTC seconds (since 1970-01-01) corresponding to simulation time 0

  protected:
    virtual void initialize(int stage) override;
    virtual void refreshDisplay() const override;

    virtual void createBackgroundFigure();
    // Shares this map's equirectangular projection (and optional figure clipping) with every other
    // canvas visualizer by configuring the canvas CanvasProjection via the EquirectangularProjection.
    // Done in INITSTAGE_LAST so it overrides the SceneCanvasVisualizer defaults.
    virtual void configureCanvasProjection();
    // Creates the lower-left wall-clock figure from the 'epoch' parameter (ISO-8601 UTC at
    // simulation time 0); does nothing when no epoch is configured.
    virtual void createClockFigure();
};

} // namespace visualizer

} // namespace inet

#endif
