//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.basic;

import inet.linklayer.base.MacProtocolBase;

//
// Implements the Ethernet CSMA/CD (Carrier Sense Multiple Access with Collision
// Detection) MAC protocol as defined in the IEEE 802.3 standard. This module
// provides the core medium access control functionality for both traditional
// Ethernet CSMA/CD and modern Ethernet PLCA network interfaces.
//
// Key features:
// - Carrier sensing: Monitors the channel for ongoing transmissions
// - Collision detection: Identifies when multiple stations transmit simultaneously
// - Exponential backoff: Implements the binary exponential backoff algorithm
//   to resolve collisions
// - Jam signaling: Sends jam signals when collisions are detected to ensure
//   all stations recognize the collision
// - Inter-frame gap (IFG) management: Maintains proper spacing between frames
//
// The module operates in a state machine with the following states:
// - IDLE: No transmission or reception in progress
// - WAIT_IFG: Waiting for the inter-frame gap to elapse before transmission
// - TRANSMITTING: Sending a frame onto the medium
// - JAMMING: Sending a jam signal after detecting a collision
// - BACKOFF: Waiting a random time after a collision before retransmission
// - RECEIVING: Processing an incoming frame
//
// This MAC implementation is designed to work with physical layer modules that
// implement the ~IEthernetCsmaPhy interface. It can be paired with either:
// - ~EthernetCsmaPhy: For traditional CSMA/CD operation
// - ~EthernetPlca: For Physical Layer Collision Avoidance in 10BASE-T1S networks
//
// @see ~EthernetCsmaPhy, ~EthernetPlca, ~EthernetPlcaInterface, ~EthernetInterface
//
simple EthernetCsmaMac extends MacProtocolBase
{
    parameters:
        bool promiscuous = default(false);  // if true, all packets are received, otherwise only the
                                            // ones with matching destination MAC address
        int mtu @unit(B) = default(1500B);
        string fcsMode @enum("declared","computed");
        @class(EthernetCsmaMac);
        // emitted when the incoming carrier sense signal changes, the value is 1 or 0
        @signal[carrierSenseChanged](type=int);
        // emitted when the incoming collision signal changes, the value is 1 or 0
        @signal[collisionChanged](type=int);
        // emitted when the state of the state machine changes, the value is one of IDLE, WAIT_IFG, TRANSMITTING, JAMMING, BACKOFF, RECEIVING
        @signal[stateChanged](type=int);
        // the time evolution of the incoming carrier sense signal
        @statistic[carrierSense](title="carrier sense"; type=int; source=carrierSenseChanged; record=count,vector; interpolationmode=sample-hold);
        // the time evolution of the incoming collision signal
        @statistic[collision](title="collision"; type=int; source=collisionChanged; record=count,vector; interpolationmode=sample-hold);
        // the time evolution of the state of the state machine
        @statistic[state](title="state"; type=enum; enum=IDLE, WAIT_IFG, TRANSMITTING, JAMMING, BACKOFF, RECEIVING; source=stateChanged; record=count,vector; interpolationmode=sample-hold);
}
