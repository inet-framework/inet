//
// Copyright (C) 2013 OpenSim Ltd.
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

#include "inet/common/geometry/common/CoordinateSystem.h"

#if defined(WITH_OSGEARTH) && defined(WITH_VISUALIZERS)
#include <osg/PositionAttitudeTransform>
#include <osgEarth/GeoTransform>
#endif

namespace inet {

const GeoCoord GeoCoord::NIL = GeoCoord(NaN, NaN, NaN);

Define_Module(SimpleGeographicCoordinateSystem);

void SimpleGeographicCoordinateSystem::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        playgroundLatitude = par("playgroundLatitude");
        playgroundLongitude = par("playgroundLongitude");
        playgroundAltitude = par("playgroundAltitude");
    }
}

Coord SimpleGeographicCoordinateSystem::computePlaygroundCoordinate(const GeoCoord& geographicCoordinate) const
{
    double playgroundX = (geographicCoordinate.longitude - playgroundLongitude) * cos(fabs(playgroundLatitude / 180 * M_PI)) * metersPerDegree;
    double playgroundY = (playgroundLatitude - geographicCoordinate.latitude) * metersPerDegree;
    return Coord(playgroundX, playgroundY, geographicCoordinate.altitude + playgroundAltitude);
}

GeoCoord SimpleGeographicCoordinateSystem::computeGeographicCoordinate(const Coord& playgroundCoordinate) const
{
    double geograpicLatitude = playgroundLatitude - playgroundCoordinate.y / metersPerDegree;
    double geograpicLongitude = playgroundLongitude + playgroundCoordinate.x / metersPerDegree / cos(fabs(playgroundLatitude / 180 * M_PI));
    return GeoCoord(geograpicLatitude, geograpicLongitude, playgroundCoordinate.z - playgroundAltitude);
}

#if defined(WITH_OSGEARTH) && defined(WITH_VISUALIZERS)

Define_Module(OsgGeographicCoordinateSystem);

void OsgGeographicCoordinateSystem::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        auto mapScene = getParentModule()->getOsgCanvas()->getScene();
        mapNode = osgEarth::MapNode::findMapNode(mapScene);
        if (mapNode == nullptr)
            throw cRuntimeError("Count not find map node in the scene");
        double playgroundLatitude = par("playgroundLatitude");
        double playgroundLongitude = par("playgroundLongitude");
        double playgroundAltitude = par("playgroundAltitude");
        double playgroundHeading = par("playgroundHeading");
        double playgroundElevation = par("playgroundElevation");
        double playgroundBank = par("playgroundBank");
        playgroundPosition = GeoCoord(playgroundLatitude, playgroundLongitude, playgroundAltitude);
        playgroundOrientation = EulerAngles(playgroundHeading, playgroundElevation, playgroundBank);

        osg::ref_ptr<osgEarth::GeoTransform> geoTransform = new osgEarth::GeoTransform();
        osg::ref_ptr<osg::PositionAttitudeTransform> localTransform = new osg::PositionAttitudeTransform();

        geoTransform->addChild(localTransform);
        geoTransform->setPosition(osgEarth::GeoPoint(mapNode->getMapSRS()->getGeographicSRS(),
            playgroundPosition.longitude, playgroundPosition.latitude, playgroundPosition.altitude));

        localTransform->setAttitude(
            osg::Quat(playgroundOrientation.gamma, osg::Vec3d(1.0, 0.0, 0.0)) *
            osg::Quat(playgroundOrientation.beta, osg::Vec3d(0.0, 1.0, 0.0)) *
            osg::Quat(playgroundOrientation.alpha, osg::Vec3d(0.0, 0.0, 1.0)));

        osg::ref_ptr<osg::Group> child = new osg::Group();
        localTransform->addChild(child);

        auto matrices = child->getWorldMatrices();
        ASSERT(matrices.size() == 1);

        locatorMatrix = matrices[0];
        inverseLocatorMatrix.invert(locatorMatrix);
    }
}

Coord OsgGeographicCoordinateSystem::computePlaygroundCoordinate(const GeoCoord& geographicCoordinate) const
{
    auto mapSrs = mapNode->getMapSRS();
    osg::Vec3d ecefCoordinate;
    mapSrs->getGeographicSRS()->transform(osg::Vec3d(geographicCoordinate.longitude, geographicCoordinate.latitude, geographicCoordinate.altitude), mapSrs->getECEF(), ecefCoordinate);
    auto playgroundCoordinate = osg::Vec4d(ecefCoordinate.x(), ecefCoordinate.y(), ecefCoordinate.z(), 1.0) * inverseLocatorMatrix;
    return Coord(playgroundCoordinate.x(), playgroundCoordinate.y(), playgroundCoordinate.z());
}

GeoCoord OsgGeographicCoordinateSystem::computeGeographicCoordinate(const Coord& playgroundCoordinate) const
{
    auto ecefCoordinate = osg::Vec4d(playgroundCoordinate.x, playgroundCoordinate.y, playgroundCoordinate.z, 1.0) * locatorMatrix;
    auto mapSrs = mapNode->getMapSRS();
    osg::Vec3d geographicCoordinate;
    mapSrs->getECEF()->transform(osg::Vec3d(ecefCoordinate.x(), ecefCoordinate.y(), ecefCoordinate.z()), mapSrs->getGeographicSRS(), geographicCoordinate);
    return GeoCoord(geographicCoordinate.y(), geographicCoordinate.x(), geographicCoordinate.z());
}

#endif // WITH_OSGEARTH

} // namespace inet

