//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/vsg/common/QueueVsgVisualizer.h"

#include <sstream>

#include "inet/common/ModuleAccess.h"
#include "inet/visualizer/vsg/util/VsgUtils.h"

namespace inet {

namespace visualizer {

Define_Module(QueueVsgVisualizer);

QueueVsgVisualizer::QueueVsgVisualization::QueueVsgVisualization(NetworkNodeVsgVisualization *networkNodeVisualization, ::vsg::ref_ptr<::vsg::Group> node, queueing::IPacketQueue *queue) :
    QueueVisualization(queue),
    networkNodeVisualization(networkNodeVisualization),
    node(node)
{
}

void QueueVsgVisualizer::initialize(int stage)
{
    QueueVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL)
        networkNodeVisualizer.reference(this, "networkNodeVisualizerModule", true);
    else if (stage == INITSTAGE_LAST) {
        for (auto queueVisualization : queueVisualizations) {
            auto queueVsgVisualization = static_cast<const QueueVsgVisualization *>(queueVisualization);
            queueVsgVisualization->networkNodeVisualization->addAnnotation(queueVsgVisualization->node, ::vsg::dvec3(0, 0, 0), 0);
        }
    }
}

QueueVisualizerBase::QueueVisualization *QueueVsgVisualizer::createQueueVisualization(queueing::IPacketQueue *queue) const
{
    auto node = ::vsg::Group::create();
    auto ownedObject = check_and_cast<cOwnedObject *>(queue);
    auto module = check_and_cast<cModule *>(ownedObject->getOwner());
    auto networkNode = getContainingNode(module);
    auto networkNodeVisualization = networkNodeVisualizer->getNetworkNodeVisualization(networkNode);
    return new QueueVsgVisualization(networkNodeVisualization, node, queue);
}

void QueueVsgVisualizer::refreshQueueVisualization(const QueueVisualization *queueVisualization) const
{
    auto queueVsgVisualization = static_cast<const QueueVsgVisualization *>(queueVisualization);
    auto queue = queueVisualization->queue;
    int numPackets = queue->getNumPackets();
    int maxNumPackets = queue->getMaxNumPackets();

    if (queueVsgVisualization->lastNumPackets == numPackets &&
        queueVsgVisualization->lastMaxNumPackets == maxNumPackets)
        return; // unchanged — avoid rebuilding the annotation node

    queueVsgVisualization->lastNumPackets = numPackets;
    queueVsgVisualization->lastMaxNumPackets = maxNumPackets;

    queueVsgVisualization->node->children.clear();

    // Represent queue fill as a coloured box whose height is proportional to fill ratio,
    // plus a text label showing "n/max" (or just "n" when capacity is unlimited).
    // TODO: OSG version used a proper bar figure; here we approximate with a box + label.
    double fillRatio = (maxNumPackets > 0) ? std::min(1.0, (double)numPackets / maxNumPackets) : 0.0;
    // Choose a colour from green (empty) → red (full).
    // When capacity is unlimited we use the configured color parameter.
    cFigure::Color barColor;
    if (maxNumPackets > 0) {
        // Interpolate green -> red based on fill ratio
        int r = (int)(fillRatio * 255);
        int g = (int)((1.0 - fillRatio) * 255);
        barColor = cFigure::Color(r, g, 0);
    }
    else {
        barColor = color; // use the parameter color when capacity unknown
    }

    // Draw a small filled bar (width × height scaled by fill). The bar sits at local origin.
    // Dimensions are in 3D world units (same scale as text labels, ~18 units tall for full).
    double barWidth = width > 0 ? width : 10.0;
    double barHeight = (height > 0 ? height : 18.0) * (maxNumPackets > 0 ? fillRatio : 1.0);
    if (barHeight < 0.5) barHeight = 0.5; // always show at least a sliver so the annotation is visible
    Coord barMin(-(barWidth / 2.0), 0, 0);
    Coord barMax( (barWidth / 2.0), barHeight, 0);
    queueVsgVisualization->node->addChild(inet::vsg::createQuad(barMin, barMax, barColor));

    // Text label showing packet count.
    std::ostringstream ss;
    if (maxNumPackets > 0)
        ss << numPackets << "/" << maxNumPackets;
    else
        ss << numPackets;
    queueVsgVisualization->node->addChild(
        inet::vsg::createText(ss.str().c_str(), Coord(0, barHeight + 2.0, 0), color, 14));
}

} // namespace visualizer

} // namespace inet
