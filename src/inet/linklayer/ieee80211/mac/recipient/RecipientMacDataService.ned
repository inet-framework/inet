//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.recipient;

//
// Implements the IEEE 802.11 MAC data service for recipient stations, responsible for 
// processing incoming MAC frames. This module handles defragmentation of received frame 
// fragments, reconstructing the original frame that was divided by the originator for 
// transmission over error-prone channels. It also performs duplicate detection to 
// identify and discard frames that have been received multiple times due to retransmissions. 
// The module maintains statistics on defragmented and deaggregated packets, as well as 
// packets dropped due to duplicate detection, providing insights into the efficiency 
// of the MAC layer operations.
//
// @see ~OriginatorMacDataService, ~RecipientQosMacDataService
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
