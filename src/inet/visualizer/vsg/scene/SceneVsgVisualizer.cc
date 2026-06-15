//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/vsg/scene/SceneVsgVisualizer.h"

#include "inet/common/ModuleAccess.h"
#include "inet/visualizer/vsg/util/VsgScene.h"
#include "inet/visualizer/vsg/util/VsgUtils.h"

#include "qtenv/vsg/vsgscenehandle.h"   // cScene3DNode::getRoot()

namespace inet {

namespace visualizer {

Define_Module(SceneVsgVisualizer);

void SceneVsgVisualizer::initialize(int stage)
{
    SceneVsgVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL)
        initializeScene();
    else if (stage == INITSTAGE_LAST) {
        if (par("displayScene"))
            initializeSceneFloor();
        double axisLength = par("axisLength");
        if (!std::isnan(axisLength))
            initializeAxis(axisLength);
        initializeViewpoint();
    }
}

void SceneVsgVisualizer::initializeScene()
{
    SceneVsgVisualizerBase::initializeScene();
    auto sceneNode = visualizationTargetModule->getOsgCanvas()->getScene();
    auto topLevelScene = sceneNode != nullptr ? dynamic_cast<inet::vsg::TopLevelScene *>(sceneNode->getRoot().get()) : nullptr;
    if (topLevelScene == nullptr)
        throw cRuntimeError("Cannot find the VSG top level scene");
    topLevelScene->addChild(inet::vsg::SimulationScene::create());
}

void SceneVsgVisualizer::initializeViewpoint()
{
    auto boundingSphere = getNetworkBoundingSphere();
    auto center = boundingSphere.first;
    auto radius = boundingSphere.second;
    double cameraDistanceFactor = par("cameraDistanceFactor");
    auto eye = cOsgCanvas::Vec3d(center.x + cameraDistanceFactor * radius, center.y + cameraDistanceFactor * radius, center.z + cameraDistanceFactor * radius);
    auto viewpointCenter = cOsgCanvas::Vec3d(center.x, center.y, center.z);
    auto osgCanvas = visualizationTargetModule->getOsgCanvas();
    osgCanvas->setGenericViewpoint(cOsgCanvas::Viewpoint(eye, viewpointCenter, cOsgCanvas::Vec3d(0, 0, 1)));
}

} // namespace visualizer

} // namespace inet
