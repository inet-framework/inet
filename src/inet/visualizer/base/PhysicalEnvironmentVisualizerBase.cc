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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#include "inet/common/ModuleAccess.h"
#include "inet/visualizer/base/PhysicalEnvironmentVisualizerBase.h"

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

