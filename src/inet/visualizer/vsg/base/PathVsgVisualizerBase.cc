//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/vsg/base/PathVsgVisualizerBase.h"

#include <algorithm>

#include "inet/common/ModuleAccess.h"
#include "inet/visualizer/vsg/util/VsgScene.h"
#include "inet/visualizer/vsg/util/VsgUtils.h"

namespace inet {

namespace visualizer {

PathVsgVisualizerBase::PathVsgVisualization::PathVsgVisualization(const char *label, const std::vector<int>& path, ::vsg::ref_ptr<::vsg::Group> node, const std::vector<Coord>& points, const cFigure::Color& color) :
    PathVisualization(label, path),
    node(node),
    points(points),
    color(color)
{
}

void PathVsgVisualizerBase::initialize(int stage)
{
    PathVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        auto canvas = visualizationTargetModule->getCanvas();
        lineManager = LineManager::getVsgLineManager(canvas);
    }
}

void PathVsgVisualizerBase::refreshDisplay() const
{
    PathVisualizerBase::refreshDisplay();
    visualizationTargetModule->getCanvas()->setAnimationSpeed(pathVisualizations.empty() ? 0 : fadeOutAnimationSpeed, this);
}

const PathVisualizerBase::PathVisualization *PathVsgVisualizerBase::createPathVisualization(const char *label, const std::vector<int>& path, cPacket *packet) const
{
    std::vector<Coord> points;
    for (auto id : path) {
        auto module = getSimulation()->getModule(id);
        points.push_back(getPosition(module));
    }
    auto color = lineColorSet.getColor(pathVisualizations.size());
    auto node = ::vsg::Group::create();   // container; the polyline is (re)built into it (fade rebuilds)
    node->addChild(inet::vsg::createPolyline(points, cFigure::ARROW_NONE, cFigure::ARROW_BARBED, color, lineStyle, lineWidth));
    return new PathVsgVisualization(label, path, node, points, color);
}

void PathVsgVisualizerBase::addPathVisualization(const PathVisualization *pathVisualization)
{
    PathVisualizerBase::addPathVisualization(pathVisualization);
    auto pathVsgVisualization = static_cast<const PathVsgVisualization *>(pathVisualization);
    lineManager->addModulePath(pathVisualization);
    auto scene = inet::vsg::TopLevelScene::getSimulationScene(visualizationTargetModule);
    scene->addChild(pathVsgVisualization->node);
}

void PathVsgVisualizerBase::removePathVisualization(const PathVisualization *pathVisualization)
{
    PathVisualizerBase::removePathVisualization(pathVisualization);
    auto pathVsgVisualization = static_cast<const PathVsgVisualization *>(pathVisualization);
    lineManager->removeModulePath(pathVisualization);
    auto scene = inet::vsg::TopLevelScene::getSimulationScene(visualizationTargetModule);
    auto& ch = scene->children;
    ch.erase(std::remove(ch.begin(), ch.end(), ::vsg::ref_ptr<::vsg::Node>(pathVsgVisualization->node)), ch.end());
}

void PathVsgVisualizerBase::setAlpha(const PathVisualization *pathVisualization, double alpha) const
{
    // VSG bakes opacity into geometry, so rebuild the polyline at the new opacity.
    auto pathVsgVisualization = static_cast<const PathVsgVisualization *>(pathVisualization);
    pathVsgVisualization->node->children.clear();
    pathVsgVisualization->node->addChild(inet::vsg::createPolyline(pathVsgVisualization->points, cFigure::ARROW_NONE, cFigure::ARROW_BARBED,
            pathVsgVisualization->color, lineStyle, lineWidth, alpha));
}

} // namespace visualizer

} // namespace inet
