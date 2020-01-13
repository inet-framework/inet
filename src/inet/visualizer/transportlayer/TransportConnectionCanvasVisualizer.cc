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

TransportConnectionCanvasVisualizer::TransportConnectionCanvasVisualization::TransportConnectionCanvasVisualization(LabeledIconFigure *sourceFigure, LabeledIconFigure *destinationFigure, int sourceModuleId, int destinationModuleId, int count) :
    TransportConnectionVisualization(sourceModuleId, destinationModuleId, count),
    sourceFigure(sourceFigure),
    destinationFigure(destinationFigure)
{
}

TransportConnectionCanvasVisualizer::~TransportConnectionCanvasVisualizer()
{
    if (displayTransportConnections)
        removeAllConnectionVisualizations();
}

void TransportConnectionCanvasVisualizer::initialize(int stage)
{
    TransportConnectionVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        zIndex = par("zIndex");
        auto canvas = visualizationTargetModule->getCanvas();
        canvasProjection = CanvasProjection::getCanvasProjection(canvas);
        networkNodeVisualizer = getModuleFromPar<NetworkNodeCanvasVisualizer>(par("networkNodeVisualizerModule"), this);
    }
}

LabeledIconFigure *TransportConnectionCanvasVisualizer::createConnectionEndFigure(tcp::TcpConnection *tcpConnection) const
{
#ifdef WITH_TCP_INET
    std::string icon(this->icon);
    auto labeledIconFigure = new LabeledIconFigure("transportConnection");
    labeledIconFigure->setTags((std::string("transport_connection ") + tags).c_str());
    labeledIconFigure->setTooltip("This icon represents a transport connection between two network nodes");
    labeledIconFigure->setAssociatedObject(tcpConnection);
    labeledIconFigure->setZIndex(zIndex);
    auto iconFigure = labeledIconFigure->getIconFigure();
    iconFigure->setTooltip("This icon represents a transport connection between two network nodes");
    iconFigure->setImageName(icon.substr(0, icon.find_first_of(".")).c_str());
    iconFigure->setTintColor(iconColorSet.getColor(connectionVisualizations.size()));
    iconFigure->setTintAmount(1);
    auto labelFigure = labeledIconFigure->getLabelFigure();
    labelFigure->setTooltip("This label represents a transport connection between two network nodes");
    labelFigure->setPosition(iconFigure->getBounds().getSize() / 2);
    labelFigure->setFont(labelFont);
    labelFigure->setColor(labelColor);
    char label[2];
    label[0] = 'A' + (char)(connectionVisualizations.size() / iconColorSet.getSize());
    label[1] = '\0';
    labelFigure->setText(label);
    return labeledIconFigure;
#else
    return nullptr;
#endif // WITH_TCP_INET
}

const TransportConnectionVisualizerBase::TransportConnectionVisualization *TransportConnectionCanvasVisualizer::createConnectionVisualization(cModule *source, cModule *destination, tcp::TcpConnection *tcpConnection) const
{
    auto sourceFigure = createConnectionEndFigure(tcpConnection);
    auto destinationFigure = createConnectionEndFigure(tcpConnection);
    auto sourceNetworkNode = getContainingNode(source);
    auto sourceVisualization = networkNodeVisualizer->getNetworkNodeVisualization(sourceNetworkNode);
    if (sourceVisualization == nullptr)
        throw cRuntimeError("Cannot create transport connection visualization for '%s', because network node visualization is not found for '%s'", source->getFullPath().c_str(), sourceNetworkNode->getFullPath().c_str());
    auto destinationNetworkNode = getContainingNode(destination);
    auto destinationVisualization = networkNodeVisualizer->getNetworkNodeVisualization(destinationNetworkNode);
    if (destinationVisualization == nullptr)
        throw cRuntimeError("Cannot create transport connection visualization for '%s', because network node visualization is not found for '%s'", source->getFullPath().c_str(), destinationNetworkNode->getFullPath().c_str());
    return new TransportConnectionCanvasVisualization(sourceFigure, destinationFigure, source->getId(), destination->getId(), 1);
}

void TransportConnectionCanvasVisualizer::addConnectionVisualization(const TransportConnectionVisualization *connectionVisualization)
{
    TransportConnectionVisualizerBase::addConnectionVisualization(connectionVisualization);
    auto connectionCanvasVisualization = static_cast<const TransportConnectionCanvasVisualization *>(connectionVisualization);
    auto sourceModule = getSimulation()->getModule(connectionVisualization->sourceModuleId);
    if (sourceModule != nullptr) {
        auto sourceVisualization = networkNodeVisualizer->getNetworkNodeVisualization(getContainingNode(sourceModule));
        sourceVisualization->addAnnotation(connectionCanvasVisualization->sourceFigure, connectionCanvasVisualization->sourceFigure->getBounds().getSize(), placementHint, placementPriority);
    }
    auto destinationModule = getSimulation()->getModule(connectionVisualization->destinationModuleId);
    if (destinationModule != nullptr) {
        auto destinationVisualization = networkNodeVisualizer->getNetworkNodeVisualization(getContainingNode(destinationModule));
        destinationVisualization->addAnnotation(connectionCanvasVisualization->destinationFigure, connectionCanvasVisualization->destinationFigure->getBounds().getSize(), placementHint, placementPriority);
    }
    setConnectionLabelsVisible(connectionVisualizations.size() > iconColorSet.getSize());
}

void TransportConnectionCanvasVisualizer::removeConnectionVisualization(const TransportConnectionVisualization *connectionVisualization)
{
    TransportConnectionVisualizerBase::removeConnectionVisualization(connectionVisualization);
    auto connectionCanvasVisualization = static_cast<const TransportConnectionCanvasVisualization *>(connectionVisualization);
    auto sourceModule = getSimulation()->getModule(connectionVisualization->sourceModuleId);
    if (sourceModule != nullptr) {
        auto sourceVisualization = networkNodeVisualizer->getNetworkNodeVisualization(getContainingNode(sourceModule));
        sourceVisualization->removeAnnotation(connectionCanvasVisualization->sourceFigure);
    }
    auto destinationModule = getSimulation()->getModule(connectionVisualization->destinationModuleId);
    if (destinationModule != nullptr) {
        auto destinationVisualization = networkNodeVisualizer->getNetworkNodeVisualization(getContainingNode(destinationModule));
        destinationVisualization->removeAnnotation(connectionCanvasVisualization->destinationFigure);
    }
    setConnectionLabelsVisible(connectionVisualizations.size() > iconColorSet.getSize());
}

void TransportConnectionCanvasVisualizer::setConnectionLabelsVisible(bool visible)
{
    for (auto connectionVisualization : connectionVisualizations) {
        auto connectionCanvasVisualization = static_cast<const TransportConnectionCanvasVisualization *>(connectionVisualization);
        connectionCanvasVisualization->sourceFigure->getLabelFigure()->setVisible(visible);
        connectionCanvasVisualization->destinationFigure->getLabelFigure()->setVisible(visible);
    }
}

} // namespace visualizer

} // namespace inet

