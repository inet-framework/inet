//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/vsg/scene/NetworkConnectionVsgVisualizer.h"

#include "inet/common/ModuleAccess.h"
#include "inet/visualizer/vsg/util/VsgScene.h"
#include "inet/visualizer/vsg/util/VsgUtils.h"

namespace inet {

namespace visualizer {

Define_Module(NetworkConnectionVsgVisualizer);

void NetworkConnectionVsgVisualizer::initialize(int stage)
{
    NetworkConnectionVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
    }
}

void NetworkConnectionVsgVisualizer::createNetworkConnectionVisualization(cModule *startNetworkNode, cModule *endNetworkNode)
{
    auto line = inet::vsg::createLine(getPosition(startNetworkNode), getPosition(endNetworkNode),
            cFigure::ARROW_NONE, cFigure::ARROW_NONE, lineColor, lineStyle, lineWidth);
    auto scene = inet::vsg::TopLevelScene::getSimulationScene(visualizationTargetModule);
    scene->addChild(line);
}

} // namespace visualizer

} // namespace inet
