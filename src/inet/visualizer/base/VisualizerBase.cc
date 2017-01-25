//
// Copyright (C) 2016 OpenSim Ltd.
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
#include "inet/mobility/contract/IMobility.h"
#include "inet/visualizer/base/VisualizerBase.h"

namespace inet {

namespace visualizer {

void VisualizerBase::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (!hasGUI()) return;
    if (stage == INITSTAGE_LOCAL) {
        const char *path = par("visualizerTargetModule");
        visualizerTargetModule = *path == '\0' ? getSystemModule() : getModuleByPath(path);
        if (visualizerTargetModule == nullptr)
            throw cRuntimeError("Module not found on path '%s' defined by par 'visualizerTargetModule'", path);
    }
}

Coord VisualizerBase::getPosition(cModule *node) const
{
    auto mobility = node->getSubmodule("mobility");
    if (mobility == nullptr) {
        double x, y;
        cDisplayString displayString = node->getDisplayString();
        x = atof(displayString.getTagArg("p", 0));
        y = atof(displayString.getTagArg("p", 1));
        return Coord(x, y, 0);
    }
    else
        return check_and_cast<IMobility *>(mobility)->getCurrentPosition();
}

} // namespace visualizer

} // namespace inet

