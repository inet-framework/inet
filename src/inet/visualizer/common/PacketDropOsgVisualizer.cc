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
#include "inet/common/OSGScene.h"
#include "inet/common/OSGUtils.h"
#include "inet/visualizer/common/PacketDropOsgVisualizer.h"

namespace inet {

namespace visualizer {

Define_Module(PacketDropOsgVisualizer);

#ifdef WITH_OSG

PacketDropOsgVisualizer::PacketDropOsgVisualization::PacketDropOsgVisualization(osg::Node *node, int moduleId, const cPacket *packet, const Coord& position) :
    PacketDropVisualization(moduleId, packet, position),
    node(node)
{
}

PacketDropOsgVisualizer::PacketDropOsgVisualization::~PacketDropOsgVisualization()
{
    // TODO: delete node;
}

void PacketDropOsgVisualizer::refreshDisplay() const
{
    PacketDropVisualizerBase::refreshDisplay();
    // TODO: switch to osg canvas when API is extended
    visualizerTargetModule->getCanvas()->setAnimationSpeed(packetDropVisualizations.empty() ? 0 : fadeOutAnimationSpeed, this);
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
    auto autoTransform = inet::osg::createAutoTransform(geometry, osg::AutoTransform::ROTATE_TO_SCREEN, true);
    auto material = new osg::Material();
    osg::Vec4 colorVec((double)iconTintColor.red / 255.0, (double)iconTintColor.green / 255.0, (double)iconTintColor.blue / 255.0, 1.0);
    material->setAmbient(osg::Material::FRONT_AND_BACK, colorVec);
    material->setDiffuse(osg::Material::FRONT_AND_BACK, colorVec);
    material->setAlpha(osg::Material::FRONT_AND_BACK, 1.0);
    autoTransform->getChild(0)->getOrCreateStateSet()->setAttribute(material);
    auto position = getPosition(getContainingNode(module));
    auto positionAttitudeTransform = inet::osg::createPositionAttitudeTransform(position, EulerAngles::ZERO);
    positionAttitudeTransform->addChild(autoTransform);
    return new PacketDropOsgVisualization(positionAttitudeTransform, module->getId(), packet, position);
}

void PacketDropOsgVisualizer::addPacketDropVisualization(const PacketDropVisualization *packetDropVisualization)
{
    PacketDropVisualizerBase::addPacketDropVisualization(packetDropVisualization);
    auto packetDropOsgVisualization = static_cast<const PacketDropOsgVisualization *>(packetDropVisualization);
    auto scene = inet::osg::TopLevelScene::getSimulationScene(visualizerTargetModule);
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
    auto positionAttitudeTransform = static_cast<osg::PositionAttitudeTransform *>(packetDropOsgVisualization->node);
    auto autoTransform = static_cast<osg::AutoTransform *>(positionAttitudeTransform->getChild(0));
    auto geode = static_cast<osg::Geode *>(autoTransform->getChild(0));
    auto material = static_cast<osg::Material *>(geode->getOrCreateStateSet()->getAttribute(osg::StateAttribute::MATERIAL));
    material->setAlpha(osg::Material::FRONT_AND_BACK, alpha);
    double dx = 10 / alpha;
    double dy = 10 / alpha;
    double dz = 58 - pow((dx / 4 - 9), 2);
    positionAttitudeTransform->setPosition(osg::Vec3d(packetDropVisualization->position.x + dx, packetDropVisualization->position.y + dy, packetDropVisualization->position.z + dz));
}

#endif // ifdef WITH_OSG

} // namespace visualizer

} // namespace inet

