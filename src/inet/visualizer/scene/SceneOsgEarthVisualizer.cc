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
#include "inet/common/OsgScene.h"
#include "inet/common/OsgUtils.h"
#include "inet/environment/contract/IPhysicalEnvironment.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/visualizer/scene/NetworkNodeOsgVisualizer.h"
#include "inet/visualizer/scene/SceneOsgEarthVisualizer.h"

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
        coordinateSystem = getModuleFromPar<IGeographicCoordinateSystem>(par("coordinateSystemModule"), this);
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
    const char* mapFileString = par("mapFile");
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
#if OMNETPP_BUILDNUM >= 1012
    osgCanvas->setEarthViewpoint(cOsgCanvas::EarthViewpoint(deg(geographicSrsEye.longitude).get(), deg(geographicSrsEye.latitude).get(), m(geographicSrsEye.altitude).get(), -45, -45, cameraDistanceFactor * radius));
#else
    osgCanvas->setEarthViewpoint(osgEarth::Viewpoint("home", deg(geographicSrsEye.longitude).get(), deg(geographicSrsEye.latitude).get(), m(geographicSrsEye.altitude).get(), -45, -45, cameraDistanceFactor * radius));
#endif
}

#endif // ifdef WITH_OSGEARTH

} // namespace visualizer

} // namespace inet

