//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/canvas/linklayer/LinkBreakCanvasVisualizer.h"

#include "inet/common/ModuleAccess.h"

namespace inet {

namespace visualizer {

Define_Module(LinkBreakCanvasVisualizer);

LinkBreakCanvasVisualizer::LinkBreakCanvasVisualization::LinkBreakCanvasVisualization(cIconFigure *figure, int transmitterModuleId, int receiverModuleId) :
    LinkBreakVisualization(transmitterModuleId, receiverModuleId),
    figure(figure)
{
}

void LinkBreakCanvasVisualizer::initialize(int stage)
{
    LinkBreakVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        zIndex = par("zIndex");
        auto canvas = visualizationTargetModule->getCanvas();
        canvasProjection = CanvasProjection::getCanvasProjection(canvas);
        linkBreakGroup = new cGroupFigure("link breaks");
        linkBreakGroup->setZIndex(zIndex);
        canvas->addFigure(linkBreakGroup);
    }
}

void LinkBreakCanvasVisualizer::refreshDisplay() const
{
    LinkBreakVisualizerBase::refreshDisplay();
    for (auto it : linkBreakVisualizations) {
        auto linkBreakVisualization = static_cast<const LinkBreakCanvasVisualization *>(it.second);
        auto transmitter = getSimulation()->getModule(linkBreakVisualization->transmitterModuleId);
        auto receiver = getSimulation()->getModule(linkBreakVisualization->receiverModuleId);
        auto transmitterPosition = canvasProjection->computeCanvasPoint(getPosition(getContainingNode(transmitter)));
        auto receiverPosition = canvasProjection->computeCanvasPoint(getPosition(getContainingNode(receiver)));
        auto figure = linkBreakVisualization->figure;
        figure->setPosition((transmitterPosition + receiverPosition) / 2);
    }
    visualizationTargetModule->getCanvas()->setAnimationSpeed(linkBreakVisualizations.empty() ? 0 : fadeOutAnimationSpeed, this);
}

const LinkBreakVisualizerBase::LinkBreakVisualization *LinkBreakCanvasVisualizer::createLinkBreakVisualization(cModule *transmitter, cModule *receiver) const
{
    std::string icon(this->icon);
    auto transmitterPosition = canvasProjection->computeCanvasPoint(getPosition(getContainingNode(transmitter)));
    auto receiverPosition = canvasProjection->computeCanvasPoint(getPosition(getContainingNode(receiver)));
    auto figure = new cIconFigure("linkBroken");
    figure->setTags((std::string("link_break ") + tags).c_str());
    figure->setTooltip("This icon represents a link break between two network nodes");
    figure->setImageName(icon.substr(0, icon.find_first_of(".")).c_str());
    figure->setTintAmount(iconTintAmount);
    figure->setTintColor(iconTintColor);
    figure->setPosition((transmitterPosition + receiverPosition) / 2);
    return new LinkBreakCanvasVisualization(figure, transmitter->getId(), receiver->getId());
}

void LinkBreakCanvasVisualizer::addLinkBreakVisualization(const LinkBreakVisualization *linkBreakVisualization)
{
    LinkBreakVisualizerBase::addLinkBreakVisualization(linkBreakVisualization);
    auto linkBreakCanvasVisualization = static_cast<const LinkBreakCanvasVisualization *>(linkBreakVisualization);
    linkBreakGroup->addFigure(linkBreakCanvasVisualization->figure);
}

void LinkBreakCanvasVisualizer::removeLinkBreakVisualization(const LinkBreakVisualization *linkBreakVisualization)
{
    LinkBreakVisualizerBase::removeLinkBreakVisualization(linkBreakVisualization);
    auto linkBreakCanvasVisualization = static_cast<const LinkBreakCanvasVisualization *>(linkBreakVisualization);
    linkBreakGroup->removeFigure(linkBreakCanvasVisualization->figure);
}

void LinkBreakCanvasVisualizer::setAlpha(const LinkBreakVisualization *linkBreakVisualization, double alpha) const
{
    auto linkBreakCanvasVisualization = static_cast<const LinkBreakCanvasVisualization *>(linkBreakVisualization);
    auto figure = linkBreakCanvasVisualization->figure;
    figure->setOpacity(alpha);
}

} // namespace visualizer

} // namespace inet

