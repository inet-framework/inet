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

#include "inet/common/geometry/common/GeographicCoordinateSystem.h"

#if defined(WITH_OSGEARTH) && defined(WITH_VISUALIZERS)
#include <osg/PositionAttitudeTransform>
#include <osgEarth/GeoTransform>
#endif

namespace inet {

const GeoCoord GeoCoord::NIL = GeoCoord(deg(NaN), deg(NaN), m(NaN));

Define_Module(SimpleGeographicCoordinateSystem);

void SimpleGeographicCoordinateSystem::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        playgroundLatitude = deg(par("playgroundLatitude"));
        playgroundLongitude = deg(par("playgroundLongitude"));
        playgroundAltitude = m(par("playgroundAltitude"));
    }
}

Coord SimpleGeographicCoordinateSystem::computePlaygroundCoordinate(const GeoCoord& geographicCoordinate) const
{
    double playgroundX = deg(geographicCoordinate.longitude - playgroundLongitude).get() * cos(fabs(rad(playgroundLatitude).get())) * metersPerDegree;
    double playgroundY = deg(playgroundLatitude - geographicCoordinate.latitude).get() * metersPerDegree;
    return Coord(playgroundX, playgroundY, m(geographicCoordinate.altitude + playgroundAltitude).get());
}

GeoCoord SimpleGeographicCoordinateSystem::computeGeographicCoordinate(const Coord& playgroundCoordinate) const
{
    auto geograpicLatitude = playgroundLatitude - deg(playgroundCoordinate.y / metersPerDegree);
    auto geograpicLongitude = playgroundLongitude + deg(playgroundCoordinate.x / metersPerDegree / cos(fabs(rad(playgroundLatitude).get())));
    return GeoCoord(geograpicLatitude, geograpicLongitude, m(playgroundCoordinate.z) - playgroundAltitude);
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
        auto playgroundLatitude = deg(par("playgroundLatitude"));
        auto playgroundLongitude = deg(par("playgroundLongitude"));
        auto playgroundAltitude = m(par("playgroundAltitude"));
        auto playgroundHeading = deg(par("playgroundHeading"));
        auto playgroundElevation = deg(par("playgroundElevation"));
        auto playgroundBank = deg(par("playgroundBank"));
        playgroundPosition = GeoCoord(playgroundLatitude, playgroundLongitude, playgroundAltitude);

        // The parameter is conventional direction of heading and elevation (positive heading turns left,
        // positive elevation lifts nose), but the EulerAngles class has different expectations.
        playgroundOrientation = EulerAngles(-rad(playgroundHeading - deg(90)), -rad(playgroundElevation), rad(playgroundBank));

        osg::ref_ptr<osgEarth::GeoTransform> geoTransform = new osgEarth::GeoTransform();
        osg::ref_ptr<osg::PositionAttitudeTransform> localTransform = new osg::PositionAttitudeTransform();

        geoTransform->addChild(localTransform);
        geoTransform->setPosition(osgEarth::GeoPoint(mapNode->getMapSRS()->getGeographicSRS(),
            deg(playgroundPosition.longitude).get(), deg(playgroundPosition.latitude).get(), m(playgroundPosition.altitude).get()));

        localTransform->setAttitude(
            osg::Quat(rad(playgroundOrientation.gamma).get(), osg::Vec3d(1.0, 0.0, 0.0)) *
            osg::Quat(rad(playgroundOrientation.beta).get(), osg::Vec3d(0.0, 1.0, 0.0)) *
            osg::Quat(rad(playgroundOrientation.alpha).get(), osg::Vec3d(0.0, 0.0, 1.0)));

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
    osg::Vec3d osgGeographicCoordinate(deg(geographicCoordinate.longitude).get(), deg(geographicCoordinate.latitude).get(), m(geographicCoordinate.altitude).get());
    mapSrs->getGeographicSRS()->transform(osgGeographicCoordinate, mapSrs->getECEF(), ecefCoordinate);
    auto playgroundCoordinate = osg::Vec4d(ecefCoordinate.x(), ecefCoordinate.y(), ecefCoordinate.z(), 1.0) * inverseLocatorMatrix;
    return Coord(playgroundCoordinate.x(), playgroundCoordinate.y(), playgroundCoordinate.z());
}

GeoCoord OsgGeographicCoordinateSystem::computeGeographicCoordinate(const Coord& playgroundCoordinate) const
{
    auto ecefCoordinate = osg::Vec4d(playgroundCoordinate.x, playgroundCoordinate.y, playgroundCoordinate.z, 1.0) * locatorMatrix;
    auto mapSrs = mapNode->getMapSRS();
    osg::Vec3d geographicCoordinate;
    mapSrs->getECEF()->transform(osg::Vec3d(ecefCoordinate.x(), ecefCoordinate.y(), ecefCoordinate.z()), mapSrs->getGeographicSRS(), geographicCoordinate);
    return GeoCoord(deg(geographicCoordinate.y()), deg(geographicCoordinate.x()), m(geographicCoordinate.z()));
}

#endif // WITH_OSGEARTH

} // namespace inet

