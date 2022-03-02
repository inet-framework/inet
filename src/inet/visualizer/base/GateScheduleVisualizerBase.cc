//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/base/GateScheduleVisualizerBase.h"

#include <algorithm>

namespace inet {

namespace visualizer {

bool GateScheduleVisualizerBase::GateVisitor::visit(cObject *object)
{
    if (auto gate = dynamic_cast<queueing::IPacketGate *>(object))
        gates.push_back(gate);
    else
        object->forEachChild(this);
    return true;
}

GateScheduleVisualizerBase::GateVisualization::GateVisualization(queueing::IPacketGate *gate) :
    gate(gate)
{
}

void GateScheduleVisualizerBase::preDelete(cComponent *root)
{
    if (displayGates)
        removeAllGateVisualizations();
}

void GateScheduleVisualizerBase::initialize(int stage)
{
    VisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        displayGates = par("displayGates");
        gateFilter.setPattern(par("gateFilter"));
        width = par("width");
        height = par("height");
        spacing = par("spacing");
        placementHint = parsePlacement(par("placementHint"));
        placementPriority = par("placementPriority");
        displayDuration = par("displayDuration");
        currentTimePosition = par("currentTimePosition");
    }
    else if (stage == INITSTAGE_LAST) {
        if (displayGates)
            addGateVisualizations();
    }
}

void GateScheduleVisualizerBase::handleParameterChange(const char *name)
{
    if (!hasGUI()) return;
    if (name != nullptr) {
        if (!strcmp(name, "displayDuration"))
            displayDuration = par("displayDuration");
        removeAllGateVisualizations();
        addGateVisualizations();
    }
}

void GateScheduleVisualizerBase::refreshDisplay() const
{
    if (simTime() - lastRefreshTime > CLOCKTIME_AS_SIMTIME(displayDuration) / width) {
        for (auto gateVisualization : gateVisualizations)
            refreshGateVisualization(gateVisualization);
        lastRefreshTime = simTime();
    }
}

void GateScheduleVisualizerBase::addGateVisualization(const GateVisualization *gateVisualization)
{
    gateVisualizations.push_back(gateVisualization);
}

void GateScheduleVisualizerBase::removeGateVisualization(const GateVisualization *gateVisualization)
{
    gateVisualizations.erase(std::remove(gateVisualizations.begin(), gateVisualizations.end(), gateVisualization), gateVisualizations.end());
}

void GateScheduleVisualizerBase::addGateVisualizations()
{
    GateVisitor gateVisitor;
    visualizationSubjectModule->forEachChild(&gateVisitor);
    for (auto gate : gateVisitor.gates) {
        if (gateFilter.matches(gate))
            addGateVisualization(createGateVisualization(gate));
    }
}

void GateScheduleVisualizerBase::removeAllGateVisualizations()
{
    for (auto gateVisualization : std::vector<const GateVisualization *>(gateVisualizations)) {
        removeGateVisualization(gateVisualization);
        delete gateVisualization;
    }
}

} // namespace visualizer

} // namespace inet

