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

PacketDropCanvasVisualizer::CanvasPacketDrop::CanvasPacketDrop(cIconFigure *figure, int moduleId, cPacket *packet, simtime_t dropSimulationTime, double dropAnimationTime, int dropRealTime) :
    PacketDropVisualization(moduleId, packet, dropSimulationTime, dropAnimationTime, dropRealTime),
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
        packetDropGroup = new cGroupFigure();
        packetDropGroup->setZIndex(zIndex);
        canvas->addFigure(packetDropGroup);
    }
}

void PacketDropCanvasVisualizer::setAlpha(const PacketDropVisualization *packetDrop, double alpha) const
{
    auto canvasPacketDrop = static_cast<const CanvasPacketDrop *>(packetDrop);
    auto figure = canvasPacketDrop->figure;
    figure->setOpacity(alpha);
    auto position = getPosition(getContainingNode(getSimulation()->getModule(packetDrop->moduleId)));
    double dx = 10 / alpha;
    double dy = -16 + pow((dx / 4 - 9), 2) - 42;
    figure->setPosition(canvasProjection->computeCanvasPoint(position) + cFigure::Point(dx, dy));
}

const PacketDropVisualizerBase::PacketDropVisualization *PacketDropCanvasVisualizer::createPacketDropVisualization(cModule *module, cPacket *packet) const
{
    auto figure = new cIconFigure();
    figure->setImageName(icon);
    figure->setTintAmount(iconTintAmount);
    figure->setTintColor(iconTintColor);
    figure->setPosition(canvasProjection->computeCanvasPoint(getPosition(getContainingNode(module))));
    return new CanvasPacketDrop(figure, module->getId(), packet, getSimulation()->getSimTime(), getSimulation()->getEnvir()->getAnimationTime(), getRealTime());
}

void PacketDropCanvasVisualizer::addPacketDropVisualization(const PacketDropVisualization *packetDrop)
{
    PacketDropVisualizerBase::addPacketDropVisualization(packetDrop);
    auto canvasPacketDrop = static_cast<const CanvasPacketDrop *>(packetDrop);
    packetDropGroup->addFigure(canvasPacketDrop->figure);
}

void PacketDropCanvasVisualizer::removePacketDropVisualization(const PacketDropVisualization *packetDrop)
{
    PacketDropVisualizerBase::removePacketDropVisualization(packetDrop);
    auto canvasPacketDrop = static_cast<const CanvasPacketDrop *>(packetDrop);
    packetDropGroup->removeFigure(canvasPacketDrop->figure);
}

} // namespace visualizer

} // namespace inet

