//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/canvas/base/LinkCanvasVisualizerBase.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/geometry/object/LineSegment.h"
#include "inet/common/geometry/shape/Cuboid.h"
#include "inet/mobility/contract/IMobility.h"

namespace inet {

namespace visualizer {

LinkCanvasVisualizerBase::LinkCanvasVisualization::LinkCanvasVisualization(LabeledLineFigure *figure, int sourceModuleId, int destinationModuleId) :
    LinkVisualization(sourceModuleId, destinationModuleId),
    figure(figure)
{
}

LinkCanvasVisualizerBase::LinkCanvasVisualization::~LinkCanvasVisualization()
{
    delete figure;
}

void LinkCanvasVisualizerBase::initialize(int stage)
{
    LinkVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        zIndex = par("zIndex");
        auto canvas = visualizationTargetModule->getCanvas();
        lineManager = LineManager::getCanvasLineManager(canvas);
        canvasProjection = CanvasProjection::getCanvasProjection(canvas);
        linkGroup = new cGroupFigure("links");
        linkGroup->setZIndex(zIndex);
        canvas->addFigure(linkGroup);
    }
}

void LinkCanvasVisualizerBase::refreshDisplay() const
{
    LinkVisualizerBase::refreshDisplay();
    auto simulation = getSimulation();
    for (auto it : linkVisualizations) {
        auto linkVisualization = it.second;
        auto linkCanvasVisualization = static_cast<const LinkCanvasVisualization *>(linkVisualization);
        auto figure = linkCanvasVisualization->figure;
        auto sourceModule = simulation->getModule(linkVisualization->sourceModuleId);
        auto destinationModule = simulation->getModule(linkVisualization->destinationModuleId);
        if (sourceModule != nullptr && destinationModule != nullptr) {
            auto sourcePosition = getContactPosition(sourceModule, getPosition(destinationModule), lineContactMode, lineContactSpacing);
            auto destinationPosition = getContactPosition(destinationModule, getPosition(sourceModule), lineContactMode, lineContactSpacing);
            auto shift = lineManager->getLineShift(linkVisualization->sourceModuleId, linkVisualization->destinationModuleId, sourcePosition, destinationPosition, lineShiftMode, linkVisualization->shiftOffset) * lineShift;
            figure->setStart(canvasProjection->computeCanvasPoint(sourcePosition + shift));
            figure->setEnd(canvasProjection->computeCanvasPoint(destinationPosition + shift));
        }
    }
    visualizationTargetModule->getCanvas()->setAnimationSpeed(linkVisualizations.empty() ? 0 : fadeOutAnimationSpeed, this);
}

const LinkVisualizerBase::LinkVisualization *LinkCanvasVisualizerBase::createLinkVisualization(cModule *source, cModule *destination, cPacket *packet) const
{
    auto figure = new LabeledLineFigure("link");
    auto lineFigure = figure->getLineFigure();
    lineFigure->setEndArrowhead(cFigure::ARROW_BARBED);
    lineFigure->setLineWidth(lineWidth);
    lineFigure->setLineColor(lineColor);
    lineFigure->setLineStyle(lineStyle);
    auto labelFigure = figure->getLabelFigure();
    labelFigure->setFont(labelFont);
    labelFigure->setColor(labelColor);
    auto text = getLinkVisualizationText(packet);
    labelFigure->setText(text.c_str());
    return new LinkCanvasVisualization(figure, source->getId(), destination->getId());
}

void LinkCanvasVisualizerBase::addLinkVisualization(std::pair<int, int> sourceAndDestination, const LinkVisualization *linkVisualization)
{
    LinkVisualizerBase::addLinkVisualization(sourceAndDestination, linkVisualization);
    auto linkCanvasVisualization = static_cast<const LinkCanvasVisualization *>(linkVisualization);
    auto figure = linkCanvasVisualization->figure;
    lineManager->addModuleLine(linkVisualization);
    linkGroup->addFigure(figure);
}

void LinkCanvasVisualizerBase::removeLinkVisualization(const LinkVisualization *linkVisualization)
{
    LinkVisualizerBase::removeLinkVisualization(linkVisualization);
    auto linkCanvasVisualization = static_cast<const LinkCanvasVisualization *>(linkVisualization);
    lineManager->removeModuleLine(linkVisualization);
    linkGroup->removeFigure(linkCanvasVisualization->figure);
}

void LinkCanvasVisualizerBase::setAlpha(const LinkVisualization *linkVisualization, double alpha) const
{
    auto linkCanvasVisualization = static_cast<const LinkCanvasVisualization *>(linkVisualization);
    auto figure = linkCanvasVisualization->figure;
    figure->getLineFigure()->setLineOpacity(alpha);
}

void LinkCanvasVisualizerBase::refreshLinkVisualization(const LinkVisualization *linkVisualization, cPacket *packet)
{
    LinkVisualizerBase::refreshLinkVisualization(linkVisualization, packet);
    auto linkCanvasVisualization = static_cast<const LinkCanvasVisualization *>(linkVisualization);
    auto text = getLinkVisualizationText(packet);
    linkCanvasVisualization->figure->getLabelFigure()->setText(text.c_str());
}

} // namespace visualizer

} // namespace inet

