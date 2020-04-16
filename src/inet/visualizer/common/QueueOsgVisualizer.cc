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
#include "inet/visualizer/common/QueueOsgVisualizer.h"

namespace inet {

namespace visualizer {

Define_Module(QueueOsgVisualizer);

#ifdef WITH_OSG

QueueOsgVisualizer::QueueOsgVisualization::QueueOsgVisualization(NetworkNodeOsgVisualization *networkNodeVisualization, osg::Geode *node, queueing::IPacketQueue *queue) :
    QueueVisualization(queue),
    networkNodeVisualization(networkNodeVisualization),
    node(node)
{
}

QueueOsgVisualizer::~QueueOsgVisualizer()
{
    if (displayQueues)
        removeAllQueueVisualizations();
}

void QueueOsgVisualizer::initialize(int stage)
{
    QueueVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        networkNodeVisualizer = getModuleFromPar<NetworkNodeOsgVisualizer>(par("networkNodeVisualizerModule"), this);
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
    if (networkNodeVisualization == nullptr)
        throw cRuntimeError("Cannot create queue visualization for '%s', because network node visualization is not found for '%s'", ownedObject->getFullPath().c_str(), networkNode->getFullPath().c_str());
    return new QueueOsgVisualization(networkNodeVisualization, geode, queue);
}

void QueueOsgVisualizer::refreshQueueVisualization(const QueueVisualization *queueVisualization) const
{
    // TODO:
    // auto infoOsgVisualization = static_cast<const QueueOsgVisualization *>(queueVisualization);
    // auto node = infoOsgVisualization->node;
}

#endif // ifdef WITH_OSG

} // namespace visualizer

} // namespace inet

