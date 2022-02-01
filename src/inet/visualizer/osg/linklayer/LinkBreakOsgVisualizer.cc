//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/osg/linklayer/LinkBreakOsgVisualizer.h"

#include <osg/Geode>
#include <osg/LineWidth>

#include "inet/visualizer/osg/util/OsgScene.h"
#include "inet/visualizer/osg/util/OsgUtils.h"

namespace inet {

namespace visualizer {

Define_Module(LinkBreakOsgVisualizer);

LinkBreakOsgVisualizer::LinkBreakOsgVisualization::LinkBreakOsgVisualization(osg::Node *node, int transmitterModuleId, int receiverModuleId) :
    LinkBreakVisualization(transmitterModuleId, receiverModuleId),
    node(node)
{
}

void LinkBreakOsgVisualizer::refreshDisplay() const
{
    LinkBreakVisualizerBase::refreshDisplay();
    // TODO switch to osg canvas when API is extended
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
    // TODO apply tinting
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

} // namespace visualizer

} // namespace inet

