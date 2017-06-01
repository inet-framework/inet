//
// Copyright (C) OpenSim Ltd.
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

#include "inet/common/ModuleAccess.h"
#include "inet/environment/contract/IPhysicalEnvironment.h"
#include "inet/mobility/contract/IMobility.h"
#include "inet/visualizer/base/SceneVisualizerBase.h"

namespace inet {

namespace visualizer {

using namespace inet::physicalenvironment;

void SceneVisualizerBase::initialize(int stage)
{
    VisualizerBase::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL)
        visualizerTargetModule->getCanvas()->setAnimationSpeed(par("animationSpeed"), this);
}

Box SceneVisualizerBase::getPlaygroundBounds()
{
    auto physicalEnvironment = getModuleFromPar<IPhysicalEnvironment>(par("physicalEnvironmentModule"), this, false);
    Coord playgroundMin = Coord::ZERO;
    Coord playgroundMax = Coord::ZERO;
    if (physicalEnvironment == nullptr) {
        auto displayString = visualizerTargetModule->getDisplayString();
        auto width = atof(displayString.getTagArg("bgb", 0));
        auto height = atof(displayString.getTagArg("bgb", 1));
        playgroundMin = Coord(0.0, 0.0, 0.0);
        playgroundMax = Coord(width, height, 0.0);
    }
    else {
        playgroundMin = physicalEnvironment->getSpaceMin();
        playgroundMax = physicalEnvironment->getSpaceMax();
    }
    for (int id = 0; id <= getSimulation()->getLastComponentId(); id++) {
        auto mobility = dynamic_cast<IMobility *>(getSimulation()->getModule(id));
        if (mobility != nullptr) {
            playgroundMin = playgroundMin.min(mobility->getConstraintAreaMin());
            playgroundMax = playgroundMax.max(mobility->getConstraintAreaMax());
        }
    }
    return Box(playgroundMin, playgroundMax);
}

} // namespace visualizer

} // namespace inet

