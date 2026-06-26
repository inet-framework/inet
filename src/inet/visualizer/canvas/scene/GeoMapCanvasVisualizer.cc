//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/canvas/scene/GeoMapCanvasVisualizer.h"

#include <cmath>
#include <cstdio>

namespace inet {

namespace visualizer {

Define_Module(GeoMapCanvasVisualizer);

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

void GeoMapCanvasVisualizer::initialize(int stage)
{
    VisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        displayMap = par("displayMap");
        zIndex = par("zIndex");
        // map background and projection
        mapImage = par("mapImage").stringValue();
        double mapWidth = par("mapWidth");
        double mapHeight = par("mapHeight");
        double minLatitude = deg(par("minLatitude")).get<deg>();
        double maxLatitude = deg(par("maxLatitude")).get<deg>();
        if (maxLatitude <= minLatitude)
            throw cRuntimeError("maxLatitude (%g deg) must be greater than minLatitude (%g deg)", maxLatitude, minLatitude);
        double minLongitude = deg(par("minLongitude")).get<deg>();
        double maxLongitude = deg(par("maxLongitude")).get<deg>();
        projection.setWindow(minLatitude, maxLatitude, minLongitude, maxLongitude, mapWidth, mapHeight);
        adjustBackgroundBox = par("adjustBackgroundBox");
        clipFigures = par("clipFigures");
        // graticule
        displayGraticule = par("displayGraticule");
        graticuleLatSpacing = deg(par("graticuleLatSpacing")).get<deg>();
        graticuleLonSpacing = deg(par("graticuleLonSpacing")).get<deg>();
        graticuleLineColor = cFigure::parseColor(par("graticuleLineColor"));
        graticuleLineWidth = par("graticuleLineWidth");
        if (displayMap) {
            createBackgroundFigure();
            createClockFigure();
        }
    }
    else if (stage == INITSTAGE_LAST)
        configureCanvasProjection();
}

void GeoMapCanvasVisualizer::configureCanvasProjection()
{
    // share this map's equirectangular projection (and the optional figure clipping) with every other
    // canvas visualizer drawing scene coordinates; done in INITSTAGE_LAST so it wins over SceneCanvasVisualizer
    if (displayMap)
        projection.configureCanvasProjection(visualizationTargetModule->getCanvas(), clipFigures);
}

void GeoMapCanvasVisualizer::createClockFigure()
{
    const char *epoch = par("epoch");
    if (!*epoch)
        return; // no absolute epoch configured, so no wall-clock
    int year, month, day, hour, minute;
    double second;
    if (sscanf(epoch, "%d-%d-%dT%d:%d:%lf", &year, &month, &day, &hour, &minute, &second) != 6)
        throw cRuntimeError("Cannot parse epoch '%s', expected ISO-8601 e.g. 2026-06-02T00:00:00Z", epoch);
    clockBaseSeconds = daysFromCivil(year, month, day) * 86400.0 + hour * 3600 + minute * 60 + second;
    clockFigure = new cLabelFigure("wallClock");
    clockFigure->setTags((std::string("wall_clock ") + tags).c_str());
    clockFigure->setColor(cFigure::parseColor(par("clockColor")));
    clockFigure->setHalo(true); // keep the text legible over the map imagery
    clockFigure->setAnchor(cFigure::ANCHOR_SW);
    clockFigure->setPosition(cFigure::Point(4, projection.getMapHeight() - 4)); // lower-left corner of the map
    clockFigure->setZIndex(zIndex);
    visualizationTargetModule->getCanvas()->addFigure(clockFigure);
}

void GeoMapCanvasVisualizer::createBackgroundFigure()
{
    auto canvas = visualizationTargetModule->getCanvas();
    double mapWidth = projection.getMapWidth(), mapHeight = projection.getMapHeight();
    if (adjustBackgroundBox) {
        cDisplayString& displayString = visualizationTargetModule->getDisplayString();
        displayString.setTagArg("bgb", 0, mapWidth);
        displayString.setTagArg("bgb", 1, mapHeight);
    }
    // the map image and the graticule are drawn together in one group at this visualizer's
    // zIndex; the graticule is added last so it renders on top of the map image (at the same
    // z-index) instead of floating above the rest of the scene
    mapFigure = new cGroupFigure("geographicMap");
    mapFigure->setTags((std::string("geographic_map ") + tags).c_str());
    mapFigure->setZIndex(zIndex);
    if (!mapImage.empty()) {
        auto imageFigure = new cImageFigure("mapImage");
        imageFigure->setImageName(mapImage.c_str());
        imageFigure->setAnchor(cFigure::ANCHOR_NW);
        imageFigure->setPosition(cFigure::Point(0, 0));
        imageFigure->setWidth(mapWidth); // stretch the image to the map area
        imageFigure->setHeight(mapHeight);
        mapFigure->addFigure(imageFigure);
    }
    if (displayGraticule) {
        auto graticule = new cGroupFigure("graticule");
        graticule->setTags((std::string("graticule ") + tags).c_str());
        double minLongitude = projection.getMinLongitude(), longitudeSpan = projection.getLongitudeSpan();
        double minLatitude = projection.getMinLatitude(), maxLatitude = projection.getMaxLatitude();
        // meridians and parallels are vertical and horizontal lines; draw only those inside the window
        for (double lon = std::ceil(minLongitude / graticuleLonSpacing) * graticuleLonSpacing;
             lon - minLongitude <= longitudeSpan + 1e-6; lon += graticuleLonSpacing)
        {
            double x = projection.geoToCanvas(0, lon, false).x;
            auto line = new cLineFigure("meridian");
            line->setStart(cFigure::Point(x, 0));
            line->setEnd(cFigure::Point(x, mapHeight));
            line->setLineColor(graticuleLineColor);
            line->setLineWidth(graticuleLineWidth);
            graticule->addFigure(line);
        }
        for (double lat = std::ceil(minLatitude / graticuleLatSpacing) * graticuleLatSpacing;
             lat <= maxLatitude + 1e-6; lat += graticuleLatSpacing)
        {
            double y = projection.geoToCanvas(lat, minLongitude).y;
            auto line = new cLineFigure("parallel");
            line->setStart(cFigure::Point(0, y));
            line->setEnd(cFigure::Point(mapWidth, y));
            line->setLineColor(graticuleLineColor);
            line->setLineWidth(graticuleLineWidth);
            graticule->addFigure(line);
        }
        mapFigure->addFigure(graticule);
    }
    canvas->addFigure(mapFigure);
}

void GeoMapCanvasVisualizer::refreshDisplay() const
{
    VisualizerBase::refreshDisplay();
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
}

} // namespace visualizer

} // namespace inet
