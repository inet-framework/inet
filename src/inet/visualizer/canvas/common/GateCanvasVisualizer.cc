//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "inet/visualizer/canvas/common/GateCanvasVisualizer.h"

#include "inet/common/ModuleAccess.h"
#include "inet/queueing/gate/PeriodicGate.h"

namespace inet {

namespace visualizer {

Define_Module(GateCanvasVisualizer);

GateCanvasVisualizer::GateCanvasVisualization::GateCanvasVisualization(NetworkNodeCanvasVisualization *networkNodeVisualization, GateFigure *figure, queueing::IPacketGate *gate) :
    GateVisualization(gate),
    networkNodeVisualization(networkNodeVisualization),
    figure(figure)
{
}

GateCanvasVisualizer::GateCanvasVisualization::~GateCanvasVisualization()
{
    delete figure;
}

void GateCanvasVisualizer::initialize(int stage)
{
    GateVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        zIndex = par("zIndex");
        networkNodeVisualizer.reference(this, "networkNodeVisualizerModule", true);
    }
}

GateVisualizerBase::GateVisualization *GateCanvasVisualizer::createGateVisualization(queueing::IPacketGate *gate) const
{
    auto module = check_and_cast<cModule *>(gate);
    auto figure = new GateFigure("gate");
    figure->setTags((std::string("gate ") + tags).c_str());
    figure->setTooltip("This figure represents a gate");
    figure->setAssociatedObject(module);
    figure->setSpacing(spacing);
    figure->setBounds(cFigure::Rectangle(0, 0, width, height));
    figure->setPosition(currentTimePosition);
    auto networkInterface = getContainingNicModule(module);
    auto label = std::string(networkInterface->getInterfaceName()) + "." + module->getFullName();
    figure->setLabel(label.c_str());
    auto networkNode = getContainingNode(module);
    auto networkNodeVisualization = networkNodeVisualizer->getNetworkNodeVisualization(networkNode);
    return new GateCanvasVisualization(networkNodeVisualization, figure, gate);
}

void GateCanvasVisualizer::addGateVisualization(const GateVisualization *gateVisualization)
{
    GateVisualizerBase::addGateVisualization(gateVisualization);
    auto gateCanvasVisualization = static_cast<const GateCanvasVisualization *>(gateVisualization);
    auto figure = gateCanvasVisualization->figure;
    gateCanvasVisualization->networkNodeVisualization->addAnnotation(figure, figure->getBounds().getSize(), placementHint, placementPriority);
}

void GateCanvasVisualizer::removeGateVisualization(const GateVisualization *gateVisualization)
{
    GateVisualizerBase::removeGateVisualization(gateVisualization);
    auto gateCanvasVisualization = static_cast<const GateCanvasVisualization *>(gateVisualization);
    auto figure = gateCanvasVisualization->figure;
    if (networkNodeVisualizer != nullptr)
        gateCanvasVisualization->networkNodeVisualization->removeAnnotation(figure);
}

void GateCanvasVisualizer::refreshGateVisualization(const GateVisualization *gateVisualization) const
{
    auto gateCanvasVisualization = static_cast<const GateCanvasVisualization *>(gateVisualization);
    auto gate = check_and_cast<queueing::PeriodicGate *>(gateVisualization->gate);
    auto figure = gateCanvasVisualization->figure;
    auto durations = gate->getDurations();
    bool open = gate->getInitiallyOpen();
    clocktime_t scheduleDuration = 0;
    for (int i = 0; i < durations->size(); i++)
        scheduleDuration += durations->get(i).doubleValueInUnit("s");
    if (scheduleDuration == 0)
        figure->addSchedule(0, width, open);
    else {
        auto displayDuration = this->displayDuration != 0 ? this->displayDuration : scheduleDuration;
        auto currentTime = gate->getClockTime();
        clocktime_t schedulePosition = std::fmod(currentTime.dbl(), scheduleDuration.dbl());
        auto scheduleDisplayStart = schedulePosition - (currentTimePosition / width) * displayDuration;
        auto scheduleDisplayEnd = scheduleDisplayStart + displayDuration;
        auto indexStart = (int)std::floor(scheduleDisplayStart.dbl() / scheduleDuration.dbl()) * durations->size();
        auto indexEnd = (int)std::ceil(scheduleDisplayEnd.dbl() / scheduleDuration.dbl()) * durations->size();
        clocktime_t displayTime = currentTime - schedulePosition + indexStart / durations->size() * scheduleDuration;
        figure->clearSchedule();
        for (int i = indexStart; i <= indexEnd; i++) {
            auto duration = durations->get((i + durations->size()) % durations->size()).doubleValueInUnit("s");
            clocktime_t startTime = displayTime - currentTime;
            clocktime_t endTime = displayTime + duration - currentTime;
            double factor = width / displayDuration.dbl();
            double start = std::max(0.0, std::min(width, currentTimePosition + startTime.dbl() * factor));
            double end = std::max(0.0, std::min(width, currentTimePosition + endTime.dbl() * factor));
            if (start != end)
                figure->addSchedule(start, end, open);
            displayTime += duration;
            open = !open;
        }
    }
}

} // namespace visualizer

} // namespace inet

