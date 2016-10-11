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
#include "inet/visualizer/linklayer/LinkBreakVOsgVisualizer.h"

#ifdef WITH_OSG
#include <osg/Geode>
#include <osg/LineWidth>
#endif // ifdef WITH_OSG

namespace inet {

namespace visualizer {

Define_Module(LinkBreakOsgVisualizer);

LinkBreakOsgVisualizer::OsgLinkBreak::OsgLinkBreak(osg::Node *node, int transmitterModuleId, int receiverModuleId, simtime_t breakSimulationTime, double breakAnimationTime, double breakRealTime) :
    LinkBreakVisualization(transmitterModuleId, receiverModuleId, breakSimulationTime, breakAnimationTime, breakRealTime),
    node(node)
{
}

LinkBreakOsgVisualizer::OsgLinkBreak::~OsgLinkBreak()
{
    // TODO: delete node;
}

void LinkBreakOsgVisualizer::setPosition(cModule *node, const Coord& position) const
{
    // TODO:
}

void LinkBreakOsgVisualizer::setAlpha(const LinkBreakVisualization *linkBreak, double alpha) const
{
    auto osgLinkBreak = static_cast<const OsgLinkBreak *>(linkBreak);
    auto node = osgLinkBreak->node;
    auto material = static_cast<osg::Material *>(node->getOrCreateStateSet()->getAttribute(osg::StateAttribute::MATERIAL));
    material->setAlpha(osg::Material::FRONT_AND_BACK, alpha);
}

const LinkBreakVisualizerBase::LinkBreakVisualization *LinkBreakOsgVisualizer::createLinkBreakVisualization(cModule *transmitter, cModule *receiver) const
{
    auto path = resolveResourcePath(icon);
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
    // TODO: apply tinting
    return new OsgLinkBreak(geode, transmitter->getId(), receiver->getId(), simTime(), getSimulation()->getEnvir()->getAnimationTime(), getRealTime());
}

void LinkBreakOsgVisualizer::addLinkBreakVisualization(const LinkBreakVisualization *linkBreak)
{
    LinkBreakVisualizerBase::addLinkBreakVisualization(linkBreak);
    auto osgLinkBreak = static_cast<const OsgLinkBreak *>(linkBreak);
    auto scene = inet::osg::TopLevelScene::getSimulationScene(visualizerTargetModule);
    scene->addChild(osgLinkBreak->node);
}

void LinkBreakOsgVisualizer::removeLinkBreakVisualization(const LinkBreakVisualization *linkBreak)
{
    LinkBreakVisualizerBase::removeLinkBreakVisualization(linkBreak);
    auto osgLinkBreak = static_cast<const OsgLinkBreak *>(linkBreak);
    auto node = osgLinkBreak->node;
    node->getParent(0)->removeChild(node);
}

} // namespace visualizer

} // namespace inet

