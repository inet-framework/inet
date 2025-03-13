//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ppp;

//
// Implements the Point-to-Point Protocol (PPP) for serial data transmission.
// Encapsulates packets in ~PppHeader and ~PppTrailer for transmission over
// point-to-point links.
//
// Focuses on the data encapsulation/decapsulation and queuing aspects of PPP,
// while omitting the complex link configuration and maintenance features of
// the full protocol.
//
// In router configurations, works with an external queue module (see ~IPacketQueue)
// to model finite buffers, implement QoS, and support RED. Requests packets
// from this external queue one-by-one as the physical link becomes available.
//
// In host configurations, uses an internal queue for packets awaiting transmission.
// The queue is conceptually infinite, but a hard limit can be specified via
// the `packetCapacity` parameter for diagnostic purposes.
//
// Received packets are immediately decapsulated and forwarded to the upper layer
// without additional buffering.
//
// @see ~PppInterface, ~IPppInterface, ~PppFrame
//
simple Ppp
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        string displayStringTextFormat = default("rate: %b\nsent: %s, rcvd: %r\nqueue: %q, drop: %d");
        bool sendRawBytes = default(false); // When true packets are serialized into a sequence of bytes before sending out
        int mtu @unit(B) = default(4470B);
        @lifecycleSupport;
        double stopOperationExtraTime @unit(s) = default(-1s);    // Extra time after lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s);    // Timeout value for lifecycle stop operation
        @class(Ppp);
        @display("i=block/rxtx");

        @signal[transmissionStateChanged](type=long);    // 1:transmit, 0:idle
        @signal[rxPkOk](type=inet::Packet);
        @signal[packetDropped](type=inet::Packet);
        @signal[packetSentToLower](type=inet::Packet);
        @signal[packetReceivedFromLower](type=cPacket);
        @signal[packetSentToUpper](type=cPacket);
        @signal[packetReceivedFromUpper](type=cPacket);
        @signal[transmissionEnded](type=inet::Packet);
        @signal[receptionEnded](type=inet::Packet);
        @statistic[transmissionState](title="tx state"; source=transmissionStateChanged; record=timeavg,vector; interpolationmode=sample-hold);
        @statistic[txPk](title="packets transmitted"; source=packetSentToLower; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[rxPkOk](title="packets received OK"; source=rxPkOk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[packetDropIncorrectlyReceived](title="packet drop: incorrectly received"; source=packetDropReasonIsIncorrectlyReceived(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[packetDropInterfaceDown](title="packet drop: interface down"; source=packetDropReasonIsInterfaceDown(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
        @statistic[rcvdPkFromHl](title="packets received from higher layer"; source=packetReceivedFromUpper; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[passedUpPk](title="packets passed to higher layer"; source=packetSentToUpper; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
    gates:
        input upperLayerIn;
        output upperLayerOut;
        inout phys @labels(PppFrame);
}
