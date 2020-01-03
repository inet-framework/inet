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

#include "inet/common/OsgScene.h"
#include "inet/common/OsgUtils.h"
#include "inet/visualizer/linklayer/LinkBreakOsgVisualizer.h"

#ifdef WITH_OSG
#include <osg/Geode>
#include <osg/LineWidth>
#endif // ifdef WITH_OSG

namespace inet {

namespace visualizer {

Define_Module(LinkBreakOsgVisualizer);

#ifdef WITH_OSG

LinkBreakOsgVisualizer::LinkBreakOsgVisualization::LinkBreakOsgVisualization(osg::Node *node, int transmitterModuleId, int receiverModuleId) :
    LinkBreakVisualization(transmitterModuleId, receiverModuleId),
    node(node)
{
}

LinkBreakOsgVisualizer::LinkBreakOsgVisualization::~LinkBreakOsgVisualization()
{
    // TODO: delete node;
}

LinkBreakOsgVisualizer::~LinkBreakOsgVisualizer()
{
    if (displayLinkBreaks)
        removeAllLinkBreakVisualizations();
}

void LinkBreakOsgVisualizer::refreshDisplay() const
{
    LinkBreakVisualizerBase::refreshDisplay();
    // TODO: switch to osg canvas when API is extended
    visualizationTargetModule->getCanvas()->setAnimationSpeed(linkBreakVisualizations.empty() ? 0 : fadeOutAnimationSpeed, this);
}

const LinkBreakVisualizerBase::LinkBreakVisualization *LinkBreakOsgVisualizer::createLinkBreakVisualization(cModule *transmitter, cModule *receiver) const
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
    auto geode = new osg::Geode();
    geode->addDrawable(geometry);
    auto material = new osg::Material();
    osg::Vec4 colorVec((double)iconTintColor.red / 255.0, (double)iconTintColor.green / 255.0, (double)iconTintColor.blue / 255.0, 1.0);
    material->setAmbient(osg::Material::FRONT_AND_BACK, colorVec);
    material->setDiffuse(osg::Material::FRONT_AND_BACK, colorVec);
    material->setAlpha(osg::Material::FRONT_AND_BACK, 1.0);
    geode->getOrCreateStateSet()->setAttribute(material);
    // TODO: apply tinting
    return new LinkBreakOsgVisualization(geode, transmitter->getId(), receiver->getId());
}

void LinkBreakOsgVisualizer::addLinkBreakVisualization(const LinkBreakVisualization *linkBreakVisualization)
{
    LinkBreakVisualizerBase::addLinkBreakVisualization(linkBreakVisualization);
    auto linkBreakOsgVisualization = static_cast<const LinkBreakOsgVisualization *>(linkBreakVisualization);
    auto scene = inet::osg::TopLevelScene::getSimulationScene(visualizationTargetModule);
    scene->addChild(linkBreakOsgVisualization->node);
}

void LinkBreakOsgVisualizer::removeLinkBreakVisualization(const LinkBreakVisualization *linkBreak)
{
    LinkBreakVisualizerBase::removeLinkBreakVisualization(linkBreak);
    auto linkBreakOsgVisualization = static_cast<const LinkBreakOsgVisualization *>(linkBreak);
    auto node = linkBreakOsgVisualization->node;
    node->getParent(0)->removeChild(node);
}

void LinkBreakOsgVisualizer::setAlpha(const LinkBreakVisualization *linkBreakVisualization, double alpha) const
{
    auto linkBreakOsgVisualization = static_cast<const LinkBreakOsgVisualization *>(linkBreakVisualization);
    auto node = linkBreakOsgVisualization->node;
    auto material = static_cast<osg::Material *>(node->getOrCreateStateSet()->getAttribute(osg::StateAttribute::MATERIAL));
    material->setAlpha(osg::Material::FRONT_AND_BACK, alpha);
}

#endif // ifdef WITH_OSG

} // namespace visualizer

} // namespace inet

