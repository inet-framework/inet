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

#include "inet/common/OSGScene.h"
#include "inet/common/OSGUtils.h"
#include "inet/visualizer/common/PacketDropOsgVisualizer.h"

namespace inet {

namespace visualizer {

Define_Module(PacketDropOsgVisualizer);

PacketDropOsgVisualizer::OsgPacketDrop::OsgPacketDrop(osg::Node *node, int moduleId, cPacket *packet, simtime_t dropSimulationTime, double dropAnimationTime, int dropRealTime) :
    PacketDropVisualization(moduleId, packet, dropSimulationTime, dropAnimationTime, dropRealTime),
    node(node)
{
}

PacketDropOsgVisualizer::OsgPacketDrop::~OsgPacketDrop()
{
    // TODO: delete node;
}

const PacketDropVisualizerBase::PacketDropVisualization *PacketDropOsgVisualizer::createPacketDropVisualization(cModule *module, cPacket *packet) const
{
    auto path = resolveResourcePath("msg/packet_s.png");
    auto image = inet::osg::createImage(path.c_str());
    auto texture = new osg::Texture2D();
    texture->setImage(image);
    auto geometry = osg::createTexturedQuadGeometry(osg::Vec3(-image->s() / 2, 0.0, 0.0), osg::Vec3(image->s(), 0.0, 0.0), osg::Vec3(0.0, image->t(), 0.0), 0.0, 0.0, 1.0, 1.0);
    auto stateSet = geometry->getOrCreateStateSet();
    stateSet->setTextureAttributeAndModes(0, texture);
    stateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
    stateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
    stateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    stateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    auto geode = new osg::Geode();
    geode->addDrawable(geometry);
    return new OsgPacketDrop(geode, module->getId(), packet, getSimulation()->getSimTime(), getSimulation()->getEnvir()->getAnimationTime(), getRealTime());
}

void PacketDropOsgVisualizer::setAlpha(const PacketDropVisualization *packetDrop, double alpha) const
{
    // TODO:
//    auto osgPacketDrop = static_cast<const OsgPacketDrop *>(packetDrop);
//    auto node = osgPacketDrop->node;
//    auto material = static_cast<osg::Material *>(node->getOrCreateStateSet()->getAttribute(osg::StateAttribute::MATERIAL));
//    material->setAlpha(osg::Material::FRONT_AND_BACK, alpha);
}

void PacketDropOsgVisualizer::addPacketDropVisualization(const PacketDropVisualization *packetDrop)
{
    PacketDropVisualizerBase::addPacketDropVisualization(packetDrop);
    auto osgPacketDrop = static_cast<const OsgPacketDrop *>(packetDrop);
    auto scene = inet::osg::TopLevelScene::getSimulationScene(visualizerTargetModule);
    scene->addChild(osgPacketDrop->node);
}

void PacketDropOsgVisualizer::removePacketDropVisualization(const PacketDropVisualization *packetDrop)
{
    PacketDropVisualizerBase::removePacketDropVisualization(packetDrop);
    auto osgPacketDrop = static_cast<const OsgPacketDrop *>(packetDrop);
    auto node = osgPacketDrop->node;
    node->getParent(0)->removeChild(node);
}

} // namespace visualizer

} // namespace inet

