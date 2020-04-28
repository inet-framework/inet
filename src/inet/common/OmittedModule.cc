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

#include "inet/common/OmittedModule.h"
#include "inet/common/SubmoduleLayout.h"

namespace inet {

Define_Module(OmittedModule);

void OmittedModule::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        for (cModule::GateIterator ig(this); !ig.end(); ig++) {
            auto gateIn = *ig;
            if (gateIn->getType() == cGate::INPUT && gateIn->findTransmissionChannel() == nullptr) {
                auto gateOut = gateIn->getNextGate();
                auto previousGate = gateIn->getPreviousGate();
                auto nextGate = gateOut->getNextGate();
                EV_INFO << "Reconnecting gates: " << previousGate->getFullPath() << " --> " << nextGate->getFullPath() << std::endl;
                gateOut->disconnect();
                previousGate->disconnect();
                previousGate->connectTo(nextGate);
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

