//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_CANVASPROJECTION_H
#define __INET_CANVASPROJECTION_H

#include <memory>
#include <vector>

#include "inet/common/geometry/common/IMapProjection.h"
#include "inet/common/geometry/common/RotationMatrix.h"

namespace inet {

class INET_API CanvasProjection
{
  public:
    // Re-exported here so consumers can keep referring to CanvasProjection::DirectionProjection; the
    // actual direction-projection logic lives in the map projection (see IMapProjection).
    using DirectionProjection = IMapProjection::DirectionProjection;
    static constexpr DirectionProjection DIRECTION_PROJECTED = IMapProjection::DIRECTION_PROJECTED;
    static constexpr DirectionProjection DIRECTION_TRUE_LENGTH = IMapProjection::DIRECTION_TRUE_LENGTH;
    static constexpr DirectionProjection DIRECTION_ISOMETRIC = IMapProjection::DIRECTION_ISOMETRIC;

  protected:
    // Optional first projection stage applied to the scene coordinate before the affine transform
    // (rotation/scale/translation). Null means the identity (the historical no-projection behavior);
    // a geographic map visualizer attaches an EquirectangularProjection. CanvasProjection delegates all
    // map-specific math to it and contains no map-projection knowledge itself.
    std::unique_ptr<IMapProjection> mapProjection;
    RotationMatrix rotation;
    cFigure::Point scale;
    cFigure::Point translation;

    // Optional clip rectangle in canvas (post-projection) pixel space. When clipping is enabled
    // (a scene visualizer that shows a limited map area sets it, e.g. GeoMapCanvasVisualizer with a
    // regional window), visualizers can use the helpers below to keep their figures inside the map.
    // Disabled by default, so non-geographic and full-window scenes are unaffected.
    bool clipEnabled = false;
    cFigure::Rectangle clipRect;

  public:
    CanvasProjection() : scale(cFigure::Point(1, 1)) {}
    CanvasProjection(RotationMatrix rotation, cFigure::Point translation);
    CanvasProjection(const CanvasProjection&) = delete; // per-canvas singleton; owns a unique_ptr
    CanvasProjection& operator=(const CanvasProjection&) = delete;
    CanvasProjection(CanvasProjection&&) = default;
    CanvasProjection& operator=(CanvasProjection&&) = default;
    virtual ~CanvasProjection();

    // The optional first projection stage. setMapProjection takes ownership of the given projection;
    // clearMapProjection restores the identity first stage.
    const IMapProjection *getMapProjection() const { return mapProjection.get(); }
    void setMapProjection(IMapProjection *projection) { mapProjection.reset(projection); }
    void clearMapProjection() { mapProjection.reset(); }

    const RotationMatrix& getRotation() const { return rotation; }
    void setRotation(const RotationMatrix& rotation) { this->rotation = rotation; }

    const cFigure::Point& getScale() const { return scale; }
    void setScale(const cFigure::Point& scale) { this->scale = scale; }

    const cFigure::Point& getTranslation() const { return translation; }
    void setTranslation(const cFigure::Point& translation) { this->translation = translation; }

    // Pass applyMapProjection = false to skip the optional map-projection first stage and run the affine
    // view transform only. For callers that pass synthetic scene coordinates (axis basis vectors, a
    // heat-map grid origin) which must not be reinterpreted as geographic positions when a map projection
    // is attached to this shared projection.
    cFigure::Point computeCanvasPoint(const Coord& point, bool applyMapProjection = true) const;
    cFigure::Point computeCanvasPoint(const Coord& point, double& depth, bool applyMapProjection = true) const;
    Coord computeCanvasPointInverse(const cFigure::Point& point, double depth) const;

    // Projects a direction (tangent) vector anchored at the given scene point to a canvas-space vector.
    // With no map projection this is the linear part of the affine transform (directionProjection has no
    // effect); with a map projection the computation is delegated to it (see IMapProjection). The depth
    // overload also returns the out-of-plane (toward/away the viewer) component, positive when the
    // direction points toward the viewer (e.g. upward/away from Earth on the geographic map).
    cFigure::Point computeCanvasDirection(const Coord& point, const Coord& direction, DirectionProjection directionProjection = DIRECTION_ISOMETRIC) const;
    cFigure::Point computeCanvasDirection(const Coord& point, const Coord& direction, double& depth, DirectionProjection directionProjection = DIRECTION_ISOMETRIC) const;

    // Clip rectangle (canvas pixel space). When set, the helpers below limit figures to this rect.
    bool hasClipRect() const { return clipEnabled; }
    const cFigure::Rectangle& getClipRect() const { return clipRect; }
    void setClipRect(const cFigure::Rectangle& rect) { clipRect = rect; clipEnabled = true; }
    void clearClipRect() { clipEnabled = false; }

    // Figure-clipping helpers. All are no-ops (return true / leave geometry unchanged) when no clip
    // rect is set, so callers can use them unconditionally.
    // - isPointInsideClip: is the point inside the clip rect?
    // - isRectVisibleInClip: does the given figure bounding box overlap the clip rect at all
    //   (i.e. "any part inside" - used to hide non-clippable figures only when fully outside)?
    // - clipLine: clips the segment to the clip rect; returns false if it is fully outside, else
    //   updates p0/p1 to the visible portion.
    // - clipPolygon: returns the polygon clipped to the clip rect (empty if fully outside).
    // - clipPolyline: clips an open polyline to the clip rect, re-joining the visible parts into one
    //   connected polyline (returns the input unchanged when no clip rect is set).
    bool isPointInsideClip(const cFigure::Point& point) const;
    bool isRectVisibleInClip(const cFigure::Rectangle& bounds) const;
    bool clipLine(cFigure::Point& p0, cFigure::Point& p1) const;
    std::vector<cFigure::Point> clipPolygon(const std::vector<cFigure::Point>& polygon) const;
    std::vector<cFigure::Point> clipPolyline(const std::vector<cFigure::Point>& polyline) const;

    // Stateless clip algorithms against an arbitrary rectangle (also used by EquirectangularProjection).
    static std::vector<cFigure::Point> clipPolygonToRect(std::vector<cFigure::Point> polygon, double xmin, double ymin, double xmax, double ymax);
    static bool clipSegmentToRect(cFigure::Point& p0, cFigure::Point& p1, double xmin, double ymin, double xmax, double ymax);

    static CanvasProjection *getCanvasProjection(const cCanvas *canvas);
};

} // namespace inet

#endif

