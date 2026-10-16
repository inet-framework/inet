//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.basic;

import inet.linklayer.base.MacProtocolBase;

//
// Implements the Ethernet CSMA/CD MAC protocol. It supports building
// both Ethernet CSMA/CD and Ethernet PLCA network interfaces.
//
// Implements the exponential backoff algorithm, sends jam signal
// when a collision is detected, and maintains IFG between signals on the channel.
// It expects complete Ethernet MAC protocol frames containing an Ethernet MAC
// header and an Ethernet FCS.
//
// The lower layer must be connected to a module implementing the ~IEthernetCsmaPhy
// C++ interface. There are two such modules: ~EthernetCsmaPhy and ~EthernetPlca.
// The former can be used to build an Ethernet CSMA/CD network interface, the
// latter can be used to build an Ethernet PLCA network interface.
//
// @see ~EthernetCsmaPhy, ~EthernetPlca, ~EthernetPlcaInterface
//
simple EthernetCsmaMac extends MacProtocolBase
{
    parameters:
        bool promiscuous = default(false);  // if true, all packets are received, otherwise only the
                                            // ones with matching destination MAC address
        int mtu @unit(B) = default(1500B);
        string fcsMode @enum("declared","computed");
        @class(EthernetCsmaMac);
        displayStringTextFormat = default("{fsmState}");
        // emitted when the incoming carrier sense signal changes, the value is 1 or 0
        @signal[carrierSenseChanged](type=int);
        // emitted when the incoming collision signal changes, the value is 1 or 0
        @signal[collisionChanged](type=int);
        // emitted when the state of the state machine changes, the value is one of IDLE, WAIT_IFG, TRANSMITTING, JAMMING, BACKOFF, RECEIVING
        @signal[stateChanged](type=int);
        // emitted when a frame transmission completes successfully; the value is the medium access
        // delay of the frame: the time from the start of the frame's first transmission attempt to
        // the start of the transmission attempt that completed successfully (zero for frames that
        // succeed on their first attempt; not emitted for frames dropped at the retry limit)
        @signal[packetPendingDelay](type=simtime_t);
        // the time evolution of the incoming carrier sense signal
        @statistic[carrierSense](title="carrier sense"; type=int; source=carrierSenseChanged; record=count,vector; interpolationmode=sample-hold);
        // the time evolution of the incoming collision signal
        @statistic[collision](title="collision"; type=int; source=collisionChanged; record=count,vector; interpolationmode=sample-hold);
        // the time evolution of the state of the state machine
        @statistic[state](title="state"; type=enum; enum=IDLE, WAIT_IFG, TRANSMITTING, JAMMING, BACKOFF, RECEIVING; source=stateChanged; record=count,vector; interpolationmode=sample-hold);
        // the medium access delay of successfully transmitted frames: the time from the start of the
        // frame's first transmission attempt to the start of the transmission attempt that completed
        // successfully; comparable to the packetPendingDelay statistic of ~EthernetPlca
        @statistic[packetPendingDelay](title="packet pending delay"; unit=s; record=count,histogram,vector; interpolationmode=none);
        // the frames dropped because they suffered a collision on each of the allowed 16 transmission attempts
        @statistic[packetDropRetryLimitReached](title="packet drop: retry limit reached"; source=packetDropReasonIsRetryLimitReached(packetDropped); record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
}

