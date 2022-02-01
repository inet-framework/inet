//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/canvas/common/PacketDropCanvasVisualizer.h"

#include "inet/common/ModuleAccess.h"

namespace inet {

namespace visualizer {

Define_Module(PacketDropCanvasVisualizer);

PacketDropCanvasVisualizer::PacketDropCanvasVisualization::PacketDropCanvasVisualization(LabeledIconFigure *figure, const PacketDrop *packetDrop) :
    PacketDropVisualization(packetDrop),
    figure(figure)
{
}

PacketDropCanvasVisualizer::PacketDropCanvasVisualization::~PacketDropCanvasVisualization()
{
    if (figure->getParentFigure() == nullptr)
        delete figure;
}

void PacketDropCanvasVisualizer::initialize(int stage)
{
    PacketDropVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        zIndex = par("zIndex");
        dx = par("dx");
        dy = par("dx");
        auto canvas = visualizationTargetModule->getCanvas();
        canvasProjection = CanvasProjection::getCanvasProjection(canvas);
        packetDropGroup = new cGroupFigure("packet drops");
        packetDropGroup->setZIndex(zIndex);
        canvas->addFigure(packetDropGroup);
    }
}

void PacketDropCanvasVisualizer::refreshDisplay() const
{
    PacketDropVisualizerBase::refreshDisplay();
    visualizationTargetModule->getCanvas()->setAnimationSpeed(packetDropVisualizations.empty() ? 0 : fadeOutAnimationSpeed, this);
}

const PacketDropVisualizerBase::PacketDropVisualization *PacketDropCanvasVisualizer::createPacketDropVisualization(PacketDrop *packetDrop) const
{
    std::string icon(this->icon);
    auto labeledIconFigure = new LabeledIconFigure("packetDropped");
    labeledIconFigure->setTags((std::string("packet_drop ") + tags).c_str());
    labeledIconFigure->setAssociatedObject(packetDrop);
    labeledIconFigure->setZIndex(zIndex);
    labeledIconFigure->setPosition(canvasProjection->computeCanvasPoint(packetDrop->getPosition()));
    auto iconFigure = labeledIconFigure->getIconFigure();
    iconFigure->setTooltip("This icon represents a packet dropped in a network node");
    iconFigure->setImageName(icon.substr(0, icon.find_first_of(".")).c_str());
    iconFigure->setTintColor(iconTintColorSet.getColor(packetDrop->getReason() % iconTintColorSet.getSize()));
    iconFigure->setTintAmount(iconTintAmount);
    auto labelFigure = labeledIconFigure->getLabelFigure();
    labelFigure->setTooltip("This label represents the name of a packet dropped in a network node");
    labelFigure->setFont(labelFont);
    labelFigure->setColor(labelColor);
    auto text = getPacketDropVisualizationText(packetDrop);
    labelFigure->setText(text.c_str());
    return new PacketDropCanvasVisualization(labeledIconFigure, packetDrop);
}

void PacketDropCanvasVisualizer::addPacketDropVisualization(const PacketDropVisualization *packetDropVisualization)
{
    PacketDropVisualizerBase::addPacketDropVisualization(packetDropVisualization);
    auto packetDropCanvasVisualization = static_cast<const PacketDropCanvasVisualization *>(packetDropVisualization);
    packetDropGroup->addFigure(packetDropCanvasVisualization->figure);
}

void PacketDropCanvasVisualizer::removePacketDropVisualization(const PacketDropVisualization *packetDropVisualization)
{
    PacketDropVisualizerBase::removePacketDropVisualization(packetDropVisualization);
    auto packetDropCanvasVisualization = static_cast<const PacketDropCanvasVisualization *>(packetDropVisualization);
    packetDropGroup->removeFigure(packetDropCanvasVisualization->figure);
}

void PacketDropCanvasVisualizer::setAlpha(const PacketDropVisualization *packetDropVisualization, double alpha) const
{
    auto packetDropCanvasVisualization = static_cast<const PacketDropCanvasVisualization *>(packetDropVisualization);
    auto figure = packetDropCanvasVisualization->figure;
    figure->setOpacity(alpha);
    double a = dy / -(dx * dx);
    double b = -2 * a * dx;
    double px = 4 * dx * (1 - alpha);
    double py = a * px * px + b * px;
    auto& position = packetDropVisualization->packetDrop->getPosition();
    double zoomLevel = getEnvir()->getZoomLevel(packetDropCanvasVisualization->packetDrop->getNetworkNode()->getParentModule());
    if (std::isnan(zoomLevel))
        zoomLevel = 1;
    figure->setPosition(canvasProjection->computeCanvasPoint(position) + cFigure::Point(px, -py) / zoomLevel);
}

} // namespace visualizer

} // namespace inet

