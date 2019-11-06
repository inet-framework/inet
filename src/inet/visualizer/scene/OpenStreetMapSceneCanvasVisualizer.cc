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
#include "inet/visualizer/scene/OpenStreetMapSceneCanvasVisualizer.h"

using namespace inet::osm;

namespace inet {

namespace visualizer {

Define_Module(OpenStreetMapSceneCanvasVisualizer);

inline const char *nullToEmpty(const char *s)
{
    return s ? s : "";
}

inline bool isEmpty(const char *s)
{
    return !s || !s[0];
}

void OpenStreetMapSceneCanvasVisualizer::initialize(int stage)
{
    SceneVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_PHYSICAL_ENVIRONMENT) {
        auto canvas = visualizationTargetModule->getCanvas();
        coordinateSystem = getModuleFromPar<IGeographicCoordinateSystem>(par("coordinateSystemModule"), this);
        canvasProjection = CanvasProjection::getCanvasProjection(canvas);

        cXMLElement *mapXml = par("mapFile").xmlValue();
        OpenStreetMap map = OpenStreetMap::from(mapXml);
        EV << "Loaded " << map.getNodes().size() << " nodes, " << map.getWays().size() << " ways\n";
        auto mapGroupFigure = createMapFigure(map);
        mapGroupFigure->setZIndex(par("zIndex"));
        visualizationTargetModule->getCanvas()->addFigure(mapGroupFigure);

        cDisplayString& displayString = visualizationTargetModule->getDisplayString();
        if (par("adjustBackgroundBox").boolValue()) {
            const osm::Bounds& bounds = map.getBounds();
            cFigure::Point bottomRight = toCanvas(map, bounds.minlat, bounds.maxlon);
            displayString.setTagArg("bgb", 0, bottomRight.x);
            displayString.setTagArg("bgb", 1, bottomRight.y);
        }
    }
}

cFigure::Point OpenStreetMapSceneCanvasVisualizer::toCanvas(const OpenStreetMap& map, double lat, double lon)
{
    Coord coord = coordinateSystem->computeSceneCoordinate(GeoCoord(deg(lat), deg(lon), m(0)));
    return canvasProjection->computeCanvasPoint(coord);
}

cGroupFigure *OpenStreetMapSceneCanvasVisualizer::createMapFigure(const OpenStreetMap& map)
{
    const cFigure::Color COLOR_HIGHWAY_PRIMARY = {255, 255, 120};
    const cFigure::Color COLOR_HIGHWAY_RESIDENTIAL = {240, 240, 240};
    const cFigure::Color COLOR_HIGHWAY_PATH = {128, 128, 128};

    auto buildings = new cGroupFigure("buildings");
    buildings->setTags("buildings");
    auto primaryStreets = new cGroupFigure("primaryStreets");
    primaryStreets->setTags("primary_streets");
    auto residentialStreets = new cGroupFigure("residentialStreets");
    residentialStreets->setTags("residential_streets");
    auto pathways = new cGroupFigure("pathways");
    pathways->setTags("pathways");

    auto mapFigure = new cGroupFigure("openStreetMap");
    mapFigure->setTags("street_map");
    mapFigure->addFigure(buildings);
    mapFigure->addFigure(pathways);
    mapFigure->addFigure(residentialStreets);
    mapFigure->addFigure(primaryStreets);

    for (const auto& way : map.getWays()) {
        std::vector<cFigure::Point> points;
        for (const auto& node : way->getNodes())
            points.push_back(toCanvas(map, node->getLat(), node->getLon()));
        bool isArea = way->getNodes().front() == way->getNodes().back();

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
                primaryStreets->addFigure(polyline);
            }
            else if (highwayType == "residential" || highwayType == "service") {
                polyline->setLineWidth(4);
                polyline->setLineColor(COLOR_HIGHWAY_RESIDENTIAL);
                polyline->setCapStyle(cFigure::CAP_ROUND);
                polyline->setJoinStyle(cFigure::JOIN_ROUND);
                residentialStreets->addFigure(polyline);
            }
            else if (highwayType != "") { // footpath or similar
                polyline->setLineStyle(cFigure::LINE_DOTTED);
                polyline->setLineColor(COLOR_HIGHWAY_PATH);
                pathways->addFigure(polyline);
            }
            else { // administrative boundary or some other non-path thing
                delete polyline;
                polyline = nullptr;
            }
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
            buildings->addFigure(polygon);
        }
    }
    return mapFigure;
}

} // namespace visualizer

} // namespace inet

