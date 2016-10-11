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
#include "inet/visualizer/transportlayer/TransportConnectionCanvasVisualizer.h"

namespace inet {

namespace visualizer {

Define_Module(TransportConnectionCanvasVisualizer);

TransportConnectionCanvasVisualizer::TransportConnectionCanvasVisualization::TransportConnectionCanvasVisualization(cFigure *sourceFigure, cFigure *destinationFigure, int sourceModuleId, int destinationModuleId, int count) :
    TransportConnectionVisualization(sourceModuleId, destinationModuleId, count),
    sourceFigure(sourceFigure),
    destinationFigure(destinationFigure)
{
}

void TransportConnectionCanvasVisualizer::initialize(int stage)
{
    TransportConnectionVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        zIndex = par("zIndex");
        auto canvas = visualizerTargetModule->getCanvas();
        canvasProjection = CanvasProjection::getCanvasProjection(canvas);
        networkNodeVisualizer = getModuleFromPar<NetworkNodeCanvasVisualizer>(par("networkNodeVisualizerModule"), this);
    }
}

cFigure *TransportConnectionCanvasVisualizer::createConnectionEndFigure(tcp::TCPConnection *tcpConnection) const
{
    auto figure = new cIconFigure();
    figure->setImageName("misc/marker");
    figure->setTintAmount(1);
    figure->setTintColor(cFigure::GOOD_DARK_COLORS[connections.size() % (sizeof(cFigure::GOOD_DARK_COLORS) / sizeof(cFigure::Color))]);
    figure->setAssociatedObject(tcpConnection);
    figure->setZIndex(zIndex);
    return figure;
}

const TransportConnectionVisualizerBase::TransportConnectionVisualization *TransportConnectionCanvasVisualizer::createConnectionVisualization(cModule *source, cModule *destination, tcp::TCPConnection *tcpConnection) const
{
    auto sourceFigure = createConnectionEndFigure(tcpConnection);
    auto destinationFigure = createConnectionEndFigure(tcpConnection);
    return new TransportConnectionCanvasVisualization(sourceFigure, destinationFigure, source->getId(), destination->getId(), 1);
}

void TransportConnectionCanvasVisualizer::addConnectionVisualization(const TransportConnectionVisualization *connection)
{
    TransportConnectionVisualizerBase::addConnectionVisualization(connection);
    auto canvasConnection = static_cast<const TransportConnectionCanvasVisualization *>(connection);
    auto sourceModule = getSimulation()->getModule(connection->sourceModuleId);
    auto sourceVisualization = networkNodeVisualizer->getNeworkNodeVisualization(getContainingNode(sourceModule));
    sourceVisualization->addAnnotation(canvasConnection->sourceFigure, cFigure::Point(0, 8));
    auto destinationModule = getSimulation()->getModule(connection->destinationModuleId);
    auto destinationVisualization = networkNodeVisualizer->getNeworkNodeVisualization(getContainingNode(destinationModule));
    destinationVisualization->addAnnotation(canvasConnection->destinationFigure, cFigure::Point(0, 8));
}

void TransportConnectionCanvasVisualizer::removeConnectionVisualization(const TransportConnectionVisualization *connection)
{
    TransportConnectionVisualizerBase::removeConnectionVisualization(connection);
    auto canvasConnection = static_cast<const TransportConnectionCanvasVisualization *>(connection);
    auto sourceModule = getSimulation()->getModule(connection->sourceModuleId);
    auto sourceVisualization = networkNodeVisualizer->getNeworkNodeVisualization(getContainingNode(sourceModule));
    sourceVisualization->removeAnnotation(canvasConnection->sourceFigure);
    auto destinationModule = getSimulation()->getModule(connection->destinationModuleId);
    auto destinationVisualization = networkNodeVisualizer->getNeworkNodeVisualization(getContainingNode(destinationModule));
    destinationVisualization->removeAnnotation(canvasConnection->destinationFigure);
}

} // namespace visualizer

} // namespace inet

