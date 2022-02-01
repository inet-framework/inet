//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/SubmoduleLayout.h"

#include <algorithm>

#include "inet/common/IPrintableObject.h"
#include "inet/common/OmittedModule.h"
#include "inet/common/stlutils.h"

namespace inet {

static double getPosition(cModule *submodule, int dimensionIndex)
{
    auto& displayString = submodule->getDisplayString();
    return atof(displayString.getTagArg("p", dimensionIndex));
}

void layoutSubmodulesWithoutGates(cModule *module, int dimensionIndex, double moduleSpacing)
{
    double position = moduleSpacing;
    for (cModule::SubmoduleIterator it(module); !it.end(); it++) {
        auto submodule = *it;
        if (submodule->getGateNames().empty()) {
            auto& displayString = submodule->getDisplayString();
            const char *dimension = dimensionIndex == 0 ? "x" : "y";
            EV_DEBUG << "Setting submodule position" << EV_FIELD(submodule, submodule->getFullPath()) << EV_FIELD(dimension) << EV_FIELD(position) << EV_ENDL;
            displayString.setTagArg("p", dimensionIndex, position);
            position += moduleSpacing;
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
        if (*submodule->getDisplayString().getTagArg("p", 2) != '\0')
            continue;
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
            auto jt = find(submodules, connectedModule);
            if (jt != submodules.end() && jt - submodules.begin() < i) {
                auto connectedPosition = getPosition(connectedModule, dimensionIndex);
                maxPosition = std::max(maxPosition, connectedPosition);
            }
        }
        for (int j = 0; j < i; j++) {
            auto alignedSubmodule = submodules[j];
            if (getPosition(submodule, 1 - dimensionIndex) == getPosition(alignedSubmodule, 1 - dimensionIndex)) {
                auto alignedPosition = getPosition(alignedSubmodule, dimensionIndex);
                maxPosition = std::max(maxPosition, alignedPosition);
            }
        }
        auto position = maxPosition + moduleSpacing;
        auto& displayString = submodule->getDisplayString();
        const char *dimension = dimensionIndex == 0 ? "x" : "y";
        EV_INFO << "Setting submodule position" << EV_FIELD(submodule, submodule->getFullPath()) << EV_FIELD(dimension) << EV_FIELD(position) << EV_ENDL;
        displayString.setTagArg("p", dimensionIndex, position);
    }
}

void layoutSubmodules(cModule *module, int dimensionIndex, double moduleSpacing)
{
    layoutSubmodulesWithoutGates(module, dimensionIndex, moduleSpacing);
    layoutSubmodulesWithGates(module, dimensionIndex, moduleSpacing);
}

} // namespace inet

