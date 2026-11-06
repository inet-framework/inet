//
// Copyright (C) 2006-2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/vsg/util/VsgScene.h"

#include <omnetpp/cvsgcanvas.h>

namespace inet {

namespace vsg {

SimulationScene *TopLevelScene::getSimulationScene()
{
    if (simulationScene == nullptr) {
        FindNodesVisitor<SimulationScene> visitor;
        accept(visitor);
        auto foundNodes = visitor.getFoundNodes();
        ASSERT(foundNodes.size() == 1);
        simulationScene = foundNodes[0];
    }
    return simulationScene;
}

SimulationScene *TopLevelScene::getSimulationScene(cModule *module)
{
    auto canvas = module->getVsgCanvas();
    auto topLevelScene = dynamic_cast<TopLevelScene *>(canvas->getScene());
    if (topLevelScene != nullptr)
        return topLevelScene->getSimulationScene();
    else {
        auto simulationScene = SimulationScene::create();
        auto newTopLevelScene = TopLevelScene::create();
        newTopLevelScene->addChild(simulationScene);
        // NOTE: these are the default values when there's no SceneVsgVisualizer
        canvas->setScene(newTopLevelScene);
        canvas->setClearColor(cFigure::Color("#FFFFFF"));
        canvas->setZNear(0.1);
        canvas->setZFar(100000);
        canvas->setCameraManipulatorType(cVsgCanvas::CAM_TERRAIN);
        return simulationScene;
    }
}

} // namespace vsg

} // namespace inet
