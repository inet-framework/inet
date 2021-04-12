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

#include "inet/visualizer/base/GateVisualizerBase.h"

#include <algorithm>

namespace inet {

namespace visualizer {

VISIT_RETURNTYPE GateVisualizerBase::GateVisitor::visit(cObject *object)
{
    if (auto gate = dynamic_cast<queueing::IPacketGate *>(object))
        gates.push_back(gate);
    else
        object->forEachChild(this);
    VISIT_RETURN(true);
}

GateVisualizerBase::GateVisualization::GateVisualization(queueing::IPacketGate *gate) :
    gate(gate)
{
}

void GateVisualizerBase::preDelete(cComponent *root)
{
    if (displayGates)
        removeAllGateVisualizations();
}

void GateVisualizerBase::initialize(int stage)
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

void GateVisualizerBase::handleParameterChange(const char *name)
{
    if (!hasGUI()) return;
    if (name != nullptr) {
        if (!strcmp(name, "displayDuration"))
            displayDuration = par("displayDuration");
        removeAllGateVisualizations();
        addGateVisualizations();
    }
}

void GateVisualizerBase::refreshDisplay() const
{
    if (simTime() - lastRefreshTime > CLOCKTIME_AS_SIMTIME(displayDuration) / width) {
        for (auto gateVisualization : gateVisualizations)
            refreshGateVisualization(gateVisualization);
        lastRefreshTime = simTime();
    }
}

void GateVisualizerBase::addGateVisualization(const GateVisualization *gateVisualization)
{
    gateVisualizations.push_back(gateVisualization);
}

void GateVisualizerBase::removeGateVisualization(const GateVisualization *gateVisualization)
{
    gateVisualizations.erase(std::remove(gateVisualizations.begin(), gateVisualizations.end(), gateVisualization), gateVisualizations.end());
}

void GateVisualizerBase::addGateVisualizations()
{
    GateVisitor gateVisitor;
    visualizationSubjectModule->forEachChild(&gateVisitor);
    for (auto gate : gateVisitor.gates) {
        if (gateFilter.matches(gate))
            addGateVisualization(createGateVisualization(gate));
    }
}

void GateVisualizerBase::removeAllGateVisualizations()
{
    for (auto gateVisualization : std::vector<const GateVisualization *>(gateVisualizations)) {
        removeGateVisualization(gateVisualization);
        delete gateVisualization;
    }
}

} // namespace visualizer

} // namespace inet

