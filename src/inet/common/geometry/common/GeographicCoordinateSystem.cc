//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/geometry/common/GeographicCoordinateSystem.h"

#if defined(WITH_OSGEARTH) && defined(INET_WITH_VISUALIZATIONOSG)
#include <osg/PositionAttitudeTransform>
#include <osgEarth/GeoTransform>
#endif

namespace inet {

const GeoCoord GeoCoord::NIL = GeoCoord(deg(NaN), deg(NaN), m(NaN));

Define_Module(SimpleGeographicCoordinateSystem);

void SimpleGeographicCoordinateSystem::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        sceneLatitude = deg(par("sceneLatitude"));
        sceneLongitude = deg(par("sceneLongitude"));
        sceneAltitude = m(par("sceneAltitude"));
    }
}

Coord SimpleGeographicCoordinateSystem::computeSceneCoordinate(const GeoCoord& geographicCoordinate) const
{
    double sceneX = (geographicCoordinate.longitude - sceneLongitude).get<deg>() * cos(fabs(sceneLatitude.get<rad>())) * metersPerDegree;
    double sceneY = (sceneLatitude - geographicCoordinate.latitude).get<deg>() * metersPerDegree;
    return Coord(sceneX, sceneY, (geographicCoordinate.altitude + sceneAltitude).get<m>());
}

GeoCoord SimpleGeographicCoordinateSystem::computeGeographicCoordinate(const Coord& sceneCoordinate) const
{
    auto geograpicLatitude = sceneLatitude - deg(sceneCoordinate.y / metersPerDegree);
    auto geograpicLongitude = sceneLongitude + deg(sceneCoordinate.x / metersPerDegree / cos(fabs(sceneLatitude.get<rad>())));
    return GeoCoord(geograpicLatitude, geograpicLongitude, m(sceneCoordinate.z) - sceneAltitude);
}

#if defined(WITH_OSGEARTH) && defined(INET_WITH_VISUALIZATIONOSG)

Define_Module(OsgGeographicCoordinateSystem);

void OsgGeographicCoordinateSystem::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        auto mapScene = getParentModule()->getOsgCanvas()->getScene();
        mapNode = osgEarth::MapNode::findMapNode(mapScene);
        if (mapNode == nullptr)
            throw cRuntimeError("Count not find map node in the scene");
        auto sceneLatitude = deg(par("sceneLatitude"));
        auto sceneLongitude = deg(par("sceneLongitude"));
        auto sceneAltitude = m(par("sceneAltitude"));
        auto sceneHeading = deg(par("sceneHeading"));
        auto sceneElevation = deg(par("sceneElevation"));
        auto sceneBank = deg(par("sceneBank"));
        scenePosition = GeoCoord(sceneLatitude, sceneLongitude, sceneAltitude);

        // The parameter is conventional direction of heading and elevation (positive heading turns left,
        // positive elevation lifts nose), but the EulerAngles class has different expectations.
        sceneOrientation = Quaternion(EulerAngles(-rad(sceneHeading - deg(90)), -rad(sceneElevation), rad(sceneBank)));

        osg::ref_ptr<osgEarth::GeoTransform> geoTransform = new osgEarth::GeoTransform();
        osg::ref_ptr<osg::PositionAttitudeTransform> localTransform = new osg::PositionAttitudeTransform();

        geoTransform->addChild(localTransform);
        geoTransform->setPosition(osgEarth::GeoPoint(mapNode->getMapSRS()->getGeographicSRS(),
            scenePosition.longitude.get<deg>(), scenePosition.latitude.get<deg>(), scenePosition.altitude.get<m>()));

        localTransform->setAttitude(osg::Quat(osg::Vec4d(sceneOrientation.v.x, sceneOrientation.v.y, sceneOrientation.v.z, sceneOrientation.s)));

        osg::ref_ptr<osg::Group> child = new osg::Group();
        localTransform->addChild(child);

        auto matrices = child->getWorldMatrices();
        ASSERT(matrices.size() == 1);

        locatorMatrix = matrices[0];
        inverseLocatorMatrix.invert(locatorMatrix);
    }
}

Coord OsgGeographicCoordinateSystem::computeSceneCoordinate(const GeoCoord& geographicCoordinate) const
{
    auto mapSrs = mapNode->getMapSRS();
    osg::Vec3d ecefCoordinate;
    osg::Vec3d osgGeographicCoordinate(geographicCoordinate.longitude.get<deg>(), geographicCoordinate.latitude.get<deg>(), geographicCoordinate.altitude.get<m>());
    mapSrs->getGeographicSRS()->transform(osgGeographicCoordinate, mapSrs->getECEF(), ecefCoordinate);
    auto sceneCoordinate = osg::Vec4d(ecefCoordinate.x(), ecefCoordinate.y(), ecefCoordinate.z(), 1.0) * inverseLocatorMatrix;
    return Coord(sceneCoordinate.x(), sceneCoordinate.y(), sceneCoordinate.z());
}

GeoCoord OsgGeographicCoordinateSystem::computeGeographicCoordinate(const Coord& sceneCoordinate) const
{
    auto ecefCoordinate = osg::Vec4d(sceneCoordinate.x, sceneCoordinate.y, sceneCoordinate.z, 1.0) * locatorMatrix;
    auto mapSrs = mapNode->getMapSRS();
    osg::Vec3d geographicCoordinate;
    mapSrs->getECEF()->transform(osg::Vec3d(ecefCoordinate.x(), ecefCoordinate.y(), ecefCoordinate.z()), mapSrs->getGeographicSRS(), geographicCoordinate);
    return GeoCoord(deg(geographicCoordinate.y()), deg(geographicCoordinate.x()), m(geographicCoordinate.z()));
}

#endif // WITH_OSGEARTH

} // namespace inet

