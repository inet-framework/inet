//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/common/ModuleAccess.h"
#include "OpenStreetMapCanvasVisualizer.h"

using namespace inet::osm;

namespace inet {

namespace visualizer {

Define_Module(OpenStreetMapCanvasVisualizer);

inline const char *nullToEmpty(const char *s)
{
    return s ? s : "";
}

inline bool isEmpty(const char *s)
{
    return !s || !s[0];
}

void OpenStreetMapCanvasVisualizer::initialize(int stage)
{
    SceneVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_PHYSICAL_ENVIRONMENT) {
        zIndex = par("zIndex");
        canvas = visualizationTargetModule->getCanvas();
        coordinateSystem = getModuleFromPar<IGeographicCoordinateSystem>(par("coordinateSystemModule"), this);
        canvasProjection = CanvasProjection::getCanvasProjection(canvas);

        cXMLElement *mapXml = par("mapFile").xmlValue();
        StreetMap map = StreetMap::from(mapXml);
        EV << "Loaded " << map.getNodes().size() << " nodes, " << map.getWays().size() << " ways\n";

        //TODO adjust coordinate system origin

        EV << "Drawing map\n";
        drawMap(map);

        cDisplayString& displayString = visualizationTargetModule->getDisplayString();
        if (displayString.containsTag("bgb")) {
            EV << "Display string sets background box size (bgb tag), respecting that\n";
        }
        else {
            EV << "Display string does not specify background box size (bgb tag), setting it to include map bounds\n";
            const osm::Bounds& bounds = map.getBounds();
            cFigure::Point bottomRight = toCanvas(map, bounds.minlat, bounds.maxlon);
            displayString.setTagArg("bgb", 0, bottomRight.x);
            displayString.setTagArg("bgb", 1, bottomRight.y);
        }
    }
}

cFigure::Point OpenStreetMapCanvasVisualizer::toCanvas(const StreetMap& map, double lat, double lon)
{
    Coord coord = coordinateSystem->computeSceneCoordinate(GeoCoord(deg(lat), deg(lon), m(0)));
    cFigure::Point p = canvasProjection->computeCanvasPoint(coord);
    return p;
}

void OpenStreetMapCanvasVisualizer::drawMap(const StreetMap& map)
{
    const cFigure::Color COLOR_HIGHWAY_PRIMARY = {255, 255, 120};
    const cFigure::Color COLOR_HIGHWAY_RESIDENTIAL = {240, 240, 240};
    const cFigure::Color COLOR_HIGHWAY_PATH = {128, 128, 128};

    cCanvas *canvas = getSystemModule()->getCanvas();
    canvas->setAnimationSpeed(1, this);

    for (const auto& way : map.getWays()) {
        std::vector<cFigure::Point> points;
        for (const auto& node : way->getNodes())
            points.push_back(toCanvas(map, node->getLat(), node->getLon()));
        bool isArea = way->getNodes().front() == way->getNodes().back();

        //TODO z-order (primary road on top of others)
        if (!isArea) {
            // road, street, footway, etc.
            cPolylineFigure *polyline = new cPolylineFigure();
            polyline->setPoints(points);
            polyline->setZoomLineWidth(true);

            polyline->setName(std::to_string(way->getId()).c_str());
            const char *name = way->getTag("name");
            if (name != nullptr)
                polyline->setTooltip(name);

            std::string highwayType = nullToEmpty(way->getTag("highway"));
            if (highwayType == "primary" || highwayType == "secondary" || highwayType == "tertiary" ||
                highwayType == "primary_link" || highwayType == "secondary_link" || highwayType == "tertiary_link") {
                polyline->setLineWidth(8);
                polyline->setLineColor(COLOR_HIGHWAY_PRIMARY);
                polyline->setCapStyle(cFigure::CAP_ROUND);
                polyline->setJoinStyle(cFigure::JOIN_ROUND);
            }
            else if (highwayType == "residential" || highwayType == "service") {
                polyline->setLineWidth(4);
                polyline->setLineColor(COLOR_HIGHWAY_RESIDENTIAL);
                polyline->setCapStyle(cFigure::CAP_ROUND);
                polyline->setJoinStyle(cFigure::JOIN_ROUND);
            }
            else if (highwayType != "") { // footpath or similar
                polyline->setLineStyle(cFigure::LINE_DOTTED);
                polyline->setLineColor(COLOR_HIGHWAY_PATH);
            }
            else { // administrative boundary or some other non-path thing
                delete polyline;
                polyline = nullptr;
            }

            if (polyline)
                canvas->addFigure(polyline);
        }
        else {
            // building, park, etc.
            cPolygonFigure *polygon = new cPolygonFigure();
            points.pop_back();
            polygon->setPoints(points);

            polygon->setName(std::to_string(way->getId()).c_str());
            const char *name = way->getTag("name");
            if (name != nullptr)
                polygon->setTooltip(name);

            polygon->setFilled(true);
            polygon->setFillOpacity(0.1);
            polygon->setLineOpacity(0.5);
            polygon->setLineColor(cFigure::GREY);
            if (!isEmpty(way->getTag("building")))
                polygon->setFillColor(cFigure::RED);
            else
                polygon->setFillColor(cFigure::GREEN);
            canvas->addFigure(polygon);
        }
    }
}

} // namespace visualizer

} // namespace inet

