//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.recipient;

import inet.common.Module;

//
// Implements the MAC data service for the recipient side in non-QoS IEEE 802.11 networks.
// Processes received frames by performing defragmentation and duplicate detection.
//
// @see RecipientQosMacDataService
//
module RecipientMacDataService extends Module
{
    parameters:
        @class(RecipientMacDataService);
        double maxReceiveLifetime @unit(s) = default(524288us); // Nonnegative relative receive lifetime; IEEE Std 802.11-2024 dot11MaxReceiveLifetime default: 512 TUs
        @display("i=block/join");
        @signal[packetDefragmented](type=inet::Packet);
        @signal[packetDeaggregated](type=inet::Packet);
        @signal[packetDropped](type=inet::Packet);
        @statistic[packetDefragmented](title="packets defragmented"; record=count);
        @statistic[packetDeaggregated](title="packets deaggregated"; record=count);
        @statistic[packetDropDuplicateDetected](title="packet drops: duplicate detected"; source=packetDropReasonIsDuplicateDetected(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
}
