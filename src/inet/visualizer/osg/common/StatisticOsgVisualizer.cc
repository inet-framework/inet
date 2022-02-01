//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/osg/common/StatisticOsgVisualizer.h"

#include "inet/common/ModuleAccess.h"
#include "inet/visualizer/osg/util/OsgUtils.h"

namespace inet {

namespace visualizer {

Define_Module(StatisticOsgVisualizer);

StatisticOsgVisualizer::StatisticOsgVisualization::StatisticOsgVisualization(NetworkNodeOsgVisualization *networkNodeVisualization, osg::Node *node, int moduleId, simsignal_t signal, const char *unit) :
    StatisticVisualization(moduleId, signal, unit),
    networkNodeVisualization(networkNodeVisualization),
    node(node)
{
}

void StatisticOsgVisualizer::initialize(int stage)
{
    StatisticVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        networkNodeVisualizer.reference(this, "networkNodeVisualizerModule", true);
    }
}

StatisticVisualizerBase::StatisticVisualization *StatisticOsgVisualizer::createStatisticVisualization(cComponent *source, simsignal_t signal)
{
    auto label = new osgText::Text();
    label->setCharacterSize(18);
    label->setBoundingBoxColor(osg::Vec4(backgroundColor.red / 255.0, backgroundColor.green / 255.0, backgroundColor.blue / 255.0, 0.5));
    label->setColor(osg::Vec4(textColor.red / 255.0, textColor.green / 255.0, textColor.blue / 255.0, 0.5));
    label->setAlignment(osgText::Text::CENTER_BOTTOM);
    label->setText("");
    label->setDrawMode(osgText::Text::FILLEDBOUNDINGBOX | osgText::Text::TEXT);
    label->setPosition(osg::Vec3(0.0, 0.0, 0.0));
    auto geode = new osg::Geode();
    geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    geode->addDrawable(label);
    auto networkNode = getContainingNode(check_and_cast<cModule *>(source));
    auto networkNodeVisualization = networkNodeVisualizer->getNetworkNodeVisualization(networkNode);
    return new StatisticOsgVisualization(networkNodeVisualization, geode, source->getId(), signal, getUnit(source));
}

void StatisticOsgVisualizer::addStatisticVisualization(const StatisticVisualization *statisticVisualization)
{
    StatisticVisualizerBase::addStatisticVisualization(statisticVisualization);
    auto statisticOsgVisualization = static_cast<const StatisticOsgVisualization *>(statisticVisualization);
    statisticOsgVisualization->networkNodeVisualization->addAnnotation(statisticOsgVisualization->node, osg::Vec3d(100, 18, 0), 1.0);
}

void StatisticOsgVisualizer::removeStatisticVisualization(const StatisticVisualization *statisticVisualization)
{
    StatisticVisualizerBase::removeStatisticVisualization(statisticVisualization);
    auto statisticOsgVisualization = static_cast<const StatisticOsgVisualization *>(statisticVisualization);
    if (networkNodeVisualizer != nullptr)
        statisticOsgVisualization->networkNodeVisualization->removeAnnotation(statisticOsgVisualization->node);
}

void StatisticOsgVisualizer::refreshStatisticVisualization(const StatisticVisualization *statisticVisualization)
{
    StatisticVisualizerBase::refreshStatisticVisualization(statisticVisualization);
    auto statisticOsgVisualization = static_cast<const StatisticOsgVisualization *>(statisticVisualization);
    auto geode = check_and_cast<osg::Geode *>(statisticOsgVisualization->node);
    auto label = check_and_cast<osgText::Text *>(geode->getDrawable(0));
    label->setText(getText(statisticVisualization));
}

} // namespace visualizer

} // namespace inet

