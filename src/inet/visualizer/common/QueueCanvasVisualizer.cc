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
#include "inet/queueing/queue/PacketQueue.h"
#include "inet/visualizer/common/QueueCanvasVisualizer.h"

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

QueueCanvasVisualizer::~QueueCanvasVisualizer()
{
    if (displayQueues)
        removeAllQueueVisualizations();
}

void QueueCanvasVisualizer::initialize(int stage)
{
    QueueVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        zIndex = par("zIndex");
        networkNodeVisualizer = getModuleFromPar<NetworkNodeCanvasVisualizer>(par("networkNodeVisualizerModule"), this);
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
    if (networkNodeVisualization == nullptr)
        throw cRuntimeError("Cannot create queue visualization for '%s', because network node visualization is not found for '%s'", ownedObject->getFullPath().c_str(), networkNode->getFullPath().c_str());
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

