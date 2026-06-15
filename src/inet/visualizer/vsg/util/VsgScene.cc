//
// Copyright (C) 2006-2015 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/vsg/util/VsgScene.h"

#include "qtenv/vsg/vsgscenehandle.h"   // omnetpp::cScene3DNode / createScene3DNode

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
    auto osgCanvas = module->getOsgCanvas();
    // Under WITH_VSG the cOsgCanvas scene is an opaque omnetpp::cScene3DNode that
    // wraps the VSG scene-root group; recover our TopLevelScene from its root.
    auto sceneNode = osgCanvas->getScene();
    ::vsg::ref_ptr<Group> root = sceneNode != nullptr ? sceneNode->getRoot() : ::vsg::ref_ptr<Group>();
    auto topLevelScene = root ? dynamic_cast<TopLevelScene *>(root.get()) : nullptr;
    if (topLevelScene != nullptr)
        return topLevelScene->getSimulationScene();
    else {
        auto simulationScene = SimulationScene::create();
        auto newTopLevelScene = TopLevelScene::create();
        newTopLevelScene->addChild(simulationScene);
        // NOTE: these are the default values when there's no SceneVsgVisualizer
        osgCanvas->setScene(omnetpp::createScene3DNode(newTopLevelScene));
        osgCanvas->setClearColor(cFigure::Color("#FFFFFF"));
        osgCanvas->setZNear(0.1);
        osgCanvas->setZFar(100000);
        osgCanvas->setCameraManipulatorType(cOsgCanvas::CAM_TERRAIN);
        return simulationScene;
    }
}

} // namespace vsg

} // namespace inet
