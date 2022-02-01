//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/osg/common/InfoOsgVisualizer.h"

#include "inet/common/ModuleAccess.h"

namespace inet {

namespace visualizer {

Define_Module(InfoOsgVisualizer);

InfoOsgVisualizer::InfoOsgVisualization::InfoOsgVisualization(NetworkNodeOsgVisualization *networkNodeVisualization, osg::Geode *node, int moduleId) :
    InfoVisualization(moduleId),
    networkNodeVisualization(networkNodeVisualization),
    node(node)
{
}

void InfoOsgVisualizer::initialize(int stage)
{
    InfoVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        networkNodeVisualizer.reference(this, "networkNodeVisualizerModule", true);
    }
    else if (stage == INITSTAGE_LAST) {
        for (auto infoVisualization : infoVisualizations) {
            auto infoOsgVisualization = static_cast<const InfoOsgVisualization *>(infoVisualization);
            auto node = infoOsgVisualization->node;
            infoOsgVisualization->networkNodeVisualization->addAnnotation(node, osg::Vec3d(0, 0, 0), 0);
        }
    }
}

InfoVisualizerBase::InfoVisualization *InfoOsgVisualizer::createInfoVisualization(cModule *module) const
{
    auto text = new osgText::Text();
    text->setCharacterSize(18);
    text->setBoundingBoxColor(osg::Vec4(backgroundColor.red / 255.0, backgroundColor.green / 255.0, backgroundColor.blue / 255.0, 0.5));
    text->setColor(osg::Vec4(textColor.red / 255.0, textColor.green / 255.0, textColor.blue / 255.0, 1.0));
    text->setAlignment(osgText::Text::CENTER_BOTTOM);
    text->setText("");
    text->setDrawMode(osgText::Text::FILLEDBOUNDINGBOX | osgText::Text::TEXT);
    text->setPosition(osg::Vec3(0.0, 0.0, 0.0));
    auto geode = new osg::Geode();
    geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    geode->addDrawable(text);
    auto networkNode = getContainingNode(module);
    auto networkNodeVisualization = networkNodeVisualizer->getNetworkNodeVisualization(networkNode);
    return new InfoOsgVisualization(networkNodeVisualization, geode, module->getId());
}

void InfoOsgVisualizer::refreshInfoVisualization(const InfoVisualization *infoVisualization, const char *info) const
{
    auto infoOsgVisualization = static_cast<const InfoOsgVisualization *>(infoVisualization);
    auto node = infoOsgVisualization->node;
    auto text = static_cast<osgText::Text *>(node->getDrawable(0));
    text->setText(info);
}

} // namespace visualizer

} // namespace inet

