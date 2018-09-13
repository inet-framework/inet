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
#include "inet/visualizer/base/NetworkConnectionVisualizerBase.h"

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
    if (name != nullptr) {
        if (!strcmp(name, "nodeFilter"))
            nodeFilter.setPattern(par("nodeFilter"));
        // TODO:
    }
}

} // namespace visualizer

} // namespace inet

