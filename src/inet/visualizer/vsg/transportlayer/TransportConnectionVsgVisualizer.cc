//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/vsg/transportlayer/TransportConnectionVsgVisualizer.h"

#include "inet/common/ModuleAccess.h"
#include "inet/visualizer/vsg/util/VsgUtils.h"

namespace inet {

namespace visualizer {

Define_Module(TransportConnectionVsgVisualizer);

TransportConnectionVsgVisualizer::TransportConnectionVsgVisualization::TransportConnectionVsgVisualization(::vsg::ref_ptr<::vsg::Node> sourceNode, ::vsg::ref_ptr<::vsg::Node> destinationNode, int sourceModuleId, int destinationModuleId, int count) :
    TransportConnectionVisualization(sourceModuleId, destinationModuleId, count),
    sourceNode(sourceNode),
    destinationNode(destinationNode)
{
}

void TransportConnectionVsgVisualizer::initialize(int stage)
{
    TransportConnectionVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        networkNodeVisualizer.reference(this, "networkNodeVisualizerModule", true);
    }
}

::vsg::ref_ptr<::vsg::Node> TransportConnectionVsgVisualizer::createConnectionEndNode(tcp::TcpConnection *tcpConnection) const
{
    // The end node is attached as an annotation (billboard-wrapped by NetworkNodeVsgVisualization),
    // so this is the connection icon as a tinted textured quad in the X-Y plane; fall back to a
    // solid-colored quad if the icon image can't be loaded.
    auto color = iconColorSet.getColor(connectionVisualizations.size());
    const double iconSize = 32.0;
    ::vsg::ref_ptr<::vsg::Data> iconImage;
    try { iconImage = inet::vsg::createImageFromResource(icon); }
    catch (const std::exception&) { iconImage = nullptr; }
    if (iconImage)
        return inet::vsg::createTexturedQuad(iconImage, iconSize, color);
    const double halfSize = 16.0;
    return inet::vsg::createQuad(Coord(-halfSize, 0.0, 0.0), Coord(halfSize, halfSize * 2, 0.0), color);
}

const TransportConnectionVisualizerBase::TransportConnectionVisualization *TransportConnectionVsgVisualizer::createConnectionVisualization(cModule *source, cModule *destination, tcp::TcpConnection *tcpConnection) const
{
    auto sourceNode = createConnectionEndNode(tcpConnection);
    auto destinationNode = createConnectionEndNode(tcpConnection);
    return new TransportConnectionVsgVisualization(sourceNode, destinationNode, source->getId(), destination->getId(), 1);
}

void TransportConnectionVsgVisualizer::addConnectionVisualization(const TransportConnectionVisualization *connectionVisualization)
{
    TransportConnectionVisualizerBase::addConnectionVisualization(connectionVisualization);
    auto connectionVsgVisualization = static_cast<const TransportConnectionVsgVisualization *>(connectionVisualization);
    auto sourceModule = getSimulation()->getModule(connectionVisualization->sourceModuleId);
    auto sourceNetworkNode = getContainingNode(sourceModule);
    auto sourceVisualization = networkNodeVisualizer->getNetworkNodeVisualization(sourceNetworkNode);
    sourceVisualization->addAnnotation(connectionVsgVisualization->sourceNode, ::vsg::dvec3(0, 0, 32), 0); // TODO size
    auto destinationModule = getSimulation()->getModule(connectionVisualization->destinationModuleId);
    auto destinationNetworkNode = getContainingNode(destinationModule);
    auto destinationVisualization = networkNodeVisualizer->getNetworkNodeVisualization(destinationNetworkNode);
    destinationVisualization->addAnnotation(connectionVsgVisualization->destinationNode, ::vsg::dvec3(0, 0, 32), 0); // TODO size
}

void TransportConnectionVsgVisualizer::removeConnectionVisualization(const TransportConnectionVisualization *connectionVisualization)
{
    TransportConnectionVisualizerBase::removeConnectionVisualization(connectionVisualization);
    auto connectionVsgVisualization = static_cast<const TransportConnectionVsgVisualization *>(connectionVisualization);
    auto sourceModule = getSimulation()->getModule(connectionVisualization->sourceModuleId);
    auto sourceVisualization = networkNodeVisualizer->getNetworkNodeVisualization(getContainingNode(sourceModule));
    sourceVisualization->removeAnnotation(connectionVsgVisualization->sourceNode.get());
    auto destinationModule = getSimulation()->getModule(connectionVisualization->destinationModuleId);
    auto destinationVisualization = networkNodeVisualizer->getNetworkNodeVisualization(getContainingNode(destinationModule));
    destinationVisualization->removeAnnotation(connectionVsgVisualization->destinationNode.get());
}

} // namespace visualizer

} // namespace inet
