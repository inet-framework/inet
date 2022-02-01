//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/osg/scene/NetworkConnectionOsgVisualizer.h"

#include <osg/Geode>
#include <osg/LineWidth>

#include "inet/common/ModuleAccess.h"
#include "inet/visualizer/osg/util/OsgScene.h"
#include "inet/visualizer/osg/util/OsgUtils.h"

namespace inet {

namespace visualizer {

Define_Module(NetworkConnectionOsgVisualizer);

void NetworkConnectionOsgVisualizer::initialize(int stage)
{
    NetworkConnectionVisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
    }
}

void NetworkConnectionOsgVisualizer::createNetworkConnectionVisualization(cModule *startNetworkNode, cModule *endNetworkNode)
{
    auto geode = new osg::Geode();
    auto drawable = inet::osg::createLineGeometry(getPosition(startNetworkNode), getPosition(endNetworkNode));
    geode->addDrawable(drawable);
    auto stateSet = inet::osg::createLineStateSet(lineColor, lineStyle, lineWidth);
    stateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
    geode->setStateSet(stateSet);
    auto lineWidth = new osg::LineWidth();
    lineWidth->setWidth(this->lineWidth);
    stateSet->setAttributeAndModes(lineWidth, osg::StateAttribute::ON);
    auto scene = inet::osg::TopLevelScene::getSimulationScene(visualizationTargetModule);
    scene->addChild(geode);
}

} // namespace visualizer

} // namespace inet

