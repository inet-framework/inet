//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


//TODO review
package inet.linklayer.ieee80211.mac.recipient;

//
// Implements the MAC data service for the recipient side in IEEE 802.11 networks.
// Processes received frames by performing defragmentation and duplicate detection.
// Handles different frame types (data, management, control) and maintains statistics
// for defragmented and deaggregated packets, as well as dropped duplicate packets.
//
module RecipientMacDataService
{
    parameters:
        @class("RecipientMacDataService");
        @display("i=block/join");
        @signal[packetDefragmented](type=inet::Packet);
        @signal[packetDeaggregated](type=inet::Packet);
        @signal[packetDropped](type=inet::Packet);
        @statistic[packetDefragmented](title="packets defragmented"; record=count);
        @statistic[packetDeaggregated](title="packets deaggregated"; record=count);
        @statistic[packetDropDuplicateDetected](title="packet drops: duplicate detected"; source=packetDropReasonIsDuplicateDetected(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
}
