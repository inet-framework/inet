//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ieee80211/mac/queue/AirtimeFairnessCompoundQueue.h"

#include "inet/common/InitStages.h"

namespace inet {
namespace ieee80211 {

Define_Module(AirtimeFairnessCompoundQueue);

void AirtimeFairnessCompoundQueue::initialize(int stage)
{
    CompoundPacketQueueBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        // Number of per-station branches created so far (= stations served). Watched on this
        // module so the display string can use {numStations}; a cross-submodule reference like
        // {classifier.numStations} would make ModuleMixin call getModuleByPath(), which throws
        // in OMNeT++ 6 and breaks every Qtenv refreshDisplay (Cmdenv never evaluates it).
        WATCH_EXPR("numStations", (int)getSubmoduleVectorSize("branch"));
    }
}

} // namespace ieee80211
} // namespace inet
