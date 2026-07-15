//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.queue;

import inet.queueing.base.PacketClassifierBase;
import inet.queueing.contract.IPacketClassifier;

//
// The classifying half of the ~AirtimeFairnessQueue compound queue. It routes each frame
// to a per-receiver sub-queue: the first frame seen for a given receiver (destination
// station) MAC address is assigned the next free output index, and all later frames for
// that receiver follow the same branch. This gives one FIFO sub-queue (and one
// ~AirtimeFairnessGate) per station, as needed by the airtime deficit round robin.
//
// @see ~AirtimeFairnessQueue, ~AirtimeFairnessGate, ~AirtimeFairnessScheduler
//
simple AirtimeFairnessClassifier extends PacketClassifierBase like IPacketClassifier
{
    parameters:
        @class(AirtimeFairnessClassifier);
        @display("i=block/classifier");
}
