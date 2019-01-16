//
// Copyright (C) 2014 OpenSim Ltd.
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

#include "inet/environment/ground/OsgEarthGround.h"

#if defined(WITH_OSGEARTH) && defined(WITH_VISUALIZERS)
//TODO the visualizers needed only for get the map from SceneOsgEarthVisualizer

#include "inet/common/ModuleAccess.h"
#include "inet/common/geometry/common/GeographicCoordinateSystem.h"
#include "inet/visualizer/scene/SceneOsgEarthVisualizer.h"


namespace inet {

using namespace visualizer;

namespace physicalenvironment {

Define_Module(OsgEarthGround);

void OsgEarthGround::initialize()
{
    auto sceneVisualizer = getModuleFromPar<SceneOsgEarthVisualizer>(par("osgEarthSceneVisualizerModule"), this, true);
    map = sceneVisualizer->getMapNode()->getMap();
    elevationQuery = new osgEarth::ElevationQuery(map);
    coordinateSystem = getModuleFromPar<IGeographicCoordinateSystem>(par("coordinateSystemModule"), this);
}

Coord OsgEarthGround::computeGroundProjection(const Coord &position) const
{
    double elevation = 0;
    auto geoCoord = coordinateSystem->computeGeographicCoordinate(position);
    bool success = elevationQuery->getElevation(osgEarth::GeoPoint(map->getSRS(), deg(geoCoord.longitude).get(), deg(geoCoord.latitude).get()), elevation);
    if (success)
        geoCoord.altitude = m(elevation);
    else {
        // TODO: throw cRuntimeError ?
    }
    return coordinateSystem->computeSceneCoordinate(geoCoord);
}

Coord OsgEarthGround::computeGroundNormal(const Coord &position) const
{
    // we take 3 samples, one at position, and 2 at a distance from it in different directions
    // then compute a cross product to get the normal

    // ??? Make this configurable somehow?
    double distance = 1; // how far the other samples are from the center one, in meters.

    Coord A = computeGroundProjection(position);
    Coord B = computeGroundProjection(position + Coord(distance, 0, 0));
    Coord C = computeGroundProjection(position + Coord(0, distance, 0));

    Coord V1 = B - A;
    Coord V2 = C - A;

    Coord normal = V1 % V2;
    normal.normalize();

    return normal;
}

} // namespace physicalenvironment

} // namespace inet

#endif // defined(WITH_OSGEARTH) && defined(WITH_VISUALIZERS)

