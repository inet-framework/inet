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
#include "inet/common/scenario/ScenarioManager.h"
#include "inet/visualizer/base/NetworkNodeVisualizerBase.h"

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
    if (name != nullptr) {
        if (!strcmp(name, "nodeFilter"))
            nodeFilter.setPattern(par("nodeFilter"));
        // TODO:
    }
}

void NetworkNodeVisualizerBase::receiveSignal(cComponent *source, simsignal_t signal, cObject *object, cObject *details)
{
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
                delete visualization;
            }
        }
    }
    else
        throw cRuntimeError("Unknown signal");
}

} // namespace visualizer

} // namespace inet

