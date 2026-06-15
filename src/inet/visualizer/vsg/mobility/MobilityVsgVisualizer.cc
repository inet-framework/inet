//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/vsg/mobility/MobilityVsgVisualizer.h"

#include <algorithm>

#include "inet/common/ModuleAccess.h"
#include "inet/visualizer/vsg/util/VsgScene.h"
#include "inet/visualizer/vsg/util/VsgUtils.h"

namespace inet {

namespace visualizer {

Define_Module(MobilityVsgVisualizer);

MobilityVsgVisualizer::MobilityVsgVisualization::MobilityVsgVisualization(::vsg::ref_ptr<::vsg::Group> trail, IMobility *mobility, const cFigure::Color& color) :
    MobilityVisualization(mobility),
    trail(trail),
    color(color)
{
}

void MobilityVsgVisualizer::initialize(int stage)
{
    MobilityVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
}

void MobilityVsgVisualizer::refreshDisplay() const
{
    MobilityVisualizerBase::refreshDisplay();
    for (auto it : mobilityVisualizations) {
        auto mobilityVisualization = static_cast<MobilityVsgVisualization *>(it.second);
        auto position = mobilityVisualization->mobility->getCurrentPosition();
        if (displayMovementTrails)
            extendMovementTrail(mobilityVisualization, position);
    }
    visualizationTargetModule->getCanvas()->setAnimationSpeed(mobilityVisualizations.empty() ? 0 : animationSpeed, this);
}

MobilityVsgVisualizer::MobilityVisualization *MobilityVsgVisualizer::createMobilityVisualization(IMobility *mobility)
{
    auto module = const_cast<cModule *>(check_and_cast<const cModule *>(mobility));
    auto color = movementTrailLineColorSet.getColor(module->getId());
    auto trail = ::vsg::Group::create();
    return new MobilityVsgVisualization(trail, mobility, color);
}

void MobilityVsgVisualizer::addMobilityVisualization(const IMobility *mobility, MobilityVisualization *mobilityVisualization)
{
    MobilityVisualizerBase::addMobilityVisualization(mobility, mobilityVisualization);
    auto mobilityVsgVisualization = static_cast<MobilityVsgVisualization *>(mobilityVisualization);
    if (displayMovementTrails) {
        auto scene = inet::vsg::TopLevelScene::getSimulationScene(visualizationTargetModule);
        scene->addChild(mobilityVsgVisualization->trail);
    }
}

void MobilityVsgVisualizer::removeMobilityVisualization(const MobilityVisualization *mobilityVisualization)
{
    auto mobilityVsgVisualization = static_cast<const MobilityVsgVisualization *>(mobilityVisualization);
    if (displayMovementTrails) {
        auto scene = inet::vsg::TopLevelScene::getSimulationScene(visualizationTargetModule);
        auto& ch = scene->children;
        ch.erase(std::remove(ch.begin(), ch.end(), ::vsg::ref_ptr<::vsg::Node>(mobilityVsgVisualization->trail)), ch.end());
    }
    MobilityVisualizerBase::removeMobilityVisualization(mobilityVisualization);
}

void MobilityVsgVisualizer::extendMovementTrail(MobilityVsgVisualization *visualization, const Coord& position) const
{
    auto& points = visualization->trailPoints;
    if (points.empty())
        points.push_back(position);
    else {
        auto& last = points.back();
        auto dx = last.x - position.x;
        auto dy = last.y - position.y;
        auto dz = last.z - position.z;
        if (dx * dx + dy * dy + dz * dz <= 1)
            return; // didn't move far enough to extend the trail
        points.push_back(position);
        if ((int)points.size() > trailLength)
            points.erase(points.begin());
    }
    // VSG has no detachable line state, so rebuild the (small, capped) trail polyline in place.
    // TODO: a dynamic vertex buffer would avoid rebuilding the pipeline as the trail grows.
    visualization->trail->children.clear();
    if (points.size() >= 2)
        visualization->trail->addChild(inet::vsg::createPolyline(points, cFigure::ARROW_NONE, cFigure::ARROW_NONE,
                visualization->color, movementTrailLineStyle, movementTrailLineWidth));
}

} // namespace visualizer

} // namespace inet
