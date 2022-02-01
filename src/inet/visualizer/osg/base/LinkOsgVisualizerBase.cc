//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/osg/base/LinkOsgVisualizerBase.h"

#include <osg/Geode>
#include <osg/LineWidth>

#include "inet/common/LayeredProtocolBase.h"
#include "inet/common/ModuleAccess.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/visualizer/osg/util/OsgScene.h"

namespace inet {

namespace visualizer {

LinkOsgVisualizerBase::LinkOsgVisualization::LinkOsgVisualization(inet::osg::LineNode *node, int sourceModuleId, int destinationModuleId) :
    LinkVisualization(sourceModuleId, destinationModuleId),
    node(node)
{
}

void LinkOsgVisualizerBase::initialize(int stage)
{
    LinkVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        auto canvas = visualizationTargetModule->getCanvas();
        lineManager = LineManager::getOsgLineManager(canvas);
    }
}

void LinkOsgVisualizerBase::refreshDisplay() const
{
    LinkVisualizerBase::refreshDisplay();
    auto simulation = getSimulation();
    // TODO share common part with LinkCanvasVisualizerBase
    for (auto it : linkVisualizations) {
        auto linkVisualization = it.second;
        auto sourceModule = simulation->getModule(linkVisualization->sourceModuleId);
        auto destinationModule = simulation->getModule(linkVisualization->destinationModuleId);
        if (sourceModule != nullptr && destinationModule != nullptr) {
            auto linkOsgVisualization = static_cast<const LinkOsgVisualization *>(linkVisualization);
            auto sourcePosition = getContactPosition(sourceModule, getPosition(destinationModule), lineContactMode, lineContactSpacing);
            auto destinationPosition = getContactPosition(destinationModule, getPosition(sourceModule), lineContactMode, lineContactSpacing);
            auto shift = lineManager->getLineShift(linkVisualization->sourceModuleId, linkVisualization->destinationModuleId, sourcePosition, destinationPosition, lineShiftMode, linkVisualization->shiftOffset) * lineShift;
            linkOsgVisualization->node->setStart(sourcePosition + shift);
            linkOsgVisualization->node->setEnd(destinationPosition + shift);
        }
    }
    visualizationTargetModule->getCanvas()->setAnimationSpeed(linkVisualizations.empty() ? 0 : fadeOutAnimationSpeed, this);
}

const LinkVisualizerBase::LinkVisualization *LinkOsgVisualizerBase::createLinkVisualization(cModule *source, cModule *destination, cPacket *packet) const
{
    auto sourcePosition = getPosition(source);
    auto destinationPosition = getPosition(destination);
    auto node = new inet::osg::LineNode(sourcePosition, destinationPosition, cFigure::ARROW_NONE, cFigure::ARROW_BARBED, lineWidth);
    node->setStateSet(inet::osg::createLineStateSet(lineColor, lineStyle, lineWidth, true)); // < add the overlay as configurable parameter?
    return new LinkOsgVisualization(node, source->getId(), destination->getId());
}

void LinkOsgVisualizerBase::addLinkVisualization(std::pair<int, int> sourceAndDestination, const LinkVisualization *linkVisualization)
{
    LinkVisualizerBase::addLinkVisualization(sourceAndDestination, linkVisualization);
    auto linkOsgVisualization = static_cast<const LinkOsgVisualization *>(linkVisualization);
    auto scene = inet::osg::TopLevelScene::getSimulationScene(visualizationTargetModule);
    lineManager->addModuleLine(linkVisualization);
    scene->addChild(linkOsgVisualization->node);
}

void LinkOsgVisualizerBase::removeLinkVisualization(const LinkVisualization *linkVisualization)
{
    LinkVisualizerBase::removeLinkVisualization(linkVisualization);
    auto linkOsgVisualization = static_cast<const LinkOsgVisualization *>(linkVisualization);
    auto node = linkOsgVisualization->node;
    lineManager->removeModuleLine(linkVisualization);
    node->getParent(0)->removeChild(node);
}

void LinkOsgVisualizerBase::setAlpha(const LinkVisualization *link, double alpha) const
{
    auto linkOsgVisualization = static_cast<const LinkOsgVisualization *>(link);
    auto node = linkOsgVisualization->node;
    auto material = static_cast<osg::Material *>(node->getOrCreateStateSet()->getAttribute(osg::StateAttribute::MATERIAL));
    material->setAlpha(osg::Material::FRONT_AND_BACK, alpha);
}

} // namespace visualizer

} // namespace inet

