//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/osg/common/QueueOsgVisualizer.h"

#include "inet/common/ModuleAccess.h"

namespace inet {

namespace visualizer {

Define_Module(QueueOsgVisualizer);

QueueOsgVisualizer::QueueOsgVisualization::QueueOsgVisualization(NetworkNodeOsgVisualization *networkNodeVisualization, osg::Geode *node, queueing::IPacketQueue *queue) :
    QueueVisualization(queue),
    networkNodeVisualization(networkNodeVisualization),
    node(node)
{
}

void QueueOsgVisualizer::initialize(int stage)
{
    QueueVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        networkNodeVisualizer.reference(this, "networkNodeVisualizerModule", true);
    }
    else if (stage == INITSTAGE_LAST) {
        for (auto queueVisualization : queueVisualizations) {
            auto queueOsgVisualization = static_cast<const QueueOsgVisualization *>(queueVisualization);
            auto node = queueOsgVisualization->node;
            queueOsgVisualization->networkNodeVisualization->addAnnotation(node, osg::Vec3d(0, 0, 0), 0);
        }
    }
}

QueueVisualizerBase::QueueVisualization *QueueOsgVisualizer::createQueueVisualization(queueing::IPacketQueue *queue) const
{
    auto ownedObject = check_and_cast<cOwnedObject *>(queue);
    auto module = check_and_cast<cModule *>(ownedObject->getOwner());
    auto geode = new osg::Geode();
    geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    auto networkNode = getContainingNode(module);
    auto networkNodeVisualization = networkNodeVisualizer->getNetworkNodeVisualization(networkNode);
    return new QueueOsgVisualization(networkNodeVisualization, geode, queue);
}

void QueueOsgVisualizer::refreshQueueVisualization(const QueueVisualization *queueVisualization) const
{
    // TODO
//    auto infoOsgVisualization = static_cast<const QueueOsgVisualization *>(queueVisualization);
//    auto node = infoOsgVisualization->node;
}

} // namespace visualizer

} // namespace inet

