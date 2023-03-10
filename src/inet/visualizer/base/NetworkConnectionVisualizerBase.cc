//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/base/NetworkConnectionVisualizerBase.h"

#include "inet/common/ModuleAccess.h"

namespace inet {

namespace visualizer {

void NetworkConnectionVisualizerBase::initialize(int stage)
{
    VisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        nodeFilter.setPattern(par("nodeFilter"));
        lineColor = cFigure::Color(par("lineColor"));
        lineStyle = cFigure::parseLineStyle(par("lineStyle"));
        lineWidth = par("lineWidth");
    }
    else if (stage == INITSTAGE_LAST) {
        for (cModule::SubmoduleIterator it(visualizationSubjectModule); !it.end(); it++) {
            auto networkNode = *it;
            if (isNetworkNode(networkNode) && nodeFilter.matches(networkNode)) {
                for (cModule::GateIterator gt(networkNode); !gt.end(); gt++) {
                    auto gate = *gt;
                    auto startNetworkNode = getContainingNode(gate->getPathStartGate()->getOwnerModule());
                    auto endNetworkNode = getContainingNode(gate->getPathEndGate()->getOwnerModule());
                    createNetworkConnectionVisualization(startNetworkNode, endNetworkNode);
                }
            }
        }
    }
}

void NetworkConnectionVisualizerBase::handleParameterChange(const char *name)
{
    if (!hasGUI()) return;
    if (!strcmp(name, "nodeFilter"))
        nodeFilter.setPattern(par("nodeFilter"));
    // TODO
}

} // namespace visualizer

} // namespace inet

