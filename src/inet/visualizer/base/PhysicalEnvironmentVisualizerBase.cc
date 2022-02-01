//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/visualizer/base/PhysicalEnvironmentVisualizerBase.h"

#include "inet/common/ModuleAccess.h"

namespace inet {

namespace visualizer {

using namespace inet::physicalenvironment;

void PhysicalEnvironmentVisualizerBase::initialize(int stage)
{
    VisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        physicalEnvironment = dynamic_cast<IPhysicalEnvironment *>(findModuleByPath(par("physicalEnvironmentModule")));
        displayObjects = par("displayObjects");
    }
}

} // namespace visualizer

} // namespace inet

