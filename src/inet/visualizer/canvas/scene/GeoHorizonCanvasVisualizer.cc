//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/canvas/scene/GeoHorizonCanvasVisualizer.h"

#include <cmath>

#include "inet/common/ModuleAccess.h"
#include "inet/common/geometry/common/Wgs84.h"

namespace inet {

namespace visualizer {

Define_Module(GeoHorizonCanvasVisualizer);

std::vector<GeoCoord> GeoHorizonCanvasVisualizer::visibilityRing(const GeoCoord& subSatellitePoint, double altitude, double elevationMaskRad, int sampleCount)
{
    std::vector<GeoCoord> ring;
    if (altitude <= 0)
        return ring;
    double radius = wgs84::MEAN_RADIUS;
    // central (Earth) angle from the sub-satellite point to the footprint edge
    double cosArg = radius / (radius + altitude) * std::cos(elevationMaskRad);
    if (cosArg >= 1)
        return ring; // altitude too low to produce a footprint at this elevation
    double delta = std::acos(cosArg) - elevationMaskRad; // angular radius on the sphere, radians
    double lat0 = subSatellitePoint.latitude.get<rad>();
    // Guard against the exact poles, where cos(lat0)=0 collapses the destination-longitude formula and
    // the footprint ring degenerates to a sliver; clamp slightly off the pole so it stays a proper cap.
    // NOTE: a true circumpolar cap (reaching the map's top/bottom edge across all longitudes) is not yet
    // drawn - the clamped ring is only a close approximation for near-polar sub-satellite points.
    const double maxLat = 89.9 * M_PI / 180.0;
    lat0 = std::max(-maxLat, std::min(maxLat, lat0));
    double lon0Deg = subSatellitePoint.longitude.get<deg>();
    for (int i = 0; i < sampleCount; i++) {
        double bearing = 2 * M_PI * i / sampleCount;
        double lat = std::asin(std::sin(lat0) * std::cos(delta) + std::cos(lat0) * std::sin(delta) * std::cos(bearing));
        double deltaLon = std::atan2(std::sin(bearing) * std::sin(delta) * std::cos(lat0), std::cos(delta) - std::sin(lat0) * std::sin(lat));
        double lonDeg = lon0Deg + deltaLon * 180.0 / M_PI;
        ring.push_back(GeoCoord(rad(lat), deg(lonDeg), m(0)));
    }
    return ring;
}

void GeoHorizonCanvasVisualizer::initialize(int stage)
{
    VisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        zIndex = par("zIndex");
        animationSpeed = par("animationSpeed");
        moduleFilter.setPattern(par("moduleFilter"));
        // the geographic (ECEF) coordinate system used to place footprints; given by our own parameter
        coordinateSystem = findModuleFromPar<IGeographicCoordinateSystem>(par("coordinateSystemModule"), this);
        if (coordinateSystem == nullptr)
            throw cRuntimeError("Geographic coordinate system module not found on path '%s' defined by par 'coordinateSystemModule'", par("coordinateSystemModule").stringValue());
        // the map window is taken from the referenced map visualizer, so the footprint lines up with it
        auto mapVisualizerModule = findModuleFromPar<cModule>(par("mapVisualizer"), this);
        if (mapVisualizerModule == nullptr)
            throw cRuntimeError("Map visualizer module not found on path '%s' defined by par 'mapVisualizer'", par("mapVisualizer").stringValue());
        double mapWidth = mapVisualizerModule->par("mapWidth");
        double mapHeight = mapVisualizerModule->par("mapHeight");
        double minLatitude = deg(mapVisualizerModule->par("minLatitude")).get<deg>();
        double maxLatitude = deg(mapVisualizerModule->par("maxLatitude")).get<deg>();
        double minLongitude = deg(mapVisualizerModule->par("minLongitude")).get<deg>();
        double maxLongitude = deg(mapVisualizerModule->par("maxLongitude")).get<deg>();
        projection.setWindow(minLatitude, maxLatitude, minLongitude, maxLongitude, mapWidth, mapHeight);

        displayVisibility = par("displayVisibility");
        elevationMaskRad = deg(par("elevationMask")).get<rad>();
        footprintSampleCount = par("footprintSampleCount");
        visibilityFillColor = cFigure::parseColor(par("visibilityFillColor"));
        visibilityLineColor = cFigure::parseColor(par("visibilityLineColor"));
        visibilityOpacity = par("visibilityOpacity");

        subscribe();
    }
}

void GeoHorizonCanvasVisualizer::preDelete(cComponent *root)
{
    if (hasGUI()) {
        unsubscribe();
        removeAllNodeVisualizations();
    }
}

void GeoHorizonCanvasVisualizer::subscribe()
{
    visualizationSubjectModule->subscribe(IMobility::mobilityStateChangedSignal, this);
    visualizationSubjectModule->subscribe(PRE_MODEL_CHANGE, this);
}

void GeoHorizonCanvasVisualizer::unsubscribe()
{
    auto subjectModule = findModuleFromPar<cModule>(par("visualizationSubjectModule"), this);
    if (subjectModule != nullptr) {
        subjectModule->unsubscribe(IMobility::mobilityStateChangedSignal, this);
        subjectModule->unsubscribe(PRE_MODEL_CHANGE, this);
    }
}

GeoHorizonCanvasVisualizer::NodeVisualization *GeoHorizonCanvasVisualizer::getNodeVisualization(const IMobility *mobility) const
{
    auto it = nodeVisualizations.find(mobility->getId());
    return it == nodeVisualizations.end() ? nullptr : it->second;
}

GeoHorizonCanvasVisualizer::NodeVisualization *GeoHorizonCanvasVisualizer::createNodeVisualization(IMobility *mobility)
{
    auto visualization = new NodeVisualization(mobility);
    auto canvas = visualizationTargetModule->getCanvas();
    if (displayVisibility) {
        visualization->visibilityFigure = new cGroupFigure("visibility");
        visualization->visibilityFigure->setTags((std::string("satellite_visibility ") + tags).c_str());
        visualization->visibilityFigure->setZIndex(zIndex);
        canvas->addFigure(visualization->visibilityFigure);
    }
    return visualization;
}

void GeoHorizonCanvasVisualizer::removeNodeVisualization(const NodeVisualization *visualization)
{
    auto canvas = visualizationTargetModule->getCanvas();
    if (visualization->visibilityFigure != nullptr) {
        canvas->removeFigure(visualization->visibilityFigure);
        delete visualization->visibilityFigure;
    }
    nodeVisualizations.erase(visualization->mobility->getId());
}

void GeoHorizonCanvasVisualizer::removeAllNodeVisualizations()
{
    std::vector<const NodeVisualization *> removed;
    for (auto it : nodeVisualizations)
        removed.push_back(it.second);
    for (auto visualization : removed) {
        removeNodeVisualization(visualization);
        delete visualization;
    }
}

void GeoHorizonCanvasVisualizer::refreshDisplay() const
{
    VisualizerBase::refreshDisplay();
    for (auto it : nodeVisualizations)
        refreshNode(it.second);
    visualizationTargetModule->getCanvas()->setAnimationSpeed(nodeVisualizations.empty() ? 0 : animationSpeed, this);
}

void GeoHorizonCanvasVisualizer::refreshNode(NodeVisualization *visualization) const
{
    auto mobility = visualization->mobility;
    Coord position = mobility->getCurrentPosition(); // geocentric ECEF (scene == ECEF)
    GeoCoord geo = coordinateSystem->computeGeographicCoordinate(position);
    double altitude = geo.altitude.get<m>();
    double referenceLongitude = geo.longitude.get<deg>();

    if (visualization->visibilityFigure != nullptr) {
        auto ring = visibilityRing(geo, altitude, elevationMaskRad, footprintSampleCount);
        rebuildPolygons(visualization->visibilityFigure, ring, referenceLongitude, visibilityFillColor, visibilityLineColor, visibilityOpacity);
    }
}

void GeoHorizonCanvasVisualizer::rebuildPolygons(cGroupFigure *group, const std::vector<GeoCoord>& ring, double referenceLongitudeDeg,
        const cFigure::Color& fillColor, const cFigure::Color& lineColor, double opacity) const
{
    if (ring.empty()) {
        group->setVisible(false);
        return;
    }
    group->setVisible(true);
    // unwrap the longitudes continuously around the reference so the ring does not jump at the antimeridian
    std::vector<cFigure::Point> continuous;
    continuous.reserve(ring.size());
    for (auto& geo : ring) {
        double longitude = geo.longitude.get<deg>();
        while (longitude - referenceLongitudeDeg > 180) longitude -= 360;
        while (longitude - referenceLongitudeDeg < -180) longitude += 360;
        continuous.push_back(projection.geoToCanvas(geo.latitude.get<deg>(), longitude, false));
    }
    auto pieces = projection.clipAndTilePolygon(continuous);
    // reuse the existing polygon child figures; only add or remove when the piece count changes
    while (group->getNumFigures() > (int)pieces.size()) {
        auto child = group->getFigure(group->getNumFigures() - 1);
        group->removeFigure(child);
        delete child;
    }
    while (group->getNumFigures() < (int)pieces.size()) {
        auto polygon = new cPolygonFigure("piece");
        polygon->setFilled(true);
        group->addFigure(polygon);
    }
    for (size_t i = 0; i < pieces.size(); i++) {
        auto polygon = check_and_cast<cPolygonFigure *>(group->getFigure((int)i));
        polygon->setPoints(pieces[i]);
        polygon->setFillColor(fillColor);
        polygon->setFillOpacity(opacity);
        polygon->setLineColor(lineColor);
        polygon->setLineOpacity(opacity);
    }
}

void GeoHorizonCanvasVisualizer::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signal));
    if (signal == IMobility::mobilityStateChangedSignal) {
        if (moduleFilter.matches(check_and_cast<cModule *>(source))) {
            auto mobility = dynamic_cast<IMobility *>(source);
            if (getNodeVisualization(mobility) == nullptr)
                nodeVisualizations[mobility->getId()] = createNodeVisualization(mobility);
        }
    }
    else if (signal == PRE_MODEL_CHANGE) {
        if (dynamic_cast<cPreModuleDeleteNotification *>(object))
            if (auto mobility = dynamic_cast<IMobility *>(source))
                if (auto visualization = getNodeVisualization(mobility)) {
                    removeNodeVisualization(visualization);
                    delete visualization;
                }
    }
    else
        throw cRuntimeError("Unknown signal");
}

} // namespace visualizer

} // namespace inet
