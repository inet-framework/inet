//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/vsg/linklayer/InterfaceTableVsgVisualizer.h"

#include "inet/common/ModuleAccess.h"
#include "inet/visualizer/vsg/util/VsgUtils.h"

namespace inet {

namespace visualizer {

Define_Module(InterfaceTableVsgVisualizer);

InterfaceTableVsgVisualizer::InterfaceVsgVisualization::InterfaceVsgVisualization(NetworkNodeVsgVisualization *networkNodeVisualization, ::vsg::ref_ptr<::vsg::Group> node, int networkNodeId, int networkNodeGateId, int interfaceId) :
    InterfaceVisualization(networkNodeId, networkNodeGateId, interfaceId),
    networkNodeVisualization(networkNodeVisualization),
    node(node)
{
}

void InterfaceTableVsgVisualizer::initialize(int stage)
{
    InterfaceTableVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL)
        networkNodeVisualizer.reference(this, "networkNodeVisualizerModule", true);
}

InterfaceTableVisualizerBase::InterfaceVisualization *InterfaceTableVsgVisualizer::createInterfaceVisualization(cModule *networkNode, NetworkInterface *networkInterface)
{
    auto gate = getOutputGate(networkNode, networkInterface);
    auto node = ::vsg::Group::create();
    auto text = getVisualizationText(networkInterface);
    if (!text.empty())
        node->addChild(inet::vsg::createLabel(text.c_str(), Coord::ZERO, textColor, 18));
    auto networkNodeVisualization = networkNodeVisualizer->getNetworkNodeVisualization(networkNode);
    return new InterfaceVsgVisualization(networkNodeVisualization, node, networkNode->getId(), gate == nullptr ? -1 : gate->getId(), networkInterface->getInterfaceId());
}

void InterfaceTableVsgVisualizer::addInterfaceVisualization(const InterfaceVisualization *interfaceVisualization)
{
    InterfaceTableVisualizerBase::addInterfaceVisualization(interfaceVisualization);
    auto interfaceVsgVisualization = static_cast<const InterfaceVsgVisualization *>(interfaceVisualization);
    interfaceVsgVisualization->networkNodeVisualization->addAnnotation(interfaceVsgVisualization->node, ::vsg::dvec3(100, 18, 0), 1.0);
}

void InterfaceTableVsgVisualizer::removeInterfaceVisualization(const InterfaceVisualization *interfaceVisualization)
{
    InterfaceTableVisualizerBase::removeInterfaceVisualization(interfaceVisualization);
    auto interfaceVsgVisualization = static_cast<const InterfaceVsgVisualization *>(interfaceVisualization);
    if (networkNodeVisualizer != nullptr)
        interfaceVsgVisualization->networkNodeVisualization->removeAnnotation(interfaceVsgVisualization->node);
}

void InterfaceTableVsgVisualizer::refreshInterfaceVisualization(const InterfaceVisualization *interfaceVisualization, const NetworkInterface *networkInterface)
{
    auto interfaceVsgVisualization = static_cast<const InterfaceVsgVisualization *>(interfaceVisualization);
    std::string text = getVisualizationText(networkInterface);
    if (interfaceVsgVisualization->lastText == text)
        return; // unchanged — avoid rebuilding the label
    interfaceVsgVisualization->lastText = text;
    interfaceVsgVisualization->node->children.clear();
    if (!text.empty())
        interfaceVsgVisualization->node->addChild(inet::vsg::createLabel(text.c_str(), Coord::ZERO, textColor, 18));
}

} // namespace visualizer

} // namespace inet
