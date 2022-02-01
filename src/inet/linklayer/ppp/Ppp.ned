//
// Copyright (C) 2004 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ppp;

//
// PPP implementation.
//
// Packets are encapsulated in ~PppFrame.
//
// PPP is a complex protocol with strong support for link configuration
// and maintenance. This model ignores those details, and only performs
// simple encapsulation/decapsulation and queuing.
//
// In routers, PPP relies on an external queue module (see ~IPacketQueue)
// to model finite buffer, implement QoS and/or RED, and requests packets
// from this external queue one-by-one.
//
// In hosts, no such queue is used, so PPP contains an internal
// queue to store packets waiting for transmission.
// Conceptually, the queue is of infinite size, but for better diagnostics
// one can specify a hard limit in the packetCapacity parameter -- if this is
// exceeded, the simulation stops with an error.
//
// There is no buffering done on received packets -- they are just decapsulated
// and sent up immediately.
//
// @see ~PppInterface
//
simple Ppp
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        string displayStringTextFormat = default("rate: %b\nsent: %s, rcvd: %r\nqueue: %q, drop: %d");
        bool sendRawBytes = default(false); // when true packets are serialized into a sequence of bytes before sending out
        int mtu @unit(B) = default(4470B);
        @lifecycleSupport;
        double stopOperationExtraTime @unit(s) = default(-1s);    // extra time after lifecycle stop operation finished
        double stopOperationTimeout @unit(s) = default(2s);    // timeout value for lifecycle stop operation
        @class(Ppp);
        @display("i=block/rxtx");

        @signal[transmissionStateChanged](type=long);    // 1:transmit, 0:idle
        @signal[rxPkOk](type=inet::Packet);
        @signal[packetDropped](type=inet::Packet);
        @signal[packetSentToLower](type=inet::Packet);
        @signal[packetReceivedFromLower](type=cPacket);
        @signal[packetSentToUpper](type=cPacket);
        @signal[packetReceivedFromUpper](type=cPacket);
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

