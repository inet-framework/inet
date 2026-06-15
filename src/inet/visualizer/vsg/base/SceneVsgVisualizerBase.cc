//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/vsg/base/SceneVsgVisualizerBase.h"

#include <vsg/maths/transform.h>

#include "inet/common/ModuleAccess.h"
#include "inet/visualizer/vsg/scene/NetworkNodeVsgVisualizer.h"
#include "inet/visualizer/vsg/util/VsgScene.h"
#include "inet/visualizer/vsg/util/VsgUtils.h"

#include "qtenv/vsg/vsgscenehandle.h"   // omnetpp::createScene3DNode

namespace inet {

namespace visualizer {

void SceneVsgVisualizerBase::initializeScene()
{
    auto osgCanvas = visualizationTargetModule->getOsgCanvas();
    if (osgCanvas->getScene() != nullptr)
        throw cRuntimeError("3D canvas scene at '%s' has been already initialized", visualizationTargetModule->getFullPath().c_str());
    else {
        auto topLevelScene = inet::vsg::TopLevelScene::create();
        osgCanvas->setScene(omnetpp::createScene3DNode(topLevelScene));
        const char *clearColor = par("clearColor");
        if (*clearColor != '\0')
            osgCanvas->setClearColor(cFigure::Color(clearColor));
        osgCanvas->setZNear(par("zNear"));
        osgCanvas->setZFar(par("zFar"));
        osgCanvas->setFieldOfViewAngle(par("fieldOfView"));
        const char *cameraManipulatorString = par("cameraManipulator");
        cOsgCanvas::CameraManipulatorType cameraManipulator;
        if (!strcmp(cameraManipulatorString, "auto"))
            cameraManipulator = cOsgCanvas::CAM_AUTO;
        else if (!strcmp(cameraManipulatorString, "trackball"))
            cameraManipulator = cOsgCanvas::CAM_TRACKBALL;
        else if (!strcmp(cameraManipulatorString, "terrain"))
            cameraManipulator = cOsgCanvas::CAM_TERRAIN;
        else if (!strcmp(cameraManipulatorString, "overview"))
            cameraManipulator = cOsgCanvas::CAM_OVERVIEW;
        else if (!strcmp(cameraManipulatorString, "earth"))
            cameraManipulator = cOsgCanvas::CAM_EARTH;
        else
            throw cRuntimeError("Unknown camera manipulator: '%s'", cameraManipulatorString);
        osgCanvas->setCameraManipulatorType(cameraManipulator);
    }
}

void SceneVsgVisualizerBase::initializeAxis(double axisLength)
{
    auto scene = inet::vsg::TopLevelScene::getSimulationScene(visualizationTargetModule);
    auto axes = ::vsg::Group::create();
    axes->addChild(inet::vsg::createLine(Coord::ZERO, Coord(axisLength, 0.0, 0.0), cFigure::ARROW_NONE, cFigure::ARROW_BARBED, cFigure::BLACK));
    axes->addChild(inet::vsg::createLine(Coord::ZERO, Coord(0.0, axisLength, 0.0), cFigure::ARROW_NONE, cFigure::ARROW_BARBED, cFigure::BLACK));
    axes->addChild(inet::vsg::createLine(Coord::ZERO, Coord(0.0, 0.0, axisLength), cFigure::ARROW_NONE, cFigure::ARROW_BARBED, cFigure::BLACK));
    scene->addChild(axes);
    double spacing = 1;
    scene->addChild(inet::vsg::createLabel("X", Coord(axisLength + spacing, 0.0, 0.0), cFigure::BLACK));
    scene->addChild(inet::vsg::createLabel("Y", Coord(0.0, axisLength + spacing, 0.0), cFigure::BLACK));
    scene->addChild(inet::vsg::createLabel("Z", Coord(0.0, 0.0, axisLength + spacing), cFigure::BLACK));
}

void SceneVsgVisualizerBase::initializeSceneFloor()
{
    Box sceneBounds = getSceneBounds();
    if (sceneBounds.getMin() != sceneBounds.getMax()) {
        auto color = cFigure::Color(par("sceneColor"));
        double opacity = par("sceneOpacity");
        // TODO: textured floor (sceneImage) and exponential edge shading (sceneShading) not yet ported.
        auto sceneFloor = createSceneFloor(sceneBounds.getMin(), sceneBounds.getMax(), color, opacity);
        auto scene = inet::vsg::TopLevelScene::getSimulationScene(visualizationTargetModule);
        scene->addChild(sceneFloor);
    }
}

::vsg::ref_ptr<::vsg::Node> SceneVsgVisualizerBase::createSceneFloor(const Coord& min, const Coord& max, const cFigure::Color& color, double opacity) const
{
    auto dx = max.x - min.x;
    auto dy = max.y - min.y;
    if (!std::isfinite(dx) || !std::isfinite(dy))
        return ::vsg::Group::create();
    // Sit the floor slightly below the ground plane so coplanar z=0 overlays (network
    // connections, axes, links) don't z-fight with it (the OSG floor used PolygonOffset for this).
    double groundOffset = std::max(2.0, std::sqrt(dx * dx + dy * dy) * 0.005);
    return inet::vsg::createQuad(Coord(min.x, min.y, -groundOffset), Coord(max.x, max.y, -groundOffset), color, opacity);
}

std::pair<Coord, double> SceneVsgVisualizerBase::getNetworkBoundingSphere()
{
    int nodeCount = 0;
    Coord minPos = Coord::ZERO, maxPos = Coord::ZERO;
    auto networkNodeVisualizer = getModuleFromPar<NetworkNodeVsgVisualizer>(par("networkNodeVisualizerModule"), this);
    for (cModule::SubmoduleIterator it(visualizationSubjectModule); !it.end(); it++) {
        auto networkNode = *it;
        if (isNetworkNode(networkNode)) {
            if (auto visualization = networkNodeVisualizer->findNetworkNodeVisualization(networkNode)) {
                Coord p = visualization->getPosition();
                if (nodeCount == 0) { minPos = p; maxPos = p; }
                else {
                    minPos = Coord(std::min(minPos.x, p.x), std::min(minPos.y, p.y), std::min(minPos.z, p.z));
                    maxPos = Coord(std::max(maxPos.x, p.x), std::max(maxPos.y, p.y), std::max(maxPos.z, p.z));
                }
                nodeCount++;
            }
        }
    }
    if (nodeCount == 0)
        return { Coord::ZERO, 100.0 };
    Coord center = (minPos + maxPos) / 2;
    double radius = std::max(100.0, (maxPos - minPos).length() / 2);
    return { center, radius };
}

} // namespace visualizer

} // namespace inet
