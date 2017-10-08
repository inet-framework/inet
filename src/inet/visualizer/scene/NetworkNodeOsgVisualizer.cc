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

#include <omnetpp/osgutil.h>
#include "inet/common/ModuleAccess.h"
#include "inet/common/OSGScene.h"
#include "inet/common/OSGUtils.h"
#include "inet/visualizer/scene/NetworkNodeOsgVisualizer.h"

namespace inet {

namespace visualizer {

Define_Module(NetworkNodeOsgVisualizer);

#ifdef WITH_OSG

void NetworkNodeOsgVisualizer::initialize(int stage)
{
    NetworkNodeVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        displayModuleName = par("displayModuleName");
        auto scene = inet::osg::TopLevelScene::getSimulationScene(visualizerTargetModule);
        for (cModule::SubmoduleIterator it(getSystemModule()); !it.end(); it++) {
            auto networkNode = *it;
            if (isNetworkNode(networkNode) && nodeFilter.matches(networkNode)) {
                auto positionAttitudeTransform = createNetworkNodeVisualization(networkNode);
                setNetworkNodeVisualization(networkNode, positionAttitudeTransform);
                scene->addChild(positionAttitudeTransform);
            }
        }
    }
}

NetworkNodeOsgVisualization *NetworkNodeOsgVisualizer::createNetworkNodeVisualization(cModule *networkNode) const
{
    return new NetworkNodeOsgVisualization(networkNode, displayModuleName);
}

NetworkNodeOsgVisualization *NetworkNodeOsgVisualizer::getNetworkNodeVisualization(const cModule *networkNode) const
{
    auto it = networkNodeVisualizations.find(networkNode);
    return it == networkNodeVisualizations.end() ? nullptr : it->second;
}

void NetworkNodeOsgVisualizer::setNetworkNodeVisualization(const cModule *networkNode, NetworkNodeOsgVisualization *networkNodeVisualization)
{
    networkNodeVisualizations[networkNode] = networkNodeVisualization;
}

#endif // ifdef WITH_OSG

} // namespace visualizer

} // namespace inet

