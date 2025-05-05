//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.recipient;

import inet.common.Module;

//
// Implements the QoS MAC data service for the recipient side in IEEE 802.11 networks.
// Extends the basic recipient MAC data service with QoS capabilities including A-MPDU
// deaggregation, A-MSDU deaggregation, and block ACK reordering. Processes received
// frames according to the MAC data plane architecture described in the IEEE 802.11
// standard.
//
// @see RecipientMacDataService
//
module RecipientQosMacDataService extends Module
{
    parameters:
        @class(RecipientQosMacDataService);
        @display("i=block/join");
        @signal[packetDefragmented](type=inet::Packet);
        @signal[packetDeaggregated](type=inet::Packet);
        @signal[packetDropped](type=inet::Packet);
        @statistic[packetDefragmented](title="packets defragmented"; record=count);
        @statistic[packetDeaggregated](title="packets deaggregated"; record=count);
        @statistic[packetDropDuplicateDetected](title="packet drops: duplicate detected"; source=packetDropReasonIsDuplicateDetected(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
}

