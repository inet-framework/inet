//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/osg/base/PathOsgVisualizerBase.h"

#include <osg/LineWidth>

#include "inet/common/LayeredProtocolBase.h"
#include "inet/common/ModuleAccess.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/visualizer/osg/util/OsgScene.h"
#include "inet/visualizer/osg/util/OsgUtils.h"

namespace inet {

namespace visualizer {

PathOsgVisualizerBase::PathOsgVisualization::PathOsgVisualization(const char *label, const std::vector<int>& path, osg::Node *node) :
    PathVisualization(label, path),
    node(node)
{
}

void PathOsgVisualizerBase::initialize(int stage)
{
    PathVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        auto canvas = visualizationTargetModule->getCanvas();
        lineManager = LineManager::getOsgLineManager(canvas);
    }
}

void PathOsgVisualizerBase::refreshDisplay() const
{
    PathVisualizerBase::refreshDisplay();
    // TODO switch to osg canvas when API is extended
    visualizationTargetModule->getCanvas()->setAnimationSpeed(pathVisualizations.empty() ? 0 : fadeOutAnimationSpeed, this);
}

const PathVisualizerBase::PathVisualization *PathOsgVisualizerBase::createPathVisualization(const char *label, const std::vector<int>& path, cPacket *packet) const
{
    std::vector<Coord> points;
    for (auto id : path) {
        auto node = getSimulation()->getModule(id);
        points.push_back(getPosition(node));
    }
    auto node = inet::osg::createPolyline(points, cFigure::ARROW_NONE, cFigure::ARROW_BARBED);
    node->setStateSet(inet::osg::createLineStateSet(lineColorSet.getColor(pathVisualizations.size()), lineStyle, lineWidth));
    return new PathOsgVisualization(label, path, node);
}

void PathOsgVisualizerBase::addPathVisualization(const PathVisualization *pathVisualization)
{
    PathVisualizerBase::addPathVisualization(pathVisualization);
    auto pathOsgVisualization = static_cast<const PathOsgVisualization *>(pathVisualization);
    auto scene = inet::osg::TopLevelScene::getSimulationScene(visualizationTargetModule);
    lineManager->addModulePath(pathVisualization);
    scene->addChild(pathOsgVisualization->node);
}

void PathOsgVisualizerBase::removePathVisualization(const PathVisualization *pathVisualization)
{
    PathVisualizerBase::removePathVisualization(pathVisualization);
    auto pathOsgVisualization = static_cast<const PathOsgVisualization *>(pathVisualization);
    auto node = pathOsgVisualization->node;
    lineManager->removeModulePath(pathVisualization);
    node->getParent(0)->removeChild(node);
}

void PathOsgVisualizerBase::setAlpha(const PathVisualization *pathVisualization, double alpha) const
{
    auto pathOsgVisualization = static_cast<const PathOsgVisualization *>(pathVisualization);
    auto node = pathOsgVisualization->node;
    auto stateSet = node->getOrCreateStateSet();
    auto material = static_cast<osg::Material *>(stateSet->getAttribute(osg::StateAttribute::MATERIAL));
    material->setAlpha(osg::Material::FRONT_AND_BACK, alpha);
}

} // namespace visualizer

} // namespace inet

