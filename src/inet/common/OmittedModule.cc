//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/common/OmittedModule.h"

#include "inet/common/SubmoduleLayout.h"

namespace inet {

Define_Module(OmittedModule);

void OmittedModule::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        for (cModule::GateIterator ig(this); !ig.end(); ++ig) {
            auto gateIn = *ig;
            if (gateIn->getType() == cGate::INPUT && gateIn->findTransmissionChannel() == nullptr) {
                auto gateOut = gateIn->getNextGate();
                auto previousGate = gateIn->getPreviousGate();
                auto nextGate = gateOut->getNextGate();
                if (previousGate != nullptr && nextGate != nullptr) {
                    EV_INFO << "Reconnecting gates: " << previousGate->getFullPath() << " --> " << nextGate->getFullPath() << std::endl;
                    std::string direction;
                    if (previousGate->getChannel()) {
                        direction = opp_nulltoempty(previousGate->getDisplayString().getTagArg("m", 0));
                    }
                    if (direction.empty() && gateOut->getChannel())
                        direction = opp_nulltoempty(gateOut->getDisplayString().getTagArg("m", 0));
                    gateOut->disconnect();
                    previousGate->disconnect();
                    previousGate->connectTo(nextGate);
                    if (!direction.empty())
                        previousGate->getDisplayString().setTagArg("m", 0, direction.c_str());
                }
            }
        }
    }
}

bool OmittedModule::initializeModules(int stage)
{
    bool result = cModule::initializeModules(stage);
    if (stage == INITSTAGE_LAST) {
        auto parentModule = getParentModule();
        deleteModule();
        layoutSubmodulesWithGates(parentModule);
    }
    return result;
}

} // namespace inet

