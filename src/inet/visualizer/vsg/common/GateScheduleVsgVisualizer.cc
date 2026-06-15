//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/vsg/common/GateScheduleVsgVisualizer.h"

#include "inet/common/ModuleAccess.h"
#include "inet/visualizer/vsg/util/VsgUtils.h"

namespace inet {

namespace visualizer {

Define_Module(GateScheduleVsgVisualizer);

GateScheduleVsgVisualizer::GateVsgVisualization::GateVsgVisualization(NetworkNodeVsgVisualization *networkNodeVisualization, ::vsg::ref_ptr<::vsg::Group> node, queueing::IPacketGate *gate) :
    GateVisualization(gate),
    networkNodeVisualization(networkNodeVisualization),
    node(node)
{
}

void GateScheduleVsgVisualizer::initialize(int stage)
{
    GateScheduleVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL)
        networkNodeVisualizer.reference(this, "networkNodeVisualizerModule", true);
    else if (stage == INITSTAGE_LAST) {
        for (auto gateVisualization : gateVisualizations) {
            auto gateVsgVisualization = static_cast<const GateVsgVisualization *>(gateVisualization);
            gateVsgVisualization->networkNodeVisualization->addAnnotation(gateVsgVisualization->node, ::vsg::dvec3(0, 0, 0), 0);
        }
    }
}

GateScheduleVisualizerBase::GateVisualization *GateScheduleVsgVisualizer::createGateVisualization(queueing::IPacketGate *gate) const
{
    auto node = ::vsg::Group::create();
    auto ownedObject = check_and_cast<cOwnedObject *>(gate);
    auto module = check_and_cast<cModule *>(ownedObject->getOwner());
    auto networkNode = getContainingNode(module);
    auto networkNodeVisualization = networkNodeVisualizer->getNetworkNodeVisualization(networkNode);
    return new GateVsgVisualization(networkNodeVisualization, node, gate);
}

void GateScheduleVsgVisualizer::refreshGateVisualization(const GateVisualization *gateVisualization) const
{
    auto gateVsgVisualization = static_cast<const GateVsgVisualization *>(gateVisualization);
    auto gate = gateVisualization->gate;
    auto ownedObject = check_and_cast<cOwnedObject *>(gate);
    auto module = check_and_cast<cModule *>(ownedObject->getOwner());

    // Build a label text from the configured string format (e.g. gate module name).
    std::string labelText = getGateScheduleVisualizationText(module);
    bool isOpen = gate->isOpen();

    // Append open/closed state to the label so changes to gate state trigger a rebuild.
    // TODO: OSG version rendered a full gantt-like timeline bar over the displayDuration
    //       window; here we approximate with a coloured indicator box plus a text label.
    std::string fullText = labelText + (isOpen ? " [O]" : " [C]");

    if (gateVsgVisualization->lastText == fullText)
        return; // unchanged — avoid rebuilding the annotation node
    gateVsgVisualization->lastText = fullText;

    gateVsgVisualization->node->children.clear();

    // Draw a small coloured indicator box: green for open, red for closed.
    // TODO: replace with a proper gantt-style schedule bar spanning displayDuration.
    double boxW = width  > 0 ? width  : 10.0;
    double boxH = height > 0 ? height : 10.0;
    cFigure::Color boxColor = isOpen ? cFigure::Color(0, 200, 0) : cFigure::Color(200, 0, 0);
    Coord boxMin(-(boxW / 2.0), 0, 0);
    Coord boxMax( (boxW / 2.0), boxH, 0);
    gateVsgVisualization->node->addChild(inet::vsg::createQuad(boxMin, boxMax, boxColor));

    // Label above the box.
    if (!labelText.empty()) {
        gateVsgVisualization->node->addChild(
            inet::vsg::createLabel(labelText.c_str(), Coord(0, boxH + 2.0, 0), cFigure::BLACK, 14));
    }
}

} // namespace visualizer

} // namespace inet
