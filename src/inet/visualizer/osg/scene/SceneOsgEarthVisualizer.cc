//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/osg/scene/SceneOsgEarthVisualizer.h"

#include "inet/common/ModuleAccess.h"
#include "inet/environment/contract/IPhysicalEnvironment.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/visualizer/osg/scene/NetworkNodeOsgVisualizer.h"
#include "inet/visualizer/osg/util/OsgScene.h"
#include "inet/visualizer/osg/util/OsgUtils.h"

#ifdef WITH_OSGEARTH
#include <osg/Group>
#include <osgDB/ReadFile>
#include <osgEarth/Capabilities>
#include <osgEarth/Viewpoint>
#endif // ifdef WITH_OSGEARTH

namespace inet {

namespace visualizer {

Define_Module(SceneOsgEarthVisualizer);

#ifdef WITH_OSGEARTH

using namespace osgEarth;
using namespace osgEarth::Annotation;
using namespace inet::physicalenvironment;

void SceneOsgEarthVisualizer::initialize(int stage)
{
    SceneOsgVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        coordinateSystem.reference(this, "coordinateSystemModule", true);
        cameraDistanceFactor = par("cameraDistanceFactor");
        initializeScene();
    }
    else if (stage == INITSTAGE_LAST) {
        initializeLocator();
        if (par("displayScene"))
            initializeSceneFloor();
        double axisLength = par("axisLength");
        if (!std::isnan(axisLength))
            initializeAxis(axisLength);
        initializeViewpoint();
    }
}

void SceneOsgEarthVisualizer::initializeScene()
{
    SceneOsgVisualizerBase::initializeScene();
    const char *mapFileString = par("mapFile");
    auto mapScene = osgDB::readNodeFile(mapFileString);
    if (mapScene == nullptr)
        throw cRuntimeError("Could not read earth map file '%s'", mapFileString);
    auto osgCanvas = visualizationTargetModule->getOsgCanvas();
    osgCanvas->setViewerStyle(cOsgCanvas::STYLE_EARTH);
    auto topLevelScene = check_and_cast<inet::osg::TopLevelScene *>(osgCanvas->getScene());
    topLevelScene->addChild(mapScene);
    mapNode = MapNode::findMapNode(mapScene);
    if (mapNode == nullptr)
        throw cRuntimeError("Could not find map node in the scene");
    geoTransform = new osgEarth::GeoTransform();
    topLevelScene->addChild(geoTransform);
    localTransform = new osg::PositionAttitudeTransform();
    geoTransform->addChild(localTransform);
    localTransform->addChild(new inet::osg::SimulationScene());
}

void SceneOsgEarthVisualizer::initializeLocator()
{
    auto scenePosition = coordinateSystem->getScenePosition();
    geoTransform->setPosition(osgEarth::GeoPoint(mapNode->getMapSRS()->getGeographicSRS(),
            deg(scenePosition.longitude).get(), deg(scenePosition.latitude).get(), m(scenePosition.altitude).get()));

    auto sceneOrientation = coordinateSystem->getSceneOrientation();
    localTransform->setAttitude(osg::Quat(osg::Vec4d(sceneOrientation.v.x, sceneOrientation.v.y, sceneOrientation.v.z, sceneOrientation.s)));
}

void SceneOsgEarthVisualizer::initializeViewpoint()
{
    auto boundingSphere = getNetworkBoundingSphere();
    auto radius = boundingSphere.radius();
    auto euclideanCenter = boundingSphere.center();
    auto geographicSrsEye = coordinateSystem->computeGeographicCoordinate(Coord(euclideanCenter.x(), euclideanCenter.y(), euclideanCenter.z()));
    auto osgCanvas = visualizationTargetModule->getOsgCanvas();
    osgCanvas->setEarthViewpoint(cOsgCanvas::EarthViewpoint(deg(geographicSrsEye.longitude).get(), deg(geographicSrsEye.latitude).get(), m(geographicSrsEye.altitude).get(), -45, -45, cameraDistanceFactor * radius));
}

#endif // ifdef WITH_OSGEARTH

} // namespace visualizer

} // namespace inet

