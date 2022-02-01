//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/osg/common/PacketDropOsgVisualizer.h"

#include "inet/common/ModuleAccess.h"
#include "inet/visualizer/osg/util/OsgScene.h"
#include "inet/visualizer/osg/util/OsgUtils.h"

namespace inet {

namespace visualizer {

Define_Module(PacketDropOsgVisualizer);

PacketDropOsgVisualizer::PacketDropOsgVisualization::PacketDropOsgVisualization(osg::Node *node, const PacketDrop *packetDrop) :
    PacketDropVisualization(packetDrop),
    node(node)
{
}

void PacketDropOsgVisualizer::refreshDisplay() const
{
    PacketDropVisualizerBase::refreshDisplay();
    // TODO switch to osg canvas when API is extended
    visualizationTargetModule->getCanvas()->setAnimationSpeed(packetDropVisualizations.empty() ? 0 : fadeOutAnimationSpeed, this);
}

const PacketDropVisualizerBase::PacketDropVisualization *PacketDropOsgVisualizer::createPacketDropVisualization(PacketDrop *packetDrop) const
{
    auto image = inet::osg::createImageFromResource("msg/packet_s");
    auto texture = new osg::Texture2D();
    texture->setImage(image);
    auto geometry = osg::createTexturedQuadGeometry(osg::Vec3(-image->s() / 2, 0.0, 0.0), osg::Vec3(image->s(), 0.0, 0.0), osg::Vec3(0.0, image->t(), 0.0), 0.0, 0.0, 1.0, 1.0);
    auto stateSet = geometry->getOrCreateStateSet();
    stateSet->setTextureAttributeAndModes(0, texture);
    stateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
    stateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
    stateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    stateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    auto autoTransform = inet::osg::createAutoTransform(geometry, osg::AutoTransform::ROTATE_TO_SCREEN, true);
    auto material = new osg::Material();
    auto iconTintColor = iconTintColorSet.getColor(packetDrop->getReason() % iconTintColorSet.getSize());
    osg::Vec4 colorVec((double)iconTintColor.red / 255.0, (double)iconTintColor.green / 255.0, (double)iconTintColor.blue / 255.0, 1.0);
    material->setAmbient(osg::Material::FRONT_AND_BACK, colorVec);
    material->setDiffuse(osg::Material::FRONT_AND_BACK, colorVec);
    material->setAlpha(osg::Material::FRONT_AND_BACK, 1.0);
    autoTransform->getChild(0)->getOrCreateStateSet()->setAttribute(material);
    auto positionAttitudeTransform = inet::osg::createPositionAttitudeTransform(packetDrop->getPosition(), Quaternion::IDENTITY);
    positionAttitudeTransform->addChild(autoTransform);
    return new PacketDropOsgVisualization(positionAttitudeTransform, packetDrop);
}

void PacketDropOsgVisualizer::addPacketDropVisualization(const PacketDropVisualization *packetDropVisualization)
{
    PacketDropVisualizerBase::addPacketDropVisualization(packetDropVisualization);
    auto packetDropOsgVisualization = static_cast<const PacketDropOsgVisualization *>(packetDropVisualization);
    auto scene = inet::osg::TopLevelScene::getSimulationScene(visualizationTargetModule);
    scene->addChild(packetDropOsgVisualization->node);
}

void PacketDropOsgVisualizer::removePacketDropVisualization(const PacketDropVisualization *packetDropVisualization)
{
    PacketDropVisualizerBase::removePacketDropVisualization(packetDropVisualization);
    auto packetDropOsgVisualization = static_cast<const PacketDropOsgVisualization *>(packetDropVisualization);
    auto node = packetDropOsgVisualization->node;
    node->getParent(0)->removeChild(node);
}

void PacketDropOsgVisualizer::setAlpha(const PacketDropVisualization *packetDropVisualization, double alpha) const
{
    auto packetDropOsgVisualization = static_cast<const PacketDropOsgVisualization *>(packetDropVisualization);
    auto positionAttitudeTransform = static_cast<osg::PositionAttitudeTransform *>(packetDropOsgVisualization->node.get());
    auto autoTransform = static_cast<osg::AutoTransform *>(positionAttitudeTransform->getChild(0));
    auto geode = static_cast<osg::Geode *>(autoTransform->getChild(0));
    auto material = static_cast<osg::Material *>(geode->getOrCreateStateSet()->getAttribute(osg::StateAttribute::MATERIAL));
    material->setAlpha(osg::Material::FRONT_AND_BACK, alpha);
    double dx = 10 / alpha;
    double dy = 10 / alpha;
    double dz = 58 - pow((dx / 4 - 9), 2);
    auto& position = packetDropVisualization->packetDrop->getPosition();
    positionAttitudeTransform->setPosition(osg::Vec3d(position.x + dx, position.y + dy, position.z + dz));
}

} // namespace visualizer

} // namespace inet

