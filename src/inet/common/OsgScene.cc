//
// Copyright (C) 2006-2015 Opensim Ltd
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/common/OsgScene.h"

namespace inet {

namespace osg {

#if defined(WITH_OSG) && defined(WITH_VISUALIZERS)

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

#endif // ifdef WITH_OSG

} // namespace osg

} // namespace inet

