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

TransportConnectionCanvasVisualizer::TransportConnectionCanvasVisualization::TransportConnectionCanvasVisualization(cIconFigure *sourceFigure, cIconFigure *destinationFigure, int sourceModuleId, int destinationModuleId, int count) :
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

cIconFigure *TransportConnectionCanvasVisualizer::createConnectionEndFigure(tcp::TCPConnection *tcpConnection) const
{
    auto figure = new cIconFigure();
    figure->setTags("transport_connection");
    figure->setTooltip("This icon represents a transport connection between two network nodes");
    figure->setAssociatedObject(tcpConnection);
    figure->setZIndex(zIndex);
    figure->setAnchor(cFigure::ANCHOR_NW);
    figure->setImageName(icon);
    figure->setTintAmount(1);
    figure->setTintColor(cFigure::GOOD_DARK_COLORS[connectionVisualizations.size() % (sizeof(cFigure::GOOD_DARK_COLORS) / sizeof(cFigure::Color))]);
    return figure;
}

const TransportConnectionVisualizerBase::TransportConnectionVisualization *TransportConnectionCanvasVisualizer::createConnectionVisualization(cModule *source, cModule *destination, tcp::TCPConnection *tcpConnection) const
{
    auto sourceFigure = createConnectionEndFigure(tcpConnection);
    auto destinationFigure = createConnectionEndFigure(tcpConnection);
    return new TransportConnectionCanvasVisualization(sourceFigure, destinationFigure, source->getId(), destination->getId(), 1);
}

void TransportConnectionCanvasVisualizer::addConnectionVisualization(const TransportConnectionVisualization *connectionVisualization)
{
    TransportConnectionVisualizerBase::addConnectionVisualization(connectionVisualization);
    auto connectionCanvasVisualization = static_cast<const TransportConnectionCanvasVisualization *>(connectionVisualization);
    auto sourceModule = getSimulation()->getModule(connectionVisualization->sourceModuleId);
    auto sourceVisualization = networkNodeVisualizer->getNeworkNodeVisualization(getContainingNode(sourceModule));
    sourceVisualization->addAnnotation(connectionCanvasVisualization->sourceFigure, connectionCanvasVisualization->sourceFigure->getBounds().getSize());
    auto destinationModule = getSimulation()->getModule(connectionVisualization->destinationModuleId);
    auto destinationVisualization = networkNodeVisualizer->getNeworkNodeVisualization(getContainingNode(destinationModule));
    destinationVisualization->addAnnotation(connectionCanvasVisualization->destinationFigure, connectionCanvasVisualization->destinationFigure->getBounds().getSize());
}

void TransportConnectionCanvasVisualizer::removeConnectionVisualization(const TransportConnectionVisualization *connectionVisualization)
{
    TransportConnectionVisualizerBase::removeConnectionVisualization(connectionVisualization);
    auto connectionCanvasVisualization = static_cast<const TransportConnectionCanvasVisualization *>(connectionVisualization);
    auto sourceModule = getSimulation()->getModule(connectionVisualization->sourceModuleId);
    auto sourceVisualization = networkNodeVisualizer->getNeworkNodeVisualization(getContainingNode(sourceModule));
    sourceVisualization->removeAnnotation(connectionCanvasVisualization->sourceFigure);
    auto destinationModule = getSimulation()->getModule(connectionVisualization->destinationModuleId);
    auto destinationVisualization = networkNodeVisualizer->getNeworkNodeVisualization(getContainingNode(destinationModule));
    destinationVisualization->removeAnnotation(connectionCanvasVisualization->destinationFigure);
}

} // namespace visualizer

} // namespace inet

