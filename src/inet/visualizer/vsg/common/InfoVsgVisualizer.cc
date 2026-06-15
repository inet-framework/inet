//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/vsg/common/InfoVsgVisualizer.h"

#include "inet/common/ModuleAccess.h"
#include "inet/visualizer/vsg/util/VsgUtils.h"

namespace inet {

namespace visualizer {

Define_Module(InfoVsgVisualizer);

InfoVsgVisualizer::InfoVsgVisualization::InfoVsgVisualization(NetworkNodeVsgVisualization *networkNodeVisualization, ::vsg::ref_ptr<::vsg::Group> node, int moduleId) :
    InfoVisualization(moduleId),
    networkNodeVisualization(networkNodeVisualization),
    node(node)
{
}

void InfoVsgVisualizer::initialize(int stage)
{
    InfoVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL)
        networkNodeVisualizer.reference(this, "networkNodeVisualizerModule", true);
    else if (stage == INITSTAGE_LAST) {
        for (auto infoVisualization : infoVisualizations) {
            auto infoVsgVisualization = static_cast<const InfoVsgVisualization *>(infoVisualization);
            infoVsgVisualization->networkNodeVisualization->addAnnotation(infoVsgVisualization->node, ::vsg::dvec3(0, 0, 0), 0);
        }
    }
}

InfoVisualizerBase::InfoVisualization *InfoVsgVisualizer::createInfoVisualization(cModule *module) const
{
    auto node = ::vsg::Group::create();   // text is (re)built into this container on refresh
    auto networkNode = getContainingNode(module);
    auto networkNodeVisualization = networkNodeVisualizer->getNetworkNodeVisualization(networkNode);
    return new InfoVsgVisualization(networkNodeVisualization, node, module->getId());
}

void InfoVsgVisualizer::refreshInfoVisualization(const InfoVisualization *infoVisualization, const char *info) const
{
    auto infoVsgVisualization = static_cast<const InfoVsgVisualization *>(infoVisualization);
    if (infoVsgVisualization->lastInfo == info)
        return; // unchanged -> avoid rebuilding the text (VSG has no in-place text edit here)
    infoVsgVisualization->lastInfo = info;
    infoVsgVisualization->node->children.clear();
    if (info != nullptr && *info != '\0')
        infoVsgVisualization->node->addChild(inet::vsg::createText(info, Coord::ZERO, textColor, 18));
}

} // namespace visualizer

} // namespace inet
