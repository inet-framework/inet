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
#include "inet/common/OsgUtils.h"
#include "inet/visualizer/transportlayer/TransportConnectionOsgVisualizer.h"

namespace inet {

namespace visualizer {

Define_Module(TransportConnectionOsgVisualizer);

#ifdef WITH_OSG

TransportConnectionOsgVisualizer::TransportConnectionOsgVisualization::TransportConnectionOsgVisualization(osg::Node *sourceNode, osg::Node *destinationNode, int sourceModuleId, int destinationModuleId, int count) :
    TransportConnectionVisualization(sourceModuleId, destinationModuleId, count),
    sourceNode(sourceNode),
    destinationNode(destinationNode)
{
}

TransportConnectionOsgVisualizer::~TransportConnectionOsgVisualizer()
{
    if (displayTransportConnections)
        removeAllConnectionVisualizations();
}

void TransportConnectionOsgVisualizer::initialize(int stage)
{
    TransportConnectionVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        networkNodeVisualizer = getModuleFromPar<NetworkNodeOsgVisualizer>(par("networkNodeVisualizerModule"), this);
    }
}

osg::Node *TransportConnectionOsgVisualizer::createConnectionEndNode(tcp::TcpConnection *tcpConnection) const
{
    auto image = inet::osg::createImageFromResource(icon);
    auto texture = new osg::Texture2D();
    texture->setImage(image);
    auto geometry = osg::createTexturedQuadGeometry(osg::Vec3(-image->s() / 2, 0.0, 0.0), osg::Vec3(image->s(), 0.0, 0.0), osg::Vec3(0.0, image->t(), 0.0), 0.0, 0.0, 1.0, 1.0);
    auto stateSet = geometry->getOrCreateStateSet();
    stateSet->setTextureAttributeAndModes(0, texture);
    stateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
    stateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
    stateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    stateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    auto color = iconColorSet.getColor(connectionVisualizations.size());
    auto colorArray = new osg::Vec4Array();
    colorArray->push_back(osg::Vec4((double)color.red / 255.0, (double)color.green / 255.0, (double)color.blue / 255.0, 1));
    geometry->setColorArray(colorArray, osg::Array::BIND_OVERALL);
    auto geode = new osg::Geode();
    geode->addDrawable(geometry);
    return geode;
}

const TransportConnectionVisualizerBase::TransportConnectionVisualization *TransportConnectionOsgVisualizer::createConnectionVisualization(cModule *source, cModule *destination, tcp::TcpConnection *tcpConnection) const
{
    auto sourceNode = createConnectionEndNode(tcpConnection);
    auto destinationNode = createConnectionEndNode(tcpConnection);
    auto sourceNetworkNode = getContainingNode(source);
    auto sourceVisualization = networkNodeVisualizer->getNetworkNodeVisualization(sourceNetworkNode);
    if (sourceVisualization == nullptr)
        throw cRuntimeError("Cannot create transport connection visualization for '%s', because network node visualization is not found for '%s'", source->getFullPath().c_str(), sourceNetworkNode->getFullPath().c_str());
    auto destinationNetworkNode = getContainingNode(destination);
    auto destinationVisualization = networkNodeVisualizer->getNetworkNodeVisualization(destinationNetworkNode);
    if (destinationVisualization == nullptr)
        throw cRuntimeError("Cannot create transport connection visualization for '%s', because network node visualization is not found for '%s'", source->getFullPath().c_str(), destinationNetworkNode->getFullPath().c_str());
    return new TransportConnectionOsgVisualization(sourceNode, destinationNode, source->getId(), destination->getId(), 1);
}

void TransportConnectionOsgVisualizer::addConnectionVisualization(const TransportConnectionVisualization *connectionVisualization)
{
    TransportConnectionVisualizerBase::addConnectionVisualization(connectionVisualization);
    auto connectionOsgVisualization = static_cast<const TransportConnectionOsgVisualization *>(connectionVisualization);
    auto sourceModule = getSimulation()->getModule(connectionVisualization->sourceModuleId);
    auto sourceVisualization = networkNodeVisualizer->getNetworkNodeVisualization(getContainingNode(sourceModule));
    sourceVisualization->addAnnotation(connectionOsgVisualization->sourceNode, osg::Vec3d(0, 0, 32), 0); // TODO: size
    auto destinationModule = getSimulation()->getModule(connectionVisualization->destinationModuleId);
    auto destinationVisualization = networkNodeVisualizer->getNetworkNodeVisualization(getContainingNode(destinationModule));
    destinationVisualization->addAnnotation(connectionOsgVisualization->destinationNode, osg::Vec3d(0, 0, 32), 0); // TODO: size
}

void TransportConnectionOsgVisualizer::removeConnectionVisualization(const TransportConnectionVisualization *connectionVisualization)
{
    TransportConnectionVisualizerBase::removeConnectionVisualization(connectionVisualization);
    auto connectionOsgVisualization = static_cast<const TransportConnectionOsgVisualization *>(connectionVisualization);
    auto sourceModule = getSimulation()->getModule(connectionVisualization->sourceModuleId);
    auto sourceVisualization = networkNodeVisualizer->getNetworkNodeVisualization(getContainingNode(sourceModule));
    sourceVisualization->removeAnnotation(connectionOsgVisualization->sourceNode);
    auto destinationModule = getSimulation()->getModule(connectionVisualization->destinationModuleId);
    auto destinationVisualization = networkNodeVisualizer->getNetworkNodeVisualization(getContainingNode(destinationModule));
    destinationVisualization->removeAnnotation(connectionOsgVisualization->destinationNode);
}

#endif // ifdef WITH_OSG

} // namespace visualizer

} // namespace inet

