//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "inet/visualizer/linklayer/InterfaceTableOsgVisualizer.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/OsgUtils.h"

namespace inet {

namespace visualizer {

Define_Module(InterfaceTableOsgVisualizer);

#ifdef WITH_OSG

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
    if (networkNodeVisualization == nullptr)
        throw cRuntimeError("Cannot create interface visualization for '%s', because network node visualization is not found for '%s'", networkInterface->getInterfaceName(), networkNode->getFullPath().c_str());
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

#endif // ifdef WITH_OSG

} // namespace visualizer

} // namespace inet

