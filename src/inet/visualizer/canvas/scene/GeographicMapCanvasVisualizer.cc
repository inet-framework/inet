//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/canvas/scene/GeographicMapCanvasVisualizer.h"

#include <algorithm>
#include <cstdio>

#include "inet/common/ModuleAccess.h"

namespace inet {

namespace visualizer {

Define_Module(GeographicMapCanvasVisualizer);

// WGS84 semi-major axis (equatorial radius), in meters
static const double EARTH_RADIUS = 6378137.0;
// latitude limit of the Web Mercator projection (where it would reach +/-infinity)
static const double MERCATOR_MAX_LATITUDE = 85.05112878;

// Days since 1970-01-01 for a proleptic-Gregorian date (Howard Hinnant's algorithm, portable).
static long daysFromCivil(int y, int m, int d)
{
    y -= m <= 2;
    long era = (y >= 0 ? y : y - 399) / 400;
    int yoe = (int)(y - era * 400);
    int doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    int doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + doe - 719468;
}

// Inverse of daysFromCivil: civil date from days since 1970-01-01.
static void civilFromDays(long z, int& y, int& m, int& d)
{
    z += 719468;
    long era = (z >= 0 ? z : z - 146096) / 146097;
    int doe = (int)(z - era * 146097);
    int yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    y = yoe + (int)(era * 400);
    int doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    int mp = (5 * doy + 2) / 153;
    d = doy - (153 * mp + 2) / 5 + 1;
    m = mp + (mp < 10 ? 3 : -9);
    y += (m <= 2);
}

// Clips a polygon against one half-plane edge of the map rectangle (Sutherland-Hodgman).
// edge: 0 = x>=bound (left), 1 = x<=bound (right), 2 = y>=bound (top), 3 = y<=bound (bottom).
static std::vector<cFigure::Point> clipAgainstEdge(const std::vector<cFigure::Point>& in, int edge, double bound)
{
    std::vector<cFigure::Point> out;
    int n = (int)in.size();
    if (n == 0)
        return out;
    auto inside = [&](const cFigure::Point& p) -> bool {
        switch (edge) {
            case 0: return p.x >= bound;
            case 1: return p.x <= bound;
            case 2: return p.y >= bound;
            default: return p.y <= bound;
        }
    };
    auto intersect = [&](const cFigure::Point& a, const cFigure::Point& b) -> cFigure::Point {
        double t = (edge <= 1) ? (bound - a.x) / (b.x - a.x) : (bound - a.y) / (b.y - a.y);
        return cFigure::Point(a.x + t * (b.x - a.x), a.y + t * (b.y - a.y));
    };
    for (int i = 0; i < n; i++) {
        const cFigure::Point& cur = in[i];
        const cFigure::Point& prev = in[(i + n - 1) % n];
        bool curIn = inside(cur), prevIn = inside(prev);
        if (curIn) {
            if (!prevIn)
                out.push_back(intersect(prev, cur));
            out.push_back(cur);
        }
        else if (prevIn)
            out.push_back(intersect(prev, cur));
    }
    return out;
}

// Clips a polygon to the map rectangle [xmin, xmax] x [ymin, ymax].
static std::vector<cFigure::Point> clipPolygonToRect(std::vector<cFigure::Point> polygon, double xmin, double ymin, double xmax, double ymax)
{
    polygon = clipAgainstEdge(polygon, 0, xmin);
    polygon = clipAgainstEdge(polygon, 1, xmax);
    polygon = clipAgainstEdge(polygon, 2, ymin);
    polygon = clipAgainstEdge(polygon, 3, ymax);
    return polygon;
}

GeographicMapCanvasVisualizer::MapNodeCanvasVisualization::~MapNodeCanvasVisualization()
{
    delete markerFigure;
    delete labelFigure;
    delete satelliteNameLabelFigure;
    delete groundTrackFigure;
    delete footprintFigure;
}

void GeographicMapCanvasVisualizer::initialize(int stage)
{
    GeographicMapVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        createBackgroundFigure();
        createClockFigure();
    }
}

void GeographicMapCanvasVisualizer::createClockFigure()
{
    const char *controllerPath = par("satelliteController");
    if (!*controllerPath)
        return;
    auto module = findModuleFromPar<cModule>(par("satelliteController"), this);
    if (module == nullptr || !module->hasPar("epoch"))
        return;
    const char *epoch = module->par("epoch").stringValue();
    if (!*epoch)
        return; // no absolute epoch configured (e.g. the controller falls back to per-TLE epochs)
    int year, month, day, hour, minute;
    double second;
    if (sscanf(epoch, "%d-%d-%dT%d:%d:%lf", &year, &month, &day, &hour, &minute, &second) != 6)
        throw cRuntimeError("Cannot parse epoch '%s' from module '%s', expected ISO-8601 e.g. 2026-06-02T00:00:00Z", epoch, module->getFullPath().c_str());
    clockBaseSeconds = daysFromCivil(year, month, day) * 86400.0 + hour * 3600 + minute * 60 + second;
    clockFigure = new cLabelFigure("wallClock");
    clockFigure->setTags((std::string("wall_clock ") + tags).c_str());
    clockFigure->setColor(cFigure::parseColor(par("clockColor")));
    clockFigure->setHalo(true); // keep the text legible over the map imagery
    clockFigure->setAnchor(cFigure::ANCHOR_SW);
    clockFigure->setPosition(cFigure::Point(4, mapHeight - 4)); // lower-left corner of the map
    clockFigure->setZIndex(zIndex);
    visualizationTargetModule->getCanvas()->addFigure(clockFigure);
}

void GeographicMapCanvasVisualizer::createBackgroundFigure()
{
    auto canvas = visualizationTargetModule->getCanvas();
    if (adjustBackgroundBox) {
        cDisplayString& displayString = visualizationTargetModule->getDisplayString();
        displayString.setTagArg("bgb", 0, mapWidth);
        displayString.setTagArg("bgb", 1, mapHeight);
        if (!mapImage.empty()) {
            displayString.setTagArg("bgi", 0, mapImage.c_str());
            displayString.setTagArg("bgi", 1, "stretch");
        }
    }
    if (displayGraticule) {
        mapFigure = new cGroupFigure("graticule");
        mapFigure->setTags((std::string("graticule ") + tags).c_str());
        mapFigure->setZIndex(zIndex);
        double minLat = mercator ? -MERCATOR_MAX_LATITUDE : -90;
        double maxLat = mercator ? MERCATOR_MAX_LATITUDE : 90;
        for (double lon = -180; lon <= 180 + 1e-6; lon += graticuleLonSpacing) {
            auto line = new cLineFigure("meridian");
            line->setStart(geoToCanvas(maxLat, lon));
            line->setEnd(geoToCanvas(minLat, lon));
            line->setLineColor(graticuleLineColor);
            line->setLineWidth(graticuleLineWidth);
            mapFigure->addFigure(line);
        }
        for (double lat = -90; lat <= 90 + 1e-6; lat += graticuleLatSpacing) {
            if (lat < minLat || lat > maxLat)
                continue;
            auto line = new cLineFigure("parallel");
            line->setStart(geoToCanvas(lat, -180));
            line->setEnd(geoToCanvas(lat, 180));
            line->setLineColor(graticuleLineColor);
            line->setLineWidth(graticuleLineWidth);
            mapFigure->addFigure(line);
        }
        canvas->addFigure(mapFigure);
    }
}

cFigure::Point GeographicMapCanvasVisualizer::geoToCanvas(double latitudeDeg, double longitudeDeg, bool normalizeLongitude) const
{
    double relativeLongitude = longitudeDeg + longitudeOffset;
    if (normalizeLongitude) {
        while (relativeLongitude > 180) relativeLongitude -= 360;
        while (relativeLongitude < -180) relativeLongitude += 360;
    }
    double x = (relativeLongitude + 180.0) / 360.0 * mapWidth;
    double y;
    if (mercator) {
        double lat = std::max(-MERCATOR_MAX_LATITUDE, std::min(MERCATOR_MAX_LATITUDE, latitudeDeg)) * M_PI / 180.0;
        double mercatorY = std::log(std::tan(M_PI / 4 + lat / 2)); // in [-pi, pi] at the latitude limits
        y = mapHeight / 2 - mapHeight / (2 * M_PI) * mercatorY;
    }
    else
        y = (90.0 - latitudeDeg) / 180.0 * mapHeight;
    return cFigure::Point(x, y);
}

void GeographicMapCanvasVisualizer::refreshFootprint(MapNodeCanvasVisualization *visualization, double latitudeDeg, double longitudeDeg, double altitude) const
{
    auto group = visualization->footprintFigure;
    // discard the previous frame's pieces
    while (group->getNumFigures() > 0) {
        auto child = group->getFigure(0);
        group->removeFigure(child);
        delete child;
    }
    if (altitude <= 0) {
        group->setVisible(false);
        return;
    }
    double epsilon = minElevationAngle * M_PI / 180.0;
    // central (Earth) angle from the sub-satellite point to the footprint edge
    double cosArg = EARTH_RADIUS / (EARTH_RADIUS + altitude) * std::cos(epsilon);
    if (cosArg >= 1) {
        group->setVisible(false); // altitude too low to produce a footprint at this elevation
        return;
    }
    group->setVisible(true);
    double delta = std::acos(cosArg) - epsilon; // angular radius on the sphere, in radians
    double deltaDeg = delta * 180.0 / M_PI;
    double lat0 = latitudeDeg * M_PI / 180.0;
    const int sampleCount = 72;

    // a footprint that reaches over a pole cannot be drawn as a simple closed loop: build a
    // cap that is closed across the top (north) or bottom (south) edge of the map instead
    bool enclosesNorthPole = (90.0 - latitudeDeg) < deltaDeg;
    bool enclosesSouthPole = (90.0 + latitudeDeg) < deltaDeg;
    if (enclosesNorthPole || enclosesSouthPole) {
        std::vector<cFigure::Point> boundary;
        for (int i = 0; i < sampleCount; i++) {
            double bearing = 2 * M_PI * i / sampleCount;
            double lat = std::asin(std::sin(lat0) * std::cos(delta) + std::cos(lat0) * std::sin(delta) * std::cos(bearing));
            double deltaLon = std::atan2(std::sin(bearing) * std::sin(delta) * std::cos(lat0), std::cos(delta) - std::sin(lat0) * std::sin(lat));
            double lon = longitudeDeg + deltaLon * 180.0 / M_PI;
            boundary.push_back(geoToCanvas(lat * 180.0 / M_PI, lon, true)); // wrap into [0, mapWidth]
        }
        std::sort(boundary.begin(), boundary.end(), [](const cFigure::Point& a, const cFigure::Point& b) { return a.x < b.x; });
        double edgeY = enclosesNorthPole ? 0.0 : mapHeight;
        std::vector<cFigure::Point> cap;
        cap.push_back(cFigure::Point(0, edgeY));
        for (auto& p : boundary)
            cap.push_back(p);
        cap.push_back(cFigure::Point(mapWidth, edgeY));
        addFootprintPolygon(group, clipPolygonToRect(cap, 0, 0, mapWidth, mapHeight));
        return;
    }

    // general case: sample the small circle and project with continuous (unwrapped) longitude
    // around the center, then draw the polygon tiled at -mapWidth/0/+mapWidth and clipped to
    // the map, so the part that runs off one side reappears on the other
    std::vector<cFigure::Point> continuous;
    for (int i = 0; i < sampleCount; i++) {
        double bearing = 2 * M_PI * i / sampleCount;
        double lat = std::asin(std::sin(lat0) * std::cos(delta) + std::cos(lat0) * std::sin(delta) * std::cos(bearing));
        double deltaLon = std::atan2(std::sin(bearing) * std::sin(delta) * std::cos(lat0), std::cos(delta) - std::sin(lat0) * std::sin(lat));
        double lon = longitudeDeg + deltaLon * 180.0 / M_PI;
        continuous.push_back(geoToCanvas(lat * 180.0 / M_PI, lon, false));
    }
    for (int shift = -1; shift <= 1; shift++) {
        std::vector<cFigure::Point> shifted;
        shifted.reserve(continuous.size());
        for (auto& p : continuous)
            shifted.push_back(cFigure::Point(p.x + shift * mapWidth, p.y));
        addFootprintPolygon(group, clipPolygonToRect(shifted, 0, 0, mapWidth, mapHeight));
    }
}

void GeographicMapCanvasVisualizer::addFootprintPolygon(cGroupFigure *group, const std::vector<cFigure::Point>& points) const
{
    if (points.size() < 3)
        return;
    auto polygon = new cPolygonFigure("footprint");
    polygon->setTags((std::string("coverage_footprint ") + tags).c_str());
    polygon->setTooltip("This area represents the ground coverage footprint of the satellite");
    polygon->setPoints(points);
    polygon->setFilled(true);
    polygon->setLineColor(footprintLineColor);
    polygon->setLineOpacity(footprintOpacity);
    polygon->setFillColor(footprintFillColor);
    polygon->setFillOpacity(footprintOpacity);
    polygon->setZIndex(zIndex);
    group->addFigure(polygon);
}

void GeographicMapCanvasVisualizer::refreshDisplay() const
{
    GeographicMapVisualizerBase::refreshDisplay();
    if (clockFigure != nullptr) {
        double total = clockBaseSeconds + simTime().dbl(); // absolute UTC seconds
        long days = (long)std::floor(total / 86400.0);
        double remainder = total - (double)days * 86400.0;
        int year, month, day;
        civilFromDays(days, year, month, day);
        int hour = (int)(remainder / 3600);
        remainder -= hour * 3600;
        int minute = (int)(remainder / 60);
        int seconds = (int)(remainder - minute * 60);
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d UTC", year, month, day, hour, minute, seconds);
        clockFigure->setText(buffer);
    }
    for (auto it : mapNodeVisualizations) {
        auto visualization = static_cast<MapNodeCanvasVisualization *>(it.second);
        auto mobility = visualization->mobility;
        GeoCoord geo = coordinateSystem->computeGeographicCoordinate(mobility->getCurrentPosition());
        double latitude = geo.latitude.get<deg>();
        double longitude = geo.longitude.get<deg>();
        double altitude = geo.altitude.get<m>();
        cFigure::Point point = geoToCanvas(latitude, longitude);

        // marker
        if (auto imageFigure = dynamic_cast<cImageFigure *>(visualization->markerFigure))
            imageFigure->setPosition(point);
        else if (auto ovalFigure = dynamic_cast<cOvalFigure *>(visualization->markerFigure))
            ovalFigure->setBounds(cFigure::Rectangle(point.x - markerRadius, point.y - markerRadius, 2 * markerRadius, 2 * markerRadius));

        // labels (node name, and the TLE satellite name below it when present)
        if (visualization->labelFigure != nullptr) {
            cFigure::Point labelPosition(point.x + markerRadius + 2, point.y);
            visualization->labelFigure->setPosition(labelPosition);
            if (visualization->satelliteNameLabelFigure != nullptr)
                visualization->satelliteNameLabelFigure->setPosition(labelPosition);
        }

        // coverage footprint (a small circle on the Earth, correctly projected, wrapped and clipped to the map)
        if (visualization->footprintFigure != nullptr)
            refreshFootprint(visualization, latitude, longitude, altitude);

        // ground track: when the node crosses the map seam its projected x jumps by ~mapWidth
        // (a continuously moving node never jumps that far otherwise, regardless of the
        // longitudeOffset or projection). In that case draw two segments that wrap around the
        // seam - one to the edge it left, one from the opposite edge - so the track reappears
        // on the other side instead of breaking.
        if (visualization->groundTrackFigure != nullptr) {
            auto module = const_cast<cModule *>(check_and_cast<const cModule *>(mobility));
            if (!visualization->hasLastPoint) {
                visualization->lastPoint = point;
                visualization->hasLastPoint = true;
            }
            else {
                cFigure::Point last = visualization->lastPoint;
                double dx = point.x - last.x;
                if (std::fabs(dx) <= mapWidth / 2) {
                    double dy = point.y - last.y;
                    if (dx * dx + dy * dy > 1) {
                        addGroundTrackSegment(visualization, last, point, module);
                        visualization->lastPoint = point;
                    }
                }
                else if (dx < 0) {
                    // moved east: left the right edge (x=mapWidth), re-entered at the left edge (x=0)
                    double t = (mapWidth - last.x) / ((point.x + mapWidth) - last.x);
                    double crossingY = last.y + t * (point.y - last.y);
                    addGroundTrackSegment(visualization, last, cFigure::Point(mapWidth, crossingY), module);
                    addGroundTrackSegment(visualization, cFigure::Point(0, crossingY), point, module);
                    visualization->lastPoint = point;
                }
                else {
                    // moved west: left the left edge (x=0), re-entered at the right edge (x=mapWidth)
                    double t = last.x / (last.x - (point.x - mapWidth));
                    double crossingY = last.y + t * (point.y - last.y);
                    addGroundTrackSegment(visualization, last, cFigure::Point(0, crossingY), module);
                    addGroundTrackSegment(visualization, cFigure::Point(mapWidth, crossingY), point, module);
                    visualization->lastPoint = point;
                }
            }
        }
    }
    visualizationTargetModule->getCanvas()->setAnimationSpeed(mapNodeVisualizations.empty() ? 0 : animationSpeed, this);
}

GeographicMapVisualizerBase::MapNodeVisualization *GeographicMapCanvasVisualizer::createMapNodeVisualization(IMobility *mobility)
{
    auto module = const_cast<cModule *>(check_and_cast<const cModule *>(mobility));
    auto visualization = new MapNodeCanvasVisualization(mobility);

    if (!markerImage.empty()) {
        auto imageFigure = new cImageFigure("marker");
        imageFigure->setImageName(markerImage.c_str());
        imageFigure->setAnchor(cFigure::ANCHOR_CENTER);
        visualization->markerFigure = imageFigure;
    }
    else {
        auto ovalFigure = new cOvalFigure("marker");
        ovalFigure->setFilled(true);
        ovalFigure->setLineColor(markerColorSet.getColor(module->getId()));
        ovalFigure->setFillColor(markerColorSet.getColor(module->getId()));
        visualization->markerFigure = ovalFigure;
    }
    visualization->markerFigure->setTags((std::string("node_marker ") + tags).c_str());
    visualization->markerFigure->setTooltip(getNodeName(mobility));
    visualization->markerFigure->setZIndex(zIndex);

    if (displayLabels) {
        std::string satelliteName = getSatelliteName(mobility);
        visualization->labelFigure = new cLabelFigure("label");
        visualization->labelFigure->setTags((std::string("node_label ") + tags).c_str());
        visualization->labelFigure->setText(getNodeName(mobility));
        visualization->labelFigure->setColor(labelColor);
        // when a TLE name is shown too, anchor the node name above the marker line and the
        // TLE name below it; otherwise keep the single label vertically centered on the marker
        visualization->labelFigure->setAnchor(satelliteName.empty() ? cFigure::ANCHOR_W : cFigure::ANCHOR_SW);
        visualization->labelFigure->setZIndex(zIndex);
        if (!satelliteName.empty()) {
            visualization->satelliteNameLabelFigure = new cLabelFigure("satelliteName");
            visualization->satelliteNameLabelFigure->setTags((std::string("node_label ") + tags).c_str());
            visualization->satelliteNameLabelFigure->setText(satelliteName.c_str());
            visualization->satelliteNameLabelFigure->setColor(labelColor);
            visualization->satelliteNameLabelFigure->setAnchor(cFigure::ANCHOR_NW);
            visualization->satelliteNameLabelFigure->setZIndex(zIndex);
        }
    }

    if (displayGroundTracks) {
        visualization->groundTrackFigure = new TrailFigure(groundTrackLength, true, "ground track");
        visualization->groundTrackFigure->setTags((std::string("ground_track recent_history ") + tags).c_str());
        visualization->groundTrackFigure->setZIndex(zIndex);
    }

    if (displayFootprints) {
        // a container for the footprint polygon pieces, rebuilt every refresh (the footprint
        // may be split into several pieces where it wraps around the seam)
        visualization->footprintFigure = new cGroupFigure("footprint");
        visualization->footprintFigure->setTags((std::string("coverage_footprint ") + tags).c_str());
        visualization->footprintFigure->setVisible(false);
        visualization->footprintFigure->setZIndex(zIndex);
    }
    return visualization;
}

void GeographicMapCanvasVisualizer::addMapNodeVisualization(const IMobility *mobility, MapNodeVisualization *visualization)
{
    GeographicMapVisualizerBase::addMapNodeVisualization(mobility, visualization);
    auto canvasVisualization = static_cast<MapNodeCanvasVisualization *>(visualization);
    auto canvas = visualizationTargetModule->getCanvas();
    // add in back-to-front order: footprint, ground track, marker, label
    if (canvasVisualization->footprintFigure != nullptr)
        canvas->addFigure(canvasVisualization->footprintFigure);
    if (canvasVisualization->groundTrackFigure != nullptr)
        canvas->addFigure(canvasVisualization->groundTrackFigure);
    canvas->addFigure(canvasVisualization->markerFigure);
    if (canvasVisualization->labelFigure != nullptr)
        canvas->addFigure(canvasVisualization->labelFigure);
    if (canvasVisualization->satelliteNameLabelFigure != nullptr)
        canvas->addFigure(canvasVisualization->satelliteNameLabelFigure);
}

void GeographicMapCanvasVisualizer::removeMapNodeVisualization(const MapNodeVisualization *visualization)
{
    auto canvasVisualization = static_cast<const MapNodeCanvasVisualization *>(visualization);
    auto canvas = visualizationTargetModule->getCanvas();
    if (canvasVisualization->footprintFigure != nullptr)
        canvas->removeFigure(canvasVisualization->footprintFigure);
    if (canvasVisualization->groundTrackFigure != nullptr)
        canvas->removeFigure(canvasVisualization->groundTrackFigure);
    canvas->removeFigure(canvasVisualization->markerFigure);
    if (canvasVisualization->labelFigure != nullptr)
        canvas->removeFigure(canvasVisualization->labelFigure);
    if (canvasVisualization->satelliteNameLabelFigure != nullptr)
        canvas->removeFigure(canvasVisualization->satelliteNameLabelFigure);
    GeographicMapVisualizerBase::removeMapNodeVisualization(visualization);
}

void GeographicMapCanvasVisualizer::addGroundTrackSegment(MapNodeCanvasVisualization *visualization, const cFigure::Point& from, const cFigure::Point& to, cModule *module) const
{
    auto segment = new cLineFigure("groundTrackSegment");
    segment->setTags((std::string("ground_track recent_history ") + tags).c_str());
    segment->setStart(from);
    segment->setEnd(to);
    segment->setLineColor(groundTrackLineColorSet.getColor(module->getId()));
    segment->setLineStyle(groundTrackLineStyle);
    segment->setLineWidth(groundTrackLineWidth);
    segment->setZIndex(zIndex);
    visualization->groundTrackFigure->addFigure(segment);
}

const char *GeographicMapCanvasVisualizer::getNodeName(const IMobility *mobility) const
{
    auto module = check_and_cast<const cModule *>(mobility);
    auto networkNode = module->getParentModule();
    return networkNode != nullptr ? networkNode->getFullName() : module->getFullName();
}

std::string GeographicMapCanvasVisualizer::getSatelliteName(const IMobility *mobility) const
{
    auto module = const_cast<cModule *>(check_and_cast<const cModule *>(mobility));
    if (module->hasPar("satelliteName")) {
        const char *name = module->par("satelliteName").stringValue();
        if (*name)
            return name;
    }
    return "";
}

} // namespace visualizer

} // namespace inet
