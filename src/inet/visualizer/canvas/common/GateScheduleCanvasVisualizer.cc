//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/canvas/common/GateScheduleCanvasVisualizer.h"

#include "inet/common/ModuleAccess.h"
#include "inet/queueing/gate/PeriodicGate.h"

namespace inet {

namespace visualizer {

Define_Module(GateScheduleCanvasVisualizer);

GateScheduleCanvasVisualizer::GateCanvasVisualization::GateCanvasVisualization(NetworkNodeCanvasVisualization *networkNodeVisualization, GateFigure *figure, queueing::IPacketGate *gate) :
    GateVisualization(gate),
    networkNodeVisualization(networkNodeVisualization),
    figure(figure)
{
}

GateScheduleCanvasVisualizer::GateCanvasVisualization::~GateCanvasVisualization()
{
    delete figure;
}

void GateScheduleCanvasVisualizer::initialize(int stage)
{
    GateScheduleVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        zIndex = par("zIndex");
        networkNodeVisualizer.reference(this, "networkNodeVisualizerModule", true);
        stringFormat.parseFormat(par("labelFormat"));
    }
}

GateScheduleVisualizerBase::GateVisualization *GateScheduleCanvasVisualizer::createGateVisualization(queueing::IPacketGate *gate) const
{
    auto module = check_and_cast<cModule *>(gate);
    auto figure = new GateFigure("gate");
    figure->setTags((std::string("gate ") + tags).c_str());
    figure->setTooltip("This figure represents a gate");
    figure->setAssociatedObject(module);
    figure->setSpacing(spacing);
    figure->setBounds(cFigure::Rectangle(0, 0, width, height));
    figure->setPosition(currentTimePosition);
    figure->setLabel(getGateScheduleVisualizationText(module).c_str());
    auto networkNode = getContainingNode(module);
    auto networkNodeVisualization = networkNodeVisualizer->getNetworkNodeVisualization(networkNode);
    return new GateCanvasVisualization(networkNodeVisualization, figure, gate);
}

void GateScheduleCanvasVisualizer::addGateVisualization(const GateVisualization *gateVisualization)
{
    GateScheduleVisualizerBase::addGateVisualization(gateVisualization);
    auto gateCanvasVisualization = static_cast<const GateCanvasVisualization *>(gateVisualization);
    auto figure = gateCanvasVisualization->figure;
    gateCanvasVisualization->networkNodeVisualization->addAnnotation(figure, figure->getBounds().getSize(), placementHint, placementPriority);
}

void GateScheduleCanvasVisualizer::removeGateVisualization(const GateVisualization *gateVisualization)
{
    GateScheduleVisualizerBase::removeGateVisualization(gateVisualization);
    auto gateCanvasVisualization = static_cast<const GateCanvasVisualization *>(gateVisualization);
    auto figure = gateCanvasVisualization->figure;
    if (networkNodeVisualizer != nullptr)
        gateCanvasVisualization->networkNodeVisualization->removeAnnotation(figure);
}

void GateScheduleCanvasVisualizer::refreshGateVisualization(const GateVisualization *gateVisualization) const
{
    auto gateCanvasVisualization = static_cast<const GateCanvasVisualization *>(gateVisualization);
    auto gate = check_and_cast<queueing::PeriodicGate *>(gateVisualization->gate);
    auto figure = gateCanvasVisualization->figure;
    auto durations = gate->getDurations();
    bool open = gate->getInitiallyOpen();
    clocktime_t scheduleDuration = 0;
    int numDurations = (int)durations.size();  // make it signed, to avoid mixed-sign multiplications in the code below
    for (int i = 0; i < numDurations; i++)
        scheduleDuration += durations[i];
    if (scheduleDuration == 0)
        figure->addSchedule(0, width, open);
    else {
        auto displayDuration = this->displayDuration != 0 ? this->displayDuration : scheduleDuration;
        auto currentTime = gate->getClockTime();
        clocktime_t schedulePosition = std::fmod((currentTime + gate->getInitialOffset()).dbl(), scheduleDuration.dbl());
        auto scheduleDisplayStart = schedulePosition - (currentTimePosition / width) * displayDuration;
        auto scheduleDisplayEnd = scheduleDisplayStart + displayDuration;
        int indexStart = (int)std::floor(scheduleDisplayStart.dbl() / scheduleDuration.dbl()) * numDurations;
        int indexEnd = (int)std::ceil(scheduleDisplayEnd.dbl() / scheduleDuration.dbl()) * numDurations;
        clocktime_t displayTime = currentTime - schedulePosition + indexStart / numDurations * scheduleDuration;
        figure->clearSchedule();
        for (int i = indexStart; i <= indexEnd; i++) {
            auto duration = durations[(i + numDurations) % numDurations];
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

