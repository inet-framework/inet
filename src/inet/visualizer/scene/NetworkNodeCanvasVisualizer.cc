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
#include "inet/visualizer/scene/NetworkNodeCanvasVisualizer.h"

namespace inet {

namespace visualizer {

Define_Module(NetworkNodeCanvasVisualizer);

void NetworkNodeCanvasVisualizer::initialize(int stage)
{
    NetworkNodeVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        zIndex = par("zIndex");
        auto canvas = visualizationTargetModule->getCanvas();
        canvasProjection = CanvasProjection::getCanvasProjection(canvas);
        for (cModule::SubmoduleIterator it(visualizationSubjectModule); !it.end(); it++) {
            auto networkNode = *it;
            if (isNetworkNode(networkNode) && nodeFilter.matches(networkNode)) {
                auto visualization = createNetworkNodeVisualization(networkNode);
                addNetworkNodeVisualization(visualization);
            }
        }
    }
}

void NetworkNodeCanvasVisualizer::refreshDisplay() const
{
    for (auto it : networkNodeVisualizations) {
        auto networkNode = it.first;
        auto visualization = it.second;
        auto position = canvasProjection->computeCanvasPoint(getPosition(networkNode));
        visualization->setTransform(cFigure::Transform().translate(position.x, position.y));
        visualization->refreshDisplay();
    }
}

NetworkNodeCanvasVisualization *NetworkNodeCanvasVisualizer::createNetworkNodeVisualization(cModule *networkNode) const
{
    auto visualization = new NetworkNodeCanvasVisualization(networkNode, annotationSpacing, placementPenalty);
    visualization->setZIndex(zIndex);
    return visualization;
}

NetworkNodeCanvasVisualization *NetworkNodeCanvasVisualizer::getNetworkNodeVisualization(const cModule *networkNode) const
{
    auto it = networkNodeVisualizations.find(networkNode);
    return it == networkNodeVisualizations.end() ? nullptr : it->second;
}

void NetworkNodeCanvasVisualizer::addNetworkNodeVisualization(NetworkNodeVisualization *networkNodeVisualization)
{
    auto networkNodeCanvasVisualization = check_and_cast<NetworkNodeCanvasVisualization *>(networkNodeVisualization);
    networkNodeVisualizations[networkNodeCanvasVisualization->networkNode] = networkNodeCanvasVisualization;
    visualizationTargetModule->getCanvas()->addFigure(networkNodeCanvasVisualization);
}

void NetworkNodeCanvasVisualizer::removeNetworkNodeVisualization(NetworkNodeVisualization *networkNodeVisualization)
{
    auto networkNodeCanvasVisualization = check_and_cast<NetworkNodeCanvasVisualization *>(networkNodeVisualization);
    networkNodeVisualizations.erase(networkNodeVisualizations.find(networkNodeCanvasVisualization->networkNode));
    visualizationTargetModule->getCanvas()->removeFigure(networkNodeCanvasVisualization);
}

} // namespace visualizer

} // namespace inet

