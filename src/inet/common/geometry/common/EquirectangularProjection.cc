//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/geometry/common/EquirectangularProjection.h"

#include <cmath>

#include "inet/common/geometry/common/CanvasProjection.h"
#include "inet/common/geometry/common/RotationMatrix.h"
#include "inet/common/geometry/common/Wgs84.h"

namespace inet {

double EquirectangularProjection::normalizeRelativeLongitude(double relativeLongitudeDeg)
{
    while (relativeLongitudeDeg < 0) relativeLongitudeDeg += 360;
    while (relativeLongitudeDeg >= 360) relativeLongitudeDeg -= 360;
    return relativeLongitudeDeg;
}

void EquirectangularProjection::setWindow(double minLatitudeDeg, double maxLatitudeDeg, double minLongitudeDeg, double maxLongitudeDeg, double mapWidth, double mapHeight)
{
    this->minLatitude = minLatitudeDeg;
    this->maxLatitude = maxLatitudeDeg;
    this->minLongitude = minLongitudeDeg;
    this->mapWidth = mapWidth;
    this->mapHeight = mapHeight;
    longitudeSpan = maxLongitudeDeg - minLongitudeDeg;
    while (longitudeSpan <= 0) // an eastern edge at or west of the western edge means the window wraps across the antimeridian
        longitudeSpan += 360;
    wrapsGlobe = std::fabs(longitudeSpan - 360) < 1e-6;
    // put the antimeridian seam opposite the window center so a shifted or antimeridian-crossing window
    // lines up with where projected points (node markers, trails, footprints) land
    referenceLongitude = minLongitude + longitudeSpan / 2 - 180;
}

Coord EquirectangularProjection::applyForward(const Coord& scenePoint) const
{
    // the scene coordinate is WGS84 ECEF; map it to (longitude, latitude, altitude)
    GeoCoord geographicCoordinate = wgs84::ecefToGeodetic(scenePoint);
    // wrap longitude into [referenceLongitude, referenceLongitude+360) so a shifted or antimeridian-crossing
    // map window lines up with the projected points (ecefToGeodetic returns longitude in [-180, 180), which
    // would otherwise miss the wrapped-around strip)
    double longitude = geographicCoordinate.longitude.get<deg>();
    longitude -= 360.0 * std::floor((longitude - referenceLongitude) / 360.0);
    return Coord(longitude, geographicCoordinate.latitude.get<deg>(), geographicCoordinate.altitude.get<m>());
}

Coord EquirectangularProjection::applyInverse(const Coord& projectedPoint) const
{
    // (longitude, latitude, altitude) back to the WGS84 ECEF scene coordinate
    return wgs84::geodeticToEcef(GeoCoord(deg(projectedPoint.y), deg(projectedPoint.x), m(projectedPoint.z)));
}

cFigure::Point EquirectangularProjection::computeCanvasDirection(const Coord& point, const Coord& direction, double& depth,
        DirectionProjection directionProjection, const RotationMatrix& rotation, const cFigure::Point& scale) const
{
    // decompose the direction in the local geodetic East-North-Up frame at the point
    GeoCoord geographicCoordinate = wgs84::ecefToGeodetic(point);
    Coord east = wgs84::enuVectorToEcef(geographicCoordinate, Coord(1, 0, 0));
    Coord north = wgs84::enuVectorToEcef(geographicCoordinate, Coord(0, 1, 0));
    Coord up = wgs84::enuVectorToEcef(geographicCoordinate, Coord(0, 0, 1));
    double eastComponent = direction * east, northComponent = direction * north;
    depth = direction * up; // out-of-plane component: positive points up / toward the viewer
    // on-screen directions of +1 deg longitude (pre-projection x) and +1 deg latitude (pre-projection y)
    Coord longitudeAxis = rotation.rotateVector(Coord(1, 0, 0));
    Coord latitudeAxis = rotation.rotateVector(Coord(0, 1, 0));
    cFigure::Point eastDir(longitudeAxis.x * scale.x, longitudeAxis.y * scale.y);
    cFigure::Point northDir(latitudeAxis.x * scale.x, latitudeAxis.y * scale.y);
    double northLength = northDir.getLength();
    if (northLength == 0)
        return cFigure::Point(0, 0);
    double eastLength = eastDir.getLength();
    // DIRECTION_ISOMETRIC: conformal + isometric. A single uniform scale (pixels per meter, from
    // the latitude axis) keeps the mapping isotropic; the vertical (up) component is dropped
    // (orthographic onto the horizontal plane), so 3D angles and lengths are preserved
    double pixelPerMeter = northLength * (180.0 / M_PI) / wgs84::SEMI_MAJOR_AXIS;
    cFigure::Point eastUnit = eastLength > 0 ? eastDir * (1 / eastLength) : cFigure::Point(0, 0);
    cFigure::Point northUnit = northDir * (1 / northLength);
    cFigure::Point isometric = (eastUnit * eastComponent + northUnit * northComponent) * pixelPerMeter;
    if (directionProjection == DIRECTION_ISOMETRIC)
        return isometric;
    // DIRECTION_PROJECTED: non-conformal + non-isometric. The exact local Jacobian of the
    // equirectangular projection (WGS84 ellipsoid + altitude), so it matches how positions move
    // on the map via computeCanvasPoint(): a meter east changes longitude by 1/((N+h)cos(lat)) and
    // a meter north changes latitude by 1/(M+h), where N and M are the prime-vertical and meridian
    // radii of curvature and h is the altitude. This reproduces the map's longitude stretching
    // toward the poles (and the ground-track foreshortening of a high-altitude node's velocity).
    double latitudeRad = geographicCoordinate.latitude.get<deg>() * M_PI / 180.0;
    double sinLat = std::sin(latitudeRad), cosLat = std::cos(latitudeRad);
    double altitude = geographicCoordinate.altitude.get<m>();
    double t = 1.0 - wgs84::ECCENTRICITY_SQUARED * sinLat * sinLat;
    double primeVerticalRadius = wgs84::SEMI_MAJOR_AXIS / std::sqrt(t); // N
    double meridianRadius = wgs84::SEMI_MAJOR_AXIS * (1.0 - wgs84::ECCENTRICITY_SQUARED) / (t * std::sqrt(t)); // M
    double radToDeg = 180.0 / M_PI;
    double deltaLongitude = cosLat != 0 ? eastComponent / ((primeVerticalRadius + altitude) * cosLat) * radToDeg : 0;
    double deltaLatitude = northComponent / (meridianRadius + altitude) * radToDeg;
    cFigure::Point projected = eastDir * deltaLongitude + northDir * deltaLatitude;
    if (directionProjection == DIRECTION_PROJECTED)
        return projected;
    // DIRECTION_TRUE_LENGTH: non-conformal + isometric. The map heading (sheared direction) but
    // rescaled to the isometric (true metric) length, so it doesn't blow up toward the poles
    double projectedLength = projected.getLength();
    if (projectedLength == 0)
        return cFigure::Point(0, 0);
    return projected * (isometric.getLength() / projectedLength);
}

cFigure::Point EquirectangularProjection::geoToCanvas(double latitudeDeg, double longitudeDeg, bool normalizeLongitude) const
{
    double relativeLongitude = longitudeDeg - minLongitude; // degrees east of the window's western edge
    if (normalizeLongitude)
        relativeLongitude = normalizeRelativeLongitude(relativeLongitude);
    double x = relativeLongitude / longitudeSpan * mapWidth;
    double y = mapHeight * (maxLatitude - latitudeDeg) / (maxLatitude - minLatitude);
    return cFigure::Point(x, y);
}

bool EquirectangularProjection::isInsideWindow(double latitudeDeg, double longitudeDeg) const
{
    if (latitudeDeg < minLatitude || latitudeDeg > maxLatitude)
        return false;
    double relativeLongitude = normalizeRelativeLongitude(longitudeDeg - minLongitude);
    return relativeLongitude <= longitudeSpan + 1e-9; // always true when the window spans the full globe
}

void EquirectangularProjection::configureCanvasProjection(cCanvas *canvas, bool clipFigures) const
{
    auto canvasProjection = CanvasProjection::getCanvasProjection(canvas);
    // attach this equirectangular projection as the canvas first stage (a copy, owned by the canvas projection)
    canvasProjection->setMapProjection(new EquirectangularProjection(*this));
    // identity view rotation so the projected (longitude, latitude) maps straight onto the map axes
    canvasProjection->setRotation(RotationMatrix());
    // these scale/translation values reproduce geoToCanvas() for the (unwrapped) window:
    //   x = (lon - minLongitude) / longitudeSpan * mapWidth
    //   y = (maxLatitude - lat) / (maxLatitude - minLatitude) * mapHeight
    double latitudeSpan = maxLatitude - minLatitude;
    canvasProjection->setScale(cFigure::Point(mapWidth / longitudeSpan, -mapHeight / latitudeSpan));
    canvasProjection->setTranslation(cFigure::Point(-minLongitude * mapWidth / longitudeSpan, maxLatitude * mapHeight / latitudeSpan));
    // clip other visualizers' figures to the map area (a no-op on the helpers unless a clip rect is set)
    if (clipFigures)
        canvasProjection->setClipRect(cFigure::Rectangle(0, 0, mapWidth, mapHeight));
    else
        canvasProjection->clearClipRect();
}

std::vector<std::vector<cFigure::Point>> EquirectangularProjection::clipAndTilePolygon(const std::vector<cFigure::Point>& continuousPoints) const
{
    std::vector<std::vector<cFigure::Point>> pieces;
    int shiftRange = wrapsGlobe ? 1 : 0;
    for (int shift = -shiftRange; shift <= shiftRange; shift++) {
        std::vector<cFigure::Point> shifted;
        shifted.reserve(continuousPoints.size());
        for (auto& p : continuousPoints)
            shifted.push_back(cFigure::Point(p.x + shift * mapWidth, p.y));
        auto clipped = CanvasProjection::clipPolygonToRect(shifted, 0, 0, mapWidth, mapHeight);
        if (clipped.size() >= 3)
            pieces.push_back(clipped);
    }
    return pieces;
}

} // namespace inet
