//
// Copyright (C) 2016 OpenSim Ltd.
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
#include "inet/visualizer/networknode/NetworkNodeCanvasVisualizer.h"

namespace inet {

namespace visualizer {

Define_Module(NetworkNodeCanvasVisualizer);

void NetworkNodeCanvasVisualizer::initialize(int stage)
{
    NetworkNodeVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        for (cModule::SubmoduleIterator it(getSystemModule()); !it.end(); it++) {
            auto networkNode = *it;
            if (isNetworkNode(networkNode) && networkNodePathMatcher.matches(networkNode->getFullPath().c_str())) {
                auto visualization = createNetworkNodeVisualization(networkNode);
                setNetworkNodeVisualization(networkNode, visualization);
                visualizerTargetModule->getCanvas()->addFigure(visualization);
            }
        }
    }
}

NetworkNodeCanvasVisualization *NetworkNodeCanvasVisualizer::createNetworkNodeVisualization(cModule *networkNode) const
{
    return new NetworkNodeCanvasVisualization(networkNode);
}

NetworkNodeCanvasVisualization *NetworkNodeCanvasVisualizer::getNeworkNodeVisualization(const cModule *networkNode) const
{
    auto it = networkNodeVisualizations.find(networkNode);
    return it == networkNodeVisualizations.end() ? nullptr : it->second;
}

void NetworkNodeCanvasVisualizer::setNetworkNodeVisualization(const cModule *networkNode, NetworkNodeCanvasVisualization *networkNodeVisualization)
{
    networkNodeVisualizations[networkNode] = networkNodeVisualization;
}

} // namespace visualizer

} // namespace inet

