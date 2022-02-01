//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/osg/linklayer/InterfaceTableOsgVisualizer.h"

#include "inet/common/ModuleAccess.h"
#include "inet/visualizer/osg/util/OsgUtils.h"

namespace inet {

namespace visualizer {

Define_Module(InterfaceTableOsgVisualizer);

InterfaceTableOsgVisualizer::InterfaceOsgVisualization::InterfaceOsgVisualization(NetworkNodeOsgVisualization *networkNodeVisualization, osg::Node *node, int networkNodeId, int networkNodeGateId, int interfaceId) :
    InterfaceVisualization(networkNodeId, networkNodeGateId, interfaceId),
    networkNodeVisualization(networkNodeVisualization),
    node(node)
{
}

void InterfaceTableOsgVisualizer::initialize(int stage)
{
    InterfaceTableVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        networkNodeVisualizer.reference(this, "networkNodeVisualizerModule", true);
    }
}

InterfaceTableVisualizerBase::InterfaceVisualization *InterfaceTableOsgVisualizer::createInterfaceVisualization(cModule *networkNode, NetworkInterface *networkInterface)
{
    auto gate = getOutputGate(networkNode, networkInterface);
    auto label = new osgText::Text();
    label->setCharacterSize(18);
    label->setBoundingBoxColor(osg::Vec4(backgroundColor.red / 255.0, backgroundColor.green / 255.0, backgroundColor.blue / 255.0, opacity));
    label->setColor(osg::Vec4(textColor.red / 255.0, textColor.green / 255.0, textColor.blue / 255.0, opacity));
    label->setAlignment(osgText::Text::CENTER_BOTTOM);
    label->setText(getVisualizationText(networkInterface).c_str());
    label->setDrawMode(osgText::Text::FILLEDBOUNDINGBOX | osgText::Text::TEXT);
    label->setPosition(osg::Vec3(0.0, 0.0, 0.0));
    auto geode = new osg::Geode();
    geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    geode->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    geode->addDrawable(label);
    auto networkNodeVisualization = networkNodeVisualizer->getNetworkNodeVisualization(networkNode);
    return new InterfaceOsgVisualization(networkNodeVisualization, geode, networkNode->getId(), gate == nullptr ? -1 : gate->getId(), networkInterface->getInterfaceId());
}

void InterfaceTableOsgVisualizer::addInterfaceVisualization(const InterfaceVisualization *interfaceVisualization)
{
    InterfaceTableVisualizerBase::addInterfaceVisualization(interfaceVisualization);
    auto interfaceOsgVisualization = static_cast<const InterfaceOsgVisualization *>(interfaceVisualization);
    interfaceOsgVisualization->networkNodeVisualization->addAnnotation(interfaceOsgVisualization->node, osg::Vec3d(100, 18, 0), 1.0);
}

void InterfaceTableOsgVisualizer::removeInterfaceVisualization(const InterfaceVisualization *interfaceVisualization)
{
    InterfaceTableVisualizerBase::removeInterfaceVisualization(interfaceVisualization);
    auto interfaceOsgVisualization = static_cast<const InterfaceOsgVisualization *>(interfaceVisualization);
    if (networkNodeVisualizer != nullptr)
        interfaceOsgVisualization->networkNodeVisualization->removeAnnotation(interfaceOsgVisualization->node);
}

void InterfaceTableOsgVisualizer::refreshInterfaceVisualization(const InterfaceVisualization *interfaceVisualization, const NetworkInterface *networkInterface)
{
    auto interfaceOsgVisualization = static_cast<const InterfaceOsgVisualization *>(interfaceVisualization);
    auto geode = check_and_cast<osg::Geode *>(interfaceOsgVisualization->node);
    auto label = check_and_cast<osgText::Text *>(geode->getDrawable(0));
    label->setText(getVisualizationText(networkInterface).c_str());
}

} // namespace visualizer

} // namespace inet

