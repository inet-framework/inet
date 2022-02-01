//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/canvas/common/QueueCanvasVisualizer.h"

#include "inet/common/ModuleAccess.h"
#include "inet/queueing/queue/PacketQueue.h"

namespace inet {

namespace visualizer {

Define_Module(QueueCanvasVisualizer);

QueueCanvasVisualizer::QueueCanvasVisualization::QueueCanvasVisualization(NetworkNodeCanvasVisualization *networkNodeVisualization, QueueFigure *figure, queueing::IPacketQueue *queue) :
    QueueVisualization(queue),
    networkNodeVisualization(networkNodeVisualization),
    figure(figure)
{
}

QueueCanvasVisualizer::QueueCanvasVisualization::~QueueCanvasVisualization()
{
    delete figure;
}

void QueueCanvasVisualizer::initialize(int stage)
{
    QueueVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        zIndex = par("zIndex");
        networkNodeVisualizer.reference(this, "networkNodeVisualizerModule", true);
    }
}

QueueVisualizerBase::QueueVisualization *QueueCanvasVisualizer::createQueueVisualization(queueing::IPacketQueue *queue) const
{
    auto ownedObject = check_and_cast<cOwnedObject *>(queue);
    auto module = check_and_cast<cModule *>(ownedObject->getOwner());
    auto figure = new QueueFigure("queue");
    figure->setTags((std::string("queue ") + tags).c_str());
    figure->setTooltip("This figure represents a queue");
    figure->setAssociatedObject(ownedObject);
    figure->setColor(color);
    figure->setSpacing(spacing);
    figure->setBounds(cFigure::Rectangle(0, 0, width, height));
    if (auto packetQueue = dynamic_cast<queueing::PacketQueue *>(queue))
        figure->setMaxElementCount(packetQueue->getMaxNumPackets());
    auto networkNode = getContainingNode(module);
    auto networkNodeVisualization = networkNodeVisualizer->getNetworkNodeVisualization(networkNode);
    return new QueueCanvasVisualization(networkNodeVisualization, figure, queue);
}

void QueueCanvasVisualizer::addQueueVisualization(const QueueVisualization *queueVisualization)
{
    QueueVisualizerBase::addQueueVisualization(queueVisualization);
    auto queueCanvasVisualization = static_cast<const QueueCanvasVisualization *>(queueVisualization);
    auto figure = queueCanvasVisualization->figure;
    queueCanvasVisualization->networkNodeVisualization->addAnnotation(figure, figure->getBounds().getSize(), placementHint, placementPriority);
}

void QueueCanvasVisualizer::removeQueueVisualization(const QueueVisualization *queueVisualization)
{
    QueueVisualizerBase::removeQueueVisualization(queueVisualization);
    auto queueCanvasVisualization = static_cast<const QueueCanvasVisualization *>(queueVisualization);
    auto figure = queueCanvasVisualization->figure;
    if (networkNodeVisualizer != nullptr)
        queueCanvasVisualization->networkNodeVisualization->removeAnnotation(figure);
}

void QueueCanvasVisualizer::refreshQueueVisualization(const QueueVisualization *queueVisualization) const
{
    auto queueCanvasVisualization = static_cast<const QueueCanvasVisualization *>(queueVisualization);
    auto queue = queueVisualization->queue;
    auto figure = queueCanvasVisualization->figure;
    figure->setElementCount(queue->getNumPackets());
}

} // namespace visualizer

} // namespace inet

