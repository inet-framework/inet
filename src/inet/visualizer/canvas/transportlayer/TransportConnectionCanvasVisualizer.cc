//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/canvas/transportlayer/TransportConnectionCanvasVisualizer.h"

#include "inet/common/ModuleAccess.h"

namespace inet {

namespace visualizer {

Define_Module(TransportConnectionCanvasVisualizer);

TransportConnectionCanvasVisualizer::TransportConnectionCanvasVisualization::TransportConnectionCanvasVisualization(LabeledIconFigure *sourceFigure, LabeledIconFigure *destinationFigure, int sourceModuleId, int destinationModuleId, int count) :
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
        auto canvas = visualizationTargetModule->getCanvas();
        canvasProjection = CanvasProjection::getCanvasProjection(canvas);
        networkNodeVisualizer.reference(this, "networkNodeVisualizerModule", true);
    }
}

LabeledIconFigure *TransportConnectionCanvasVisualizer::createConnectionEndFigure(tcp::TcpConnection *tcpConnection) const
{
#ifdef INET_WITH_TCP_INET
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
#endif // INET_WITH_TCP_INET
}

const TransportConnectionVisualizerBase::TransportConnectionVisualization *TransportConnectionCanvasVisualizer::createConnectionVisualization(cModule *source, cModule *destination, tcp::TcpConnection *tcpConnection) const
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

