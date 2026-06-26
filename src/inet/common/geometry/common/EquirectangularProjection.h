//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_EQUIRECTANGULARPROJECTION_H
#define __INET_EQUIRECTANGULARPROJECTION_H

#include <vector>

#include "inet/common/INETDefs.h"
#include "inet/common/geometry/common/IMapProjection.h"

namespace inet {

/**
 * The equirectangular (plate-carree) map projection over a configurable latitude/longitude window.
 * This is the single owner of the equirectangular/geodesy logic: it implements the IMapProjection
 * first stage that CanvasProjection delegates to (ECEF <-> geographic mapping with antimeridian-aware
 * longitude wrapping, and the WGS84 direction Jacobian), and additionally provides the degree-space
 * window helpers used by the geo map scene/overlay visualizers (window-to-pixel mapping, the
 * antimeridian polygon tiling, and configuring the shared CanvasProjection).
 *
 * The window may span any sub-rectangle of the globe; a window whose eastern edge is at or west of its
 * western edge wraps across the antimeridian, and a window spanning the full 360 degrees of longitude
 * "wraps the globe" (geometry running off one side reappears on the other).
 */
class INET_API EquirectangularProjection : public IMapProjection
{
  protected:
    double mapWidth = 720;
    double mapHeight = 360;
    double minLatitude = -90, maxLatitude = 90; // latitude bounds of the visible window, degrees
    double minLongitude = -180; // western edge of the visible window, degrees
    double longitudeSpan = 360; // (maxLongitude - minLongitude) wrapped into (0, 360], degrees
    bool wrapsGlobe = true; // true when the window spans the full 360 deg of longitude
    double referenceLongitude = -180; // antimeridian seam; longitudes are wrapped into [ref, ref+360), degrees

  public:
    EquirectangularProjection() {}

    /** Configures the latitude/longitude window and the pixel size of the map. */
    void setWindow(double minLatitudeDeg, double maxLatitudeDeg, double minLongitudeDeg, double maxLongitudeDeg, double mapWidth, double mapHeight);

    double getMapWidth() const { return mapWidth; }
    double getMapHeight() const { return mapHeight; }
    double getLongitudeSpan() const { return longitudeSpan; }
    bool getWrapsGlobe() const { return wrapsGlobe; }
    double getMinLatitude() const { return minLatitude; }
    double getMaxLatitude() const { return maxLatitude; }
    double getMinLongitude() const { return minLongitude; }

    // IMapProjection: the first projection stage (ECEF <-> longitude/latitude/altitude with wrapping)
    // and the WGS84 direction Jacobian, delegated to from CanvasProjection.
    virtual Coord applyForward(const Coord& scenePoint) const override;
    virtual Coord applyInverse(const Coord& projectedPoint) const override;
    virtual cFigure::Point computeCanvasDirection(const Coord& point, const Coord& direction, double& depth,
            DirectionProjection directionProjection, const RotationMatrix& rotation, const cFigure::Point& scale) const override;

    /**
     * Projects a geographic point to map pixels within the configured window. When normalizeLongitude is
     * false the longitude is used as given (relative to the window's western edge, possibly outside one
     * span) so a sequence of points stays continuous across the window edge instead of wrapping.
     */
    cFigure::Point geoToCanvas(double latitudeDeg, double longitudeDeg, bool normalizeLongitude = true) const;

    /** Returns true when the geographic point falls inside the configured window. */
    bool isInsideWindow(double latitudeDeg, double longitudeDeg) const;

    /**
     * Sets up the shared canvas CanvasProjection so this equirectangular projection (and the optional
     * figure clipping) applies to every other canvas visualizer drawing scene coordinates: it attaches
     * this projection as the canvas first stage and derives the scale/translation reproducing
     * geoToCanvas() for the unwrapped window. This is the single place that derives the equirectangular
     * affine/clip setup.
     */
    void configureCanvasProjection(cCanvas *canvas, bool clipFigures = true) const;

    /**
     * Clips a polygon whose vertices are given in continuous (unwrapped) map pixels to the map rectangle,
     * tiling at +/-mapWidth when the window wraps the globe so the part running off one side reappears on
     * the other. Returns one polygon piece per tile copy that has a visible part; pieces with fewer than
     * 3 vertices are dropped. (The rectangle clipping uses CanvasProjection::clipPolygonToRect.)
     */
    std::vector<std::vector<cFigure::Point>> clipAndTilePolygon(const std::vector<cFigure::Point>& continuousPoints) const;

  private:
    // Wraps a window-relative longitude offset into [0, 360) degrees.
    static double normalizeRelativeLongitude(double relativeLongitudeDeg);
};

} // namespace inet

#endif
