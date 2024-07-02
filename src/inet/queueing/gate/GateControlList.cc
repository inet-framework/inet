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
        mapping = par("mapping");
        if (strlen(mapping) != numGates)
            throw cRuntimeError("The length of mapping is not equal to numGates");
        durations = check_and_cast<cValueArray *>(par("durations").objectValue());
        gateStates = check_and_cast<cValueArray *>(par("gateStates").objectValue());
        if (durations->size() != gateStates->size())
            throw cRuntimeError("The length of durations is not equal to the length of gateStates");
        parseGcl();
    }
    else if (stage == INITSTAGE_QUEUEING) {
        for (size_t i = 0; i < numGates; ++i) {
            std::string modulePath = "^.transmissionGate[" + std::to_string(i) + "]";
            PeriodicGate *periodicGate = check_and_cast<PeriodicGate *>(getModuleByPath(modulePath.c_str()));
            periodicGate->par("initiallyOpen").setBoolValue(initiallyOpens[i]);
            periodicGate->par("offset").setDoubleValue(offsets[i].dbl());
            cPar& durationsParameter = periodicGate->par("durations");
            durationsParameter.copyIfShared();
            durationsParameter.setObjectValue(gateDurations[i]);
        }
    }
}

void GateControlList::parseGcl()
{
    initiallyOpens = parseGclLine(gateStates->get(0).stringValue());
    for (size_t i = 0; i < numGates; ++i) {
        offsets.push_back(0);
        gateDurations.push_back(new cValueArray());
    }
    std::vector<bool> currentGateStates = initiallyOpens;
    std::vector<double> currentDuration(numGates, 0);
    for (int i = 0; i < gateStates->size(); ++i) {
        double duration = durations->get(i).doubleValueInUnit("s");
        std::vector<bool> entry = parseGclLine(gateStates->get(i).stringValue());
        for (size_t j = 0; j < numGates; ++j) {
            if (entry.at(j) != currentGateStates.at(j)) {
                gateDurations.at(j)->add(cValue(currentDuration[j], "s"));
                currentGateStates[j] = !currentGateStates[j];
                currentDuration[j] = 0;
            }
            currentDuration[j] += duration;
        }
    }
    for (size_t i = 0; i < numGates; ++i) {
        auto durations = gateDurations.at(i);
        if (durations->size() % 2 != 0)
            durations->add(cValue(currentDuration[i], "s"));
        else if (durations->size() != 0) {
            durations->set(0, cValue(durations->get(0).doubleValueInUnit("s") + currentDuration[i], "s"));
            offsets[i] = currentDuration[i];
        }
    }
}

std::vector<bool> GateControlList::parseGclLine(const char *gateStates)
{
    std::vector<bool> result(numGates);
    if (strlen(gateStates) != numGates)
        throw cRuntimeError("The length of the entry is not equal to numGates");
    for (size_t i = 0; i < numGates; ++i) {
        char ch = *(gateStates + i);
        int index = mapping[i] - '0';
        if (index < 0 || (size_t)index >= numGates)
            throw cRuntimeError("Invalid gate index %d in mapping", index);
        if (ch == '1')
            result[index] = true;
        else if (ch == '0')
            result[index] = false;
        else
            throw cRuntimeError("Unknown char");
    }
    return result;
}

} // namespace queueing
} // namespace inet
