//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/osg/scene/SceneOsgVisualizer.h"

#include <osg/Group>
#include <osgDB/ReadFile>


#include "inet/common/ModuleAccess.h"
#include "inet/visualizer/osg/util/OsgScene.h"
#include "inet/visualizer/osg/util/OsgUtils.h"

namespace inet {

namespace visualizer {

// cOsgCanvas/getOsgCanvas() and setScene()/getScene() are deprecated in favour of the
// renderer-neutral cCanvas3D/c3DSceneNode API, but INET's OSG visualizers must keep
// compiling against OMNeT++ 6.4, which has no such API. Stay on the compatibility path
// until 6.4 is no longer supported.
#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wdeprecated-declarations"
#elif defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

Define_Module(SceneOsgVisualizer);

void SceneOsgVisualizer::initialize(int stage)
{
    SceneOsgVisualizerBase::initialize(stage);
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

void SceneOsgVisualizer::initializeScene()
{
    SceneOsgVisualizerBase::initializeScene();
    auto topLevelScene = check_and_cast<inet::osg::TopLevelScene *>(visualizationTargetModule->getOsgCanvas()->getScene());
    topLevelScene->addChild(new inet::osg::SimulationScene());
}

void SceneOsgVisualizer::initializeViewpoint()
{
    auto boundingSphere = getNetworkBoundingSphere();
    auto center = boundingSphere.center();
    auto radius = boundingSphere.radius();
    double cameraDistanceFactor = par("cameraDistanceFactor");
    auto eye = cOsgCanvas::Vec3d(center.x() + cameraDistanceFactor * radius, center.y() + cameraDistanceFactor * radius, center.z() + cameraDistanceFactor * radius);
    auto viewpointCenter = cOsgCanvas::Vec3d(center.x(), center.y(), center.z());
    auto osgCanvas = visualizationTargetModule->getOsgCanvas();
    osgCanvas->setGenericViewpoint(cOsgCanvas::Viewpoint(eye, viewpointCenter, cOsgCanvas::Vec3d(0, 0, 1)));
}

#if defined(__clang__)
#  pragma clang diagnostic pop
#elif defined(__GNUC__)
#  pragma GCC diagnostic pop
#endif

} // namespace visualizer

} // namespace inet

