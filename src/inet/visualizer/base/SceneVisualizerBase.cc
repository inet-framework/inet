//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/base/SceneVisualizerBase.h"

#include "inet/common/ModuleAccess.h"
#include "inet/environment/contract/IPhysicalEnvironment.h"
#include "inet/mobility/contract/IMobility.h"

namespace inet {

namespace visualizer {

using namespace inet::physicalenvironment;

void SceneVisualizerBase::initialize(int stage)
{
    VisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        visualizationTargetModule->getCanvas()->setAnimationSpeed(par("animationSpeed"), this);
        sceneMin.x = par("sceneMinX");
        sceneMin.y = par("sceneMinY");
        sceneMin.z = par("sceneMinZ");
        sceneMax.x = par("sceneMaxX");
        sceneMax.y = par("sceneMaxY");
        sceneMax.z = par("sceneMaxZ");
    }
}

Box SceneVisualizerBase::getSceneBounds()
{
    Coord min;
    Coord max;
    auto physicalEnvironment = findModuleFromPar<IPhysicalEnvironment>(par("physicalEnvironmentModule"), this);
    if (physicalEnvironment == nullptr) {
        auto displayString = visualizationTargetModule->getDisplayString();
        auto width = atof(displayString.getTagArg("bgb", 0));
        auto height = atof(displayString.getTagArg("bgb", 1));
        min = Coord(0.0, 0.0, 0.0);
        max = Coord(width, height, 0.0);
    }
    else {
        min = physicalEnvironment->getSpaceMin();
        max = physicalEnvironment->getSpaceMax();
    }
    for (int id = 0; id <= getSimulation()->getLastComponentId(); id++) {
        auto mobility = dynamic_cast<IMobility *>(getSimulation()->getModule(id));
        if (mobility != nullptr) {
            min = mobility->getConstraintAreaMin().min(min);
            max = mobility->getConstraintAreaMax().max(max);
        }
    }
    min = sceneMin.min(min);
    max = sceneMax.max(max);
    return Box(min, max);
}

} // namespace visualizer

} // namespace inet

