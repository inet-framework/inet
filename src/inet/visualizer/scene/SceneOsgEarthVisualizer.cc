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
#include "inet/common/OSGScene.h"
#include "inet/common/OSGUtils.h"
#include "inet/environment/contract/IPhysicalEnvironment.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/visualizer/scene/NetworkNodeOsgVisualizer.h"
#include "inet/visualizer/scene/SceneOsgEarthVisualizer.h"

#ifdef WITH_OSG
#include <osg/Group>
#include <osgDB/ReadFile>
#include <osgEarth/Capabilities>
#include <osgEarth/Viewpoint>
#endif // ifdef WITH_OSG

namespace inet {

namespace visualizer {

Define_Module(SceneOsgEarthVisualizer);

#ifdef WITH_OSG

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
        if (par("displayPlayground"))
            initializePlayground();
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
    auto osgCanvas = visualizerTargetModule->getOsgCanvas();
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
    auto playgroundPosition = coordinateSystem->getPlaygroundPosition();
    geoTransform->setPosition(osgEarth::GeoPoint(mapNode->getMapSRS()->getGeographicSRS(),
            playgroundPosition.longitude, playgroundPosition.latitude, playgroundPosition.altitude));

    auto playgroundOrientation = coordinateSystem->getPlaygroundOrientation();
    localTransform->setAttitude(
        osg::Quat(playgroundOrientation.gamma, osg::Vec3d(1.0, 0.0, 0.0)) *
        osg::Quat(playgroundOrientation.beta, osg::Vec3d(0.0, 1.0, 0.0)) *
        osg::Quat(playgroundOrientation.alpha, osg::Vec3d(0.0, 0.0, 1.0)));
}

void SceneOsgEarthVisualizer::initializeViewpoint()
{
    auto boundingSphere = getNetworkBoundingSphere();
    auto radius = boundingSphere.radius();
    auto euclideanCenter = boundingSphere.center();
    auto geographicSrsEye = coordinateSystem->computeGeographicCoordinate(Coord(euclideanCenter.x(), euclideanCenter.y(), euclideanCenter.z()));
    auto osgCanvas = visualizerTargetModule->getOsgCanvas();
#if OMNETPP_BUILDNUM >= 1012
    osgCanvas->setEarthViewpoint(cOsgCanvas::EarthViewpoint(geographicSrsEye.longitude, geographicSrsEye.latitude, geographicSrsEye.altitude, -45, -45, cameraDistanceFactor * radius));
#else
    osgCanvas->setEarthViewpoint(osgEarth::Viewpoint("home", geographicSrsEye.longitude, geographicSrsEye.latitude, geographicSrsEye.altitude, -45, -45, cameraDistanceFactor * radius));
#endif
}

#endif // ifdef WITH_OSG

} // namespace visualizer

} // namespace inet

