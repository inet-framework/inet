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
// along with this program; if not, see http://www.gnu.org/licenses/.
//

#include <algorithm>
#include "inet/common/OmittedModule.h"
#include "inet/common/SubmoduleLayout.h"

namespace inet {

static double getPosition(cModule *submodule, int dimensionIndex)
{
    auto& displayString = submodule->getDisplayString();
    return atof(displayString.getTagArg("p", dimensionIndex));
}

void layoutSubmodulesWithoutGates(cModule *module, int dimensionIndex, double moduleSpacing)
{
    double submodulePosition = moduleSpacing;
    for (cModule::SubmoduleIterator it(module); !it.end(); it++) {
        auto submodule = *it;
        if (submodule->getGateNames().empty()) {
            auto& displayString = submodule->getDisplayString();
            displayString.setTagArg("p", dimensionIndex, submodulePosition);
            submodulePosition += moduleSpacing;
        }
    }
}

void layoutSubmodulesWithGates(cModule *module, int dimensionIndex, double moduleSpacing)
{
    std::vector<cModule *> submodules;
    for (cModule::SubmoduleIterator it(module); !it.end(); it++) {
        auto submodule = *it;
        if (!dynamic_cast<OmittedModule *>(submodule) && !submodule->getGateNames().empty())
            submodules.push_back(*it);
    }
    std::sort(submodules.begin(), submodules.end(), [&] (cModule *s1, cModule *s2) {
        return getPosition(s1, dimensionIndex) < getPosition(s2, dimensionIndex);
    });
    for (int i = 0; i < (int)submodules.size(); i++) {
        auto submodule = submodules[i];
        double maxPosition = 0;
        for (cModule::GateIterator it(submodule); !it.end(); it++) {
            auto gate = *it;
            cModule *connectedModule = nullptr;
            if (gate->getType() == cGate::INPUT) {
                if (auto g = gate->getPreviousGate())
                    connectedModule = check_and_cast<cModule *>(g->getOwner());
            }
            else if (gate->getType() == cGate::OUTPUT) {
                if (auto g = gate->getNextGate())
                    connectedModule = check_and_cast<cModule *>(g->getOwner());
            }
            else
                throw cRuntimeError("Unknown gate type");
            auto jt = std::find(submodules.begin(), submodules.end(), connectedModule);
            if (jt != submodules.end() && jt - submodules.begin() < i) {
                auto connectedPosition = getPosition(connectedModule, dimensionIndex);
                maxPosition = std::max(maxPosition, connectedPosition);
            }
        }
        auto submodulePosition = maxPosition + moduleSpacing;
        auto& displayString = submodule->getDisplayString();
        displayString.setTagArg("p", dimensionIndex, submodulePosition);
    }
}

void layoutSubmodules(cModule *module, int dimensionIndex, double moduleSpacing)
{
    layoutSubmodulesWithoutGates(module, dimensionIndex, moduleSpacing);
    layoutSubmodulesWithGates(module, dimensionIndex, moduleSpacing);
}

} // namespace inet

