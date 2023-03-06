//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/queueing/gate/GateControlList.h"

namespace inet {
namespace queueing {

Define_Module(GateControlList);

void GateControlList::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL) {
        numGates = par("numGates");
        offset = par("offset");
        durations = check_and_cast<cValueArray *>(par("durations").objectValue());
        gateStates = check_and_cast<cValueArray *>(par("gateStates").objectValue());

        parseGcl();

        for (int index = 0; index < numGates; ++index) {
            std::string modulePath = "^.transmissionGate[" + std::to_string(index) + "]";
            PeriodicGate *mod = check_and_cast<PeriodicGate *>(getModuleByPath(modulePath.c_str()));
            cPar& offsetPar = mod->par("offset");
            offsetPar.setDoubleValue(offset.dbl());
            cPar& durationsPar = mod->par("durations");
            durationsPar.copyIfShared();
            durationsPar.setObjectValue(durations);
        }
    }
}

void GateControlList::handleMessage(cMessage *msg)
{
    throw cRuntimeError("Do not handle cMessage");
}

void GateControlList::parseGcl() {
    if (durations->size() != gateStates->size()) {
        throw cRuntimeError("The length of durations is not equal to gateStates.");
    }

    for (int i = 0; i < numGates; ++i) {
        gateDurations.emplace_back(new cValueArray());
    }

    int numEntries = durations->size();
    std::vector<bool> currentGateStates(numGates, true);
    std::vector<double> currentDuration(numGates, 0);

    for (int indexEntry = 0; indexEntry < numEntries; ++indexEntry) {
        const char *curGateStates = gateStates->get(indexEntry).stringValue();
        double duration = durations->get(indexEntry).doubleValueInUnit("s");

        std::vector<bool> entry = retrieveGateStates(curGateStates, numGates);

        for (int i = 0; i < numGates; ++i) {
            if (entry.at(i) != currentGateStates.at(i)) {
                gateDurations.at(i)->add(cValue(currentDuration[i], "s"));

                currentGateStates[i] = !currentGateStates[i];
                currentDuration[i] = 0;
            }

            currentDuration[i] += duration;
        }

    }

    for (int i = 0; i < numGates; ++i) {
        gateDurations.at(i)->add(cValue(currentDuration[i], "s"));

        if (gateDurations.at(i)->size() % 2 != 0) {
            gateDurations.at(i)->add(cValue(0.0, "s"));
        }
    }

}

std::vector<bool> GateControlList::retrieveGateStates(const char *gateStates, uint numGates) {
    std::vector<bool> res(numGates);

    if (strlen(gateStates) != numGates) {
        throw cRuntimeError("The length of the entry is not equal to numGates.");
    }

    for (int indexGate = 0; indexGate < numGates; ++indexGate) {
        char ch = *(gateStates + indexGate);
        if (ch == '1') {
            res[numGates - indexGate - 1] = true;
        } else if (ch == '0') {
            res[numGates - indexGate - 1] = false;
        } else {
            throw cRuntimeError("Unknown char");
        }
    }
    return res;
}

GateControlList::~GateControlList() {
    for (int i = 0; i < numGates; ++i) {
        delete gateDurations.at(i);
    }
}

} // namespace queueing
} // namespace inet
