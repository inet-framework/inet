//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/vsg/common/StatisticVsgVisualizer.h"

#include "inet/common/ModuleAccess.h"
#include "inet/visualizer/vsg/util/VsgUtils.h"

namespace inet {

namespace visualizer {

Define_Module(StatisticVsgVisualizer);

StatisticVsgVisualizer::StatisticVsgVisualization::StatisticVsgVisualization(NetworkNodeVsgVisualization *networkNodeVisualization, ::vsg::ref_ptr<::vsg::Group> node, int moduleId, simsignal_t signal, const char *unit) :
    StatisticVisualization(moduleId, signal, unit),
    networkNodeVisualization(networkNodeVisualization),
    node(node)
{
}

void StatisticVsgVisualizer::initialize(int stage)
{
    StatisticVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL)
        networkNodeVisualizer.reference(this, "networkNodeVisualizerModule", true);
}

StatisticVisualizerBase::StatisticVisualization *StatisticVsgVisualizer::createStatisticVisualization(cComponent *source, simsignal_t signal)
{
    auto node = ::vsg::Group::create();
    auto networkNode = getContainingNode(check_and_cast<cModule *>(source));
    auto networkNodeVisualization = networkNodeVisualizer->getNetworkNodeVisualization(networkNode);
    return new StatisticVsgVisualization(networkNodeVisualization, node, source->getId(), signal, getUnit(source));
}

void StatisticVsgVisualizer::addStatisticVisualization(const StatisticVisualization *statisticVisualization)
{
    StatisticVisualizerBase::addStatisticVisualization(statisticVisualization);
    auto statisticVsgVisualization = static_cast<const StatisticVsgVisualization *>(statisticVisualization);
    statisticVsgVisualization->networkNodeVisualization->addAnnotation(statisticVsgVisualization->node, ::vsg::dvec3(100, 18, 0), 1.0);
}

void StatisticVsgVisualizer::removeStatisticVisualization(const StatisticVisualization *statisticVisualization)
{
    StatisticVisualizerBase::removeStatisticVisualization(statisticVisualization);
    auto statisticVsgVisualization = static_cast<const StatisticVsgVisualization *>(statisticVisualization);
    if (networkNodeVisualizer != nullptr)
        statisticVsgVisualization->networkNodeVisualization->removeAnnotation(statisticVsgVisualization->node);
}

void StatisticVsgVisualizer::refreshStatisticVisualization(const StatisticVisualization *statisticVisualization)
{
    StatisticVisualizerBase::refreshStatisticVisualization(statisticVisualization);
    auto statisticVsgVisualization = static_cast<const StatisticVsgVisualization *>(statisticVisualization);
    std::string text = getText(statisticVisualization);
    if (statisticVsgVisualization->lastText == text)
        return; // unchanged — avoid rebuilding the label
    statisticVsgVisualization->lastText = text;
    statisticVsgVisualization->node->children.clear();
    if (!text.empty())
        statisticVsgVisualization->node->addChild(inet::vsg::createText(text.c_str(), Coord::ZERO, textColor, 18));
}

} // namespace visualizer

} // namespace inet
