//
// Copyright (C) 2006-2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/osg/util/OsgScene.h"

namespace inet {

namespace osg {

template<typename T>
void FindNodesVisitor<T>::apply(Node& node) {
    T *result = dynamic_cast<T *>(&node);
    if (result)
        foundNodes.push_back(result);
    traverse(node);
}

SimulationScene *TopLevelScene::getSimulationScene()
{
    if (simulationScene == nullptr) {
        FindNodesVisitor<SimulationScene> visitor;
        accept(visitor);
        auto foundNodes = visitor.getFoundNodes();
        ASSERT(foundNodes.size() == 1);
        simulationScene = check_and_cast<SimulationScene *>(foundNodes[0]);
    }
    return simulationScene;
}

SimulationScene *TopLevelScene::getSimulationScene(cModule *module)
{
    auto osgCanvas = module->getOsgCanvas();
    auto topLevelScene = dynamic_cast<TopLevelScene *>(osgCanvas->getScene());
    if (topLevelScene != nullptr)
        return topLevelScene->getSimulationScene();
    else {
        auto simulationScene = new SimulationScene();
        topLevelScene = new TopLevelScene();
        topLevelScene->addChild(simulationScene);
        // NOTE: these are the default values when there's no SceneOsgVisualizer
        osgCanvas->setScene(topLevelScene);
        osgCanvas->setClearColor(cFigure::Color("#FFFFFF"));
        osgCanvas->setZNear(0.1);
        osgCanvas->setZFar(100000);
        osgCanvas->setCameraManipulatorType(cOsgCanvas::CAM_TERRAIN);
        return simulationScene;
    }
}

} // namespace osg

} // namespace inet

