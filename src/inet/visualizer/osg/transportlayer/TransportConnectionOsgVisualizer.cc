//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/osg/transportlayer/TransportConnectionOsgVisualizer.h"

#include "inet/common/ModuleAccess.h"
#include "inet/visualizer/osg/util/OsgUtils.h"

namespace inet {

namespace visualizer {

Define_Module(TransportConnectionOsgVisualizer);

TransportConnectionOsgVisualizer::TransportConnectionOsgVisualization::TransportConnectionOsgVisualization(osg::Node *sourceNode, osg::Node *destinationNode, int sourceModuleId, int destinationModuleId, int count) :
    TransportConnectionVisualization(sourceModuleId, destinationModuleId, count),
    sourceNode(sourceNode),
    destinationNode(destinationNode)
{
}

void TransportConnectionOsgVisualizer::initialize(int stage)
{
    TransportConnectionVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        networkNodeVisualizer.reference(this, "networkNodeVisualizerModule", true);
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
    return new TransportConnectionOsgVisualization(sourceNode, destinationNode, source->getId(), destination->getId(), 1);
}

void TransportConnectionOsgVisualizer::addConnectionVisualization(const TransportConnectionVisualization *connectionVisualization)
{
    TransportConnectionVisualizerBase::addConnectionVisualization(connectionVisualization);
    auto connectionOsgVisualization = static_cast<const TransportConnectionOsgVisualization *>(connectionVisualization);
    auto sourceModule = getSimulation()->getModule(connectionVisualization->sourceModuleId);
    auto sourceNetworkNode = getContainingNode(sourceModule);
    auto sourceVisualization = networkNodeVisualizer->getNetworkNodeVisualization(sourceNetworkNode);
    sourceVisualization->addAnnotation(connectionOsgVisualization->sourceNode, osg::Vec3d(0, 0, 32), 0); // TODO size
    auto destinationModule = getSimulation()->getModule(connectionVisualization->destinationModuleId);
    auto destinationNetworkNode = getContainingNode(destinationModule);
    auto destinationVisualization = networkNodeVisualizer->getNetworkNodeVisualization(destinationNetworkNode);
    destinationVisualization->addAnnotation(connectionOsgVisualization->destinationNode, osg::Vec3d(0, 0, 32), 0); // TODO size
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

} // namespace visualizer

} // namespace inet

