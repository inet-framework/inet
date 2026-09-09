//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/vsg/base/LinkVsgVisualizerBase.h"

#include <algorithm>

#include "inet/common/ModuleAccess.h"
#include "inet/visualizer/vsg/util/VsgScene.h"

namespace inet {

namespace visualizer {

LinkVsgVisualizerBase::LinkVsgVisualization::LinkVsgVisualization(inet::vsg::LineNode *node, int sourceModuleId, int destinationModuleId) :
    LinkVisualization(sourceModuleId, destinationModuleId),
    node(node)
{
}

void LinkVsgVisualizerBase::initialize(int stage)
{
    LinkVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        auto canvas = visualizationTargetModule->getCanvas();
        lineManager = LineManager::getVsgLineManager(canvas);
    }
}

void LinkVsgVisualizerBase::refreshDisplay() const
{
    LinkVisualizerBase::refreshDisplay();
    auto simulation = getSimulation();
    for (auto it : linkVisualizations) {
        auto linkVisualization = it.second;
        auto sourceModule = simulation->getModule(linkVisualization->sourceModuleId);
        auto destinationModule = simulation->getModule(linkVisualization->destinationModuleId);
        if (sourceModule != nullptr && destinationModule != nullptr) {
            auto linkVsgVisualization = static_cast<const LinkVsgVisualization *>(linkVisualization);
            auto sourcePosition = getContactPosition(sourceModule, getPosition(destinationModule), lineContactMode, lineContactSpacing);
            auto destinationPosition = getContactPosition(destinationModule, getPosition(sourceModule), lineContactMode, lineContactSpacing);
            auto shift = lineManager->getLineShift(linkVisualization->sourceModuleId, linkVisualization->destinationModuleId, sourcePosition, destinationPosition, lineShiftMode, linkVisualization->shiftOffset) * lineShift;
            // VSG has no detachable line state, so update both endpoints with a single rebuild.
            linkVsgVisualization->node->set(sourcePosition + shift, destinationPosition + shift, cFigure::ARROW_NONE, cFigure::ARROW_BARBED, lineColor, lineStyle, lineWidth);
        }
    }
    visualizationTargetModule->getCanvas()->setAnimationSpeed(linkVisualizations.empty() ? 0 : fadeOutAnimationSpeed, this);
}

const LinkVisualizerBase::LinkVisualization *LinkVsgVisualizerBase::createLinkVisualization(cModule *source, cModule *destination, cPacket *packet) const
{
    auto sourcePosition = getPosition(source);
    auto destinationPosition = getPosition(destination);
    auto node = inet::vsg::LineNode::create();
    node->set(sourcePosition, destinationPosition, cFigure::ARROW_NONE, cFigure::ARROW_BARBED, lineColor, lineStyle, lineWidth);
    return new LinkVsgVisualization(node, source->getId(), destination->getId());
}

void LinkVsgVisualizerBase::addLinkVisualization(std::pair<int, int> sourceAndDestination, const LinkVisualization *linkVisualization)
{
    LinkVisualizerBase::addLinkVisualization(sourceAndDestination, linkVisualization);
    auto linkVsgVisualization = static_cast<const LinkVsgVisualization *>(linkVisualization);
    lineManager->addModuleLine(linkVisualization);
    auto scene = inet::vsg::TopLevelScene::getSimulationScene(visualizationTargetModule);
    scene->addChild(linkVsgVisualization->node);
}

void LinkVsgVisualizerBase::removeLinkVisualization(const LinkVisualization *linkVisualization)
{
    LinkVisualizerBase::removeLinkVisualization(linkVisualization);
    auto linkVsgVisualization = static_cast<const LinkVsgVisualization *>(linkVisualization);
    lineManager->removeModuleLine(linkVisualization);
    auto scene = inet::vsg::TopLevelScene::getSimulationScene(visualizationTargetModule);
    auto& ch = scene->children;
    ch.erase(std::remove(ch.begin(), ch.end(), ::vsg::ref_ptr<::vsg::Node>(linkVsgVisualization->node)), ch.end());
}

void LinkVsgVisualizerBase::setAlpha(const LinkVisualization *linkVisualization, double alpha) const
{
    auto linkVsgVisualization = static_cast<const LinkVsgVisualization *>(linkVisualization);
    linkVsgVisualization->node->setAlpha(alpha);
}

} // namespace visualizer

} // namespace inet
