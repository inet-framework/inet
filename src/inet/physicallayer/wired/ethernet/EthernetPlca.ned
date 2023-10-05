//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wired.ethernet;

import inet.protocolelement.contract.IProtocolLayer;

//
// This module implements the Ethernet Phyisical Layer Collision Avoidance (PLCA)
// protocol defined in the IEEE 802.3cg-2019 standard. It can be used to build an
// Ethernet network interface that can be connected to 10BASE-T1S multidrop links.
//
// The PLCA protocol is implemented with the following features:
//  - control state machine (with minor modifications)
//  - data state machine (with some modifications)
//  - periodic beacons to synchronize cycle start
//  - keeping track of the current transmit opportunity
//  - various timing parameters (beacon length, to length, etc.)
//  - packet bursts with configurable maximum number of packets
//  - configurable delay line to temporarily hold the frame from the MAC
//  - carrier sense and collision signals
//  - various end signal delimiters
//
// The PLCA protocol is implemented with several limitations:
//  - status state machine is not implemented (plca_status)
//  - PLCA mode cannot be enabled/disabled dynamically (plca_en), it's always active
//  - PLCA cannot be reset (plca_reset)
//  - special handling of local_nodeID 255 is missing
//  - beacons are assumed to be never lost (no invalid_beacon_timer)
//  - data state machine cannot abort transmissions
//  - mixed operation with CSMA/CD nodes on the same multidrop link is not supported
//  - controller immediately start sending a beacon
//  - nodes start with waiting to receive a beacon
//
// All ~EthernetPlca modules in the network interfaces on the same multidrop link
// must have the same plca_node_count parameter value set. Also, the local_nodeID
// parameter must be set to a uniqe value in the [0, plca_node_count - 1] range.
//
// This module requires that the modules connected to it implement certain C++
// interfaces. The upper layer must be connected to a module implementing the
// ~IEthernetCsmaMac C++ interface. One such module is the ~EthernetCsmaMac module.
// The lower layer must be connected to a module implementing the ~IEthernetCsmaPhy
// C++ interface. One such module is the ~EthernetCsmaPhy module.
//
// @see ~EthernetCsmaMac, ~EthernetCsmaPhy, ~EthernetPlcaInterface
//
simple EthernetPlca like IProtocolLayer
{
    parameters:
        // Maximum number of PLCA nodes on the mixing segment receiving transmit
        // opportunities before the node with local_nodeID = 0 generates a new
        // BEACON.
        int plca_node_count;
        // ID representing the PLCA transmit opportunity number assigned to the
        // node.
        int local_nodeID;
        // Maximum number of additional packets the node is allowed to transmit
        // in a single burst.
        int max_bc = default(0);
        // The maximum number of nibbles that the PLCA RS variable delay line
        // can hold.
        int delay_line_length = default(100);
        // The transmit opportunity timer should be set equal across the mixing
        // segment for PLCA to work properly.
        int to_timer_length @unit(b) = default(32b);
        // This timer determines how long to wait for the MAC to send a new packet
        // before yielding the transmit opportunity. For PLCA burst mode to work
        // properly this timer should be set greater than one IPG.
        int burst_timer_length @unit(b) = default(128b);
        // Times the duration of the BEACON signal.
        int beacon_timer_length @unit(b) = default(20b);
        // Timer for detecting received BEACONs.
        int beacon_det_timer_length @unit(b) = default(22b);
        // Defines the time the PLCA Data state diagram waits in the DELAY_PENDING
        // state before switching to PENDING state.
        int pending_timer_length @unit(b) = default(512b);
        // Defines the maximum time the PLCA Data state machine is allowed to stay
        // in WAIT_MAC state.
        int commit_timer_length @unit(b) = default(288b);
        // emitted when the current transmit opportunity ID changes, the value
        // is the node ID of the owner of the current transmit opportunity
        @signal[curID](type=int);
        // emitted when the incoming carrier signal changes, the value is 1 or 0
        @signal[carrierSenseChanged](type=int);
        // emitted when the incoming collision signal changes, the value is 1 or 0
        @signal[collisionChanged](type=int);
        // emitted when the state of the control state machine changes, the value is one of DISABLE, RESYNC, RECOVER, SEND_BEACON, SYNCING, WAIT_TO, EARLY_RECEIVE, COMMIT, YIELD, RECEIVE, TRANSMIT, BURST, ABORT, NEXT_TX_OPPORTUNITY
        @signal[controlStateChanged](type=long);
        // emitted when the state of the data state machine changes, the value is one of WAIT_IDLE, IDLE, RECEIVE, HOLD, COLLIDE, DELAY_PENDING, PENDING, WAIT_MAC, TRANSMIT 
        @signal[dataStateChanged](type=long);
        // emitted when the value of the rx_cmd variable changes, the value is one of NONE, BEACON, COMMIT
        @signal[rxCmd](type=long);
        // emitted when the value of the tx_cmd variable changes, the value is one of NONE, BEACON, COMMIT
        @signal[txCmd](type=long);
        // emitted when a packet is sent to the PHY, the value is the delay from when the packet was received from the MAC
        @signal[packetPendingDelay](type=simtime_t);
        // emitted when a packet is sent to the PHY, the value is the time elapsed since the last packet was sent
        @signal[packetInterval](type=simtime_t);
        // emitted when a transmit opportunity starts, the value is 1 or 0
        @signal[transmitOpportunityUsed](type=int);
        // emitted when a transmit opportunity ends for any node, the value is the number of packets sent or received during the transmit opportunity
        @signal[numPacketsPerTo](type=int);
        // emitted when a transmit opportunity ends for this node, the value is the number of packets sent during the transmit opportunity
        @signal[numPacketsPerOwnTo](type=int);
        // emitted when a cycle ends, the value is the number of packets sent or received during the cycle
        @signal[numPacketsPerCycle](type=int);
        // emitted when a transmit opportunity ends for any node, the value is the duration of the transmit opportunity
        @signal[toLength](type=simtime_t);
        // emitted when a transmit opportunity ends for this node, the value is the duration of the transmit opportunity
        @signal[ownToLength](type=simtime_t);
        // emitted when a cycle ends, the value is the duration of the cycle
        @signal[cycleLength](type=simtime_t);
        @signal[packetSentToLower](type=inet::Packet);
        @signal[packetReceivedFromLower](type=inet::Packet);
        // the time evolution of the current transmit opportunity ID
        @statistic[curID](title="current transmit opportunity ID"; record=vector; interpolationmode=sample-hold);
        // the time evolution of the incoming carrier sense signal
        @statistic[carrierSense](title="carrier sense"; type=int; source=carrierSenseChanged; record=count,vector; interpolationmode=sample-hold);
        // the time evolution of the incoming collision signal
        @statistic[collision](title="collision"; type=int; source=collisionChanged; record=count,vector; interpolationmode=sample-hold);
        // the time evolution of the state of the control state machine
        @statistic[controlState](title="control state"; type=enum; enum=DISABLE, RESYNC, RECOVER, SEND_BEACON, SYNCING, WAIT_TO, EARLY_RECEIVE, COMMIT, YIELD, RECEIVE, TRANSMIT, BURST, ABORT, NEXT_TX_OPPORTUNITY; source=controlStateChanged; record=count,vector; interpolationmode=sample-hold);
        // the time evolution of the state of the data state machine
        @statistic[dataState](title="data state"; type=enum; enum=WAIT_IDLE, IDLE, RECEIVE, HOLD, COLLIDE, DELAY_PENDING, PENDING, WAIT_MAC, TRANSMIT; source=dataStateChanged; record=count,vector; interpolationmode=sample-hold);
        @statistic[rxCmd](title="RX command"; type=enum; enum=NONE, BEACON, COMMIT; record=count,vector; interpolationmode=sample-hold);
        @statistic[txCmd](title="TX command"; type=enum; enum=NONE, BEACON, COMMIT; record=count,vector; interpolationmode=sample-hold);
        @statistic[packetPendingDelay](title="packet pending delay"; unit=s; record=count,histogram,vector; interpolationmode=none);
        @statistic[packetInterval](title="packet interval"; unit=s; record=count,histogram,vector; interpolationmode=none);
        @statistic[transmitOpportunityUsed](title="transmit opportunity used"; record=count,vector; interpolationmode=none);
        // the time evolution of the number of packets sent or received per transmit opportunity for any node
        @statistic[numPacketsPerTo](title="number of packets per transmit opportunity"; record=vector; interpolationmode=none);
        // the time evolution of the number of packets sent per transmit opportunity for this node
        @statistic[numPacketsPerOwnTo](title="number of packets per transmit opportunity"; record=vector; interpolationmode=none);
        // the time evolution of the number of packets sent or received per cycle
        @statistic[numPacketsPerCycle](title="number of packets per cycle"; record=vector; interpolationmode=none);
        // the time evolution of the transmit opportunity length for any node
        @statistic[toLength](title="transmit opportunity length"; record=vector; interpolationmode=none);
        // the time evolution of the transmit opportunity length for any node
        @statistic[ownToLength](title="own transmit opportunity length"; record=vector; interpolationmode=none);
        // the time evolution of the cycle length
        @statistic[cycleLength](title="cycle length"; record=vector; interpolationmode=none);
        @display("i=block/rxtx");
    gates:
        input upperLayerIn @labels(EtherFrame);
        output upperLayerOut @labels(EtherFrame);
        input lowerLayerIn @labels(EtherFrame);
        output lowerLayerOut @labels(EtherFrame);
}

