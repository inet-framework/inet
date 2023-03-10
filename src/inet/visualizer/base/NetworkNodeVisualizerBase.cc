//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/base/NetworkNodeVisualizerBase.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/scenario/ScenarioManager.h"

namespace inet {

namespace visualizer {

void NetworkNodeVisualizerBase::initialize(int stage)
{
    VisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        nodeFilter.setPattern(par("nodeFilter"));
        annotationSpacing = par("annotationSpacing");
        placementPenalty = par("placementPenalty");
        visualizationSubjectModule->subscribe(POST_MODEL_CHANGE, this);
        visualizationSubjectModule->subscribe(PRE_MODEL_CHANGE, this);
    }
}

void NetworkNodeVisualizerBase::handleParameterChange(const char *name)
{
    if (!hasGUI()) return;
    if (!strcmp(name, "nodeFilter"))
        nodeFilter.setPattern(par("nodeFilter"));
    // TODO
}

NetworkNodeVisualizerBase::NetworkNodeVisualization *NetworkNodeVisualizerBase::getNetworkNodeVisualization(const cModule *networkNode) const
{
    auto visualization = findNetworkNodeVisualization(networkNode);
    if (visualization == nullptr)
        throw cRuntimeError("Network node visualization is not found for '%s'", networkNode->getFullPath().c_str());
    else
        return visualization;
}

void NetworkNodeVisualizerBase::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signal));

    if (signal == POST_MODEL_CHANGE) {
        if (auto moduleInit = dynamic_cast<cPreModuleInitNotification *>(object)) {
            auto module = moduleInit->module;
            if (isNetworkNode(module) && nodeFilter.matches(module)) {
                auto visualization = createNetworkNodeVisualization(module);
                addNetworkNodeVisualization(visualization);
            }
        }
    }
    else if (signal == PRE_MODEL_CHANGE) {
        if (auto moduleDelete = dynamic_cast<cPreModuleDeleteNotification *>(object)) {
            auto module = moduleDelete->module;
            if (isNetworkNode(module) && nodeFilter.matches(module)) {
                auto visualization = getNetworkNodeVisualization(module);
                removeNetworkNodeVisualization(visualization);
                destroyNetworkNodeVisualization(visualization);
            }
        }
    }
    else
        throw cRuntimeError("Unknown signal");
}

} // namespace visualizer

} // namespace inet

