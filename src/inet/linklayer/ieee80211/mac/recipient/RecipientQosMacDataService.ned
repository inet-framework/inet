//
// Copyright (C) 2016 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ieee80211.mac.recipient;

//
// Implements the IEEE 802.11e/n/ac QoS MAC data service for recipient stations, extending 
// the basic MAC data service with Quality of Service capabilities. This module processes 
// incoming frames by handling both defragmentation and deaggregation operations. It can 
// process A-MSDU (Aggregated MAC Service Data Unit) frames by extracting the individual 
// MSDUs that were combined by the originator, and can handle A-MPDU (Aggregated MAC 
// Protocol Data Unit) frames by processing multiple MAC frames that were transmitted 
// in a single PHY transmission. The module also performs duplicate detection to identify 
// and discard frames that have been received multiple times due to retransmissions. 
// Statistics are maintained on defragmented and deaggregated packets, as well as packets 
// dropped due to duplicate detection.
//
// @see ~RecipientMacDataService, ~OriginatorQosMacDataService
//
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
