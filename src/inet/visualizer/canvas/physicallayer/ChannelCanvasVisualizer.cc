//
// Copyright (C) 2024 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/canvas/physicallayer/ChannelCanvasVisualizer.h"

namespace inet {

namespace visualizer {

Define_Module(ChannelCanvasVisualizer);

ChannelCanvasVisualizer::ChannelCanvasVisualization::ChannelCanvasVisualization(LabeledLineFigure *figure, int sourceModuleId, int destinationModuleId) :
    ChannelVisualization(sourceModuleId, destinationModuleId),
    figure(figure)
{
}

ChannelCanvasVisualizer::ChannelCanvasVisualization::~ChannelCanvasVisualization()
{
    delete figure;
}

void ChannelCanvasVisualizer::initialize(int stage)
{
    ChannelVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        zIndex = par("zIndex");
        auto canvas = visualizationTargetModule->getCanvas();
        lineManager = LineManager::getCanvasLineManager(canvas);
        canvasProjection = CanvasProjection::getCanvasProjection(canvas);
        channelActivityGroup = new cGroupFigure("channels");
        channelActivityGroup->setZIndex(zIndex);
        canvas->addFigure(channelActivityGroup);
    }
}

void ChannelCanvasVisualizer::refreshDisplay() const
{
    ChannelVisualizerBase::refreshDisplay();
    auto simulation = getSimulation();
    for (auto it : channelVisualizations) {
        auto channelVisualization = it.second;
        auto channelCanvasVisualization = static_cast<const ChannelCanvasVisualization *>(channelVisualization);
        auto figure = channelCanvasVisualization->figure;
        auto sourceModule = simulation->getModule(channelVisualization->sourceModuleId);
        auto destinationModule = simulation->getModule(channelVisualization->destinationModuleId);
        if (sourceModule != nullptr && destinationModule != nullptr) {
            auto sourcePosition = getContactPosition(sourceModule, getPosition(destinationModule), lineContactMode, lineContactSpacing);
            auto destinationPosition = getContactPosition(destinationModule, getPosition(sourceModule), lineContactMode, lineContactSpacing);
            auto shift = lineManager->getLineShift(channelVisualization->sourceModuleId, channelVisualization->destinationModuleId, sourcePosition, destinationPosition, lineShiftMode, channelVisualization->shiftOffset) * lineShift;
            figure->setStart(canvasProjection->computeCanvasPoint(sourcePosition + shift));
            figure->setEnd(canvasProjection->computeCanvasPoint(destinationPosition + shift));
        }
    }
    visualizationTargetModule->getCanvas()->setAnimationSpeed(channelVisualizations.empty() ? 0 : fadeOutAnimationSpeed, this);
}

const ChannelVisualizerBase::ChannelVisualization *ChannelCanvasVisualizer::createChannelVisualization(cModule *source, cModule *destination, cPacket *packet) const
{
    auto figure = new LabeledLineFigure("channel activity");
    figure->setTags((std::string("channel ") + tags).c_str());
    figure->setTooltip("This arrow represents channel activity between two network nodes");
    auto lineFigure = figure->getLineFigure();
    lineFigure->setEndArrowhead(cFigure::ARROW_BARBED);
    lineFigure->setLineWidth(lineWidth);
    lineFigure->setLineColor(lineColor);
    lineFigure->setLineStyle(lineStyle);
    auto labelFigure = figure->getLabelFigure();
    labelFigure->setFont(labelFont);
    labelFigure->setColor(labelColor);
    auto text = getChannelVisualizationText(packet);
    labelFigure->setText(text.c_str());
    return new ChannelCanvasVisualization(figure, source->getId(), destination->getId());
}

void ChannelCanvasVisualizer::addChannelVisualization(std::pair<int, int> sourceAndDestination, const ChannelVisualization *channelVisualization)
{
    ChannelVisualizerBase::addChannelVisualization(sourceAndDestination, channelVisualization);
    auto channelCanvasVisualization = static_cast<const ChannelCanvasVisualization *>(channelVisualization);
    auto figure = channelCanvasVisualization->figure;
    lineManager->addModuleLine(channelVisualization);
    channelActivityGroup->addFigure(figure);
}

void ChannelCanvasVisualizer::removeChannelVisualization(const ChannelVisualization *channelVisualization)
{
    ChannelVisualizerBase::removeChannelVisualization(channelVisualization);
    auto channelCanvasVisualization = static_cast<const ChannelCanvasVisualization *>(channelVisualization);
    lineManager->removeModuleLine(channelVisualization);
    channelActivityGroup->removeFigure(channelCanvasVisualization->figure);
}

void ChannelCanvasVisualizer::setAlpha(const ChannelVisualization *channelVisualization, double alpha) const
{
    auto channelCanvasVisualization = static_cast<const ChannelCanvasVisualization *>(channelVisualization);
    auto figure = channelCanvasVisualization->figure;
    figure->getLineFigure()->setLineOpacity(alpha);
}

void ChannelCanvasVisualizer::refreshChannelVisualization(const ChannelVisualization *channelVisualization, cPacket *packet)
{
    ChannelVisualizerBase::refreshChannelVisualization(channelVisualization, packet);
    auto channelCanvasVisualization = static_cast<const ChannelCanvasVisualization *>(channelVisualization);
    auto text = getChannelVisualizationText(packet);
    channelCanvasVisualization->figure->getLabelFigure()->setText(text.c_str());
}

} // namespace visualizer

} // namespace inet

