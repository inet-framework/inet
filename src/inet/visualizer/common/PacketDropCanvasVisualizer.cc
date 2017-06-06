//
// Copyright (C) OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/common/ModuleAccess.h"
#include "inet/visualizer/common/PacketDropCanvasVisualizer.h"

namespace inet {

namespace visualizer {

Define_Module(PacketDropCanvasVisualizer);

PacketDropCanvasVisualizer::PacketDropCanvasVisualization::PacketDropCanvasVisualization(LabeledIconFigure *figure, int moduleId, const cPacket *packet, const Coord& position) :
    PacketDropVisualization(moduleId, packet, position),
    figure(figure)
{
}

void PacketDropCanvasVisualizer::initialize(int stage)
{
    PacketDropVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        zIndex = par("zIndex");
        auto canvas = visualizerTargetModule->getCanvas();
        canvasProjection = CanvasProjection::getCanvasProjection(canvas);
        packetDropGroup = new cGroupFigure("packet drops");
        packetDropGroup->setZIndex(zIndex);
        canvas->addFigure(packetDropGroup);
    }
}

void PacketDropCanvasVisualizer::refreshDisplay() const
{
    PacketDropVisualizerBase::refreshDisplay();
    visualizerTargetModule->getCanvas()->setAnimationSpeed(packetDropVisualizations.empty() ? 0 : fadeOutAnimationSpeed, this);
}

const PacketDropVisualizerBase::PacketDropVisualization *PacketDropCanvasVisualizer::createPacketDropVisualization(cModule *module, cPacket *packet) const
{
    std::string icon(this->icon);
    auto position = getPosition(getContainingNode(module));
    auto labeledIconFigure = new LabeledIconFigure("packetDrop");
    labeledIconFigure->setTags((std::string("packet_drop ") + tags).c_str());
    labeledIconFigure->setAssociatedObject(packet);
    labeledIconFigure->setZIndex(zIndex);
    labeledIconFigure->setPosition(canvasProjection->computeCanvasPoint(position));
    auto iconFigure = labeledIconFigure->getIconFigure();
    iconFigure->setTooltip("This icon represents a packet dropped in a network node");
    iconFigure->setImageName(icon.substr(0, icon.find_first_of(".")).c_str());
    iconFigure->setTintColor(iconTintColor);
    iconFigure->setTintAmount(iconTintAmount);
    auto labelFigure = labeledIconFigure->getLabelFigure();
    labelFigure->setTooltip("This label represents the name of a packet dropped in a network node");
    labelFigure->setFont(labelFont);
    labelFigure->setColor(labelColor);
    labelFigure->setText(packet->getName());
    return new PacketDropCanvasVisualization(labeledIconFigure, module->getId(), packet, position);
}

void PacketDropCanvasVisualizer::addPacketDropVisualization(const PacketDropVisualization *packetDrop)
{
    PacketDropVisualizerBase::addPacketDropVisualization(packetDrop);
    auto packetDropCanvasVisualization = static_cast<const PacketDropCanvasVisualization *>(packetDrop);
    packetDropGroup->addFigure(packetDropCanvasVisualization->figure);
}

void PacketDropCanvasVisualizer::removePacketDropVisualization(const PacketDropVisualization *packetDrop)
{
    PacketDropVisualizerBase::removePacketDropVisualization(packetDrop);
    auto packetDropCanvasVisualization = static_cast<const PacketDropCanvasVisualization *>(packetDrop);
    packetDropGroup->removeFigure(packetDropCanvasVisualization->figure);
}

void PacketDropCanvasVisualizer::setAlpha(const PacketDropVisualization *packetDrop, double alpha) const
{
    auto packetDropCanvasVisualization = static_cast<const PacketDropCanvasVisualization *>(packetDrop);
    auto figure = packetDropCanvasVisualization->figure;
    figure->setOpacity(alpha);
    double dx = 10 / alpha;
    double dy = pow((dx / 4 - 9), 2) - 58;
    figure->setPosition(canvasProjection->computeCanvasPoint(packetDrop->position) + cFigure::Point(dx, dy));
}

} // namespace visualizer

} // namespace inet

