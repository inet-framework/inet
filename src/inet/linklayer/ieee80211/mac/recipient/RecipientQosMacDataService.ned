//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.recipient;

module RecipientQosMacDataService
{
    parameters:
        @class("RecipientQosMacDataService");
        @display("i=block/join");
        @signal[packetDefragmented](type=inet::Packet);
        @signal[packetDeaggregated](type=inet::Packet);
        @signal[packetDropped](type=inet::Packet);
        @statistic[packetDefragmented](title="packets defragmented"; record=count);
        @statistic[packetDeaggregated](title="packets deaggregated"; record=count);
        @statistic[packetDropDuplicateDetected](title="packet drops: duplicate detected"; source=packetDropReasonIsDuplicateDetected(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
}

