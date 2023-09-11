//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wired.ethernet;

import inet.protocolelement.contract.IProtocolLayer;

simple EthernetPlca like IProtocolLayer
{
    parameters:
        // Maximum number of PLCA nodes on the mixing segment receiving transmit opportunities before the node
        // with local_nodeID = 0 generates a new BEACON.
        int plca_node_count;
        // ID representing the PLCA transmit opportunity number assigned to the node.
        int local_nodeID;
        // Maximum number of additional packets the node is allowed to transmit in a single burst.
        int max_bc = default(0);
        // The maximum number of nibbles that the PLCA RS variable delay line can hold.
        int delay_line_length = default(100);
        // The transmit opportunity timer should be set equal across the mixing segment for PLCA to work properly.
        int to_timer_length @unit(b) = default(32b);
        // This timer determines how long to wait for the MAC to send a new packet before yielding the transmit
        // opportunity. For PLCA burst mode to work properly this timer should be set greater than one IPG.
        int burst_timer_length @unit(b) = default(100b);
        // Times the duration of the BEACON signal.
        int beacon_timer_length @unit(b) = default(20b);
        // Timer for detecting received BEACONs.
        int beacon_det_timer_length @unit(b) = default(22b);
        // Defines the time the PLCA Data state diagram waits in the DELAY_PENDING state before switching to PENDING state.
        int pending_timer_length @unit(b) = default(512b);
        // Defines the maximum time the PLCA Data state machine is allowed to stay in WAIT_MAC state.
        int commit_timer_length @unit(b) = default(288b);
        @signal[curID](type=int);
        @signal[carrierSenseChanged](type=int);
        @signal[collisionChanged](type=int);
        @signal[controlStateChanged](type=long);
        @signal[dataStateChanged](type=long);
        @signal[rxCmd](type=long);
        @signal[txCmd](type=long);
        @signal[packetPendingDelay](type=simtime_t);
        @signal[packetInterval](type=simtime_t);
        @signal[transmitOpportunityUsed](type=int);
        @statistic[curID](title="current transmit opportunity ID"; record=vector; interpolationmode=sample-hold);
        @statistic[carrierSense](title="carrier sense"; type=int; source=carrierSenseChanged; record=count,vector; interpolationmode=sample-hold);
        @statistic[collision](title="collision"; type=int; source=collisionChanged; record=count,vector; interpolationmode=sample-hold);
        @statistic[controlState](title="control state"; type=enum; enum=DISABLE, RESYNC, RECOVER, SEND_BEACON, SYNCING, WAIT_TO, EARLY_RECEIVE, COMMIT, YIELD, RECEIVE, TRANSMIT, BURST, ABORT, NEXT_TX_OPPORTUNITY; source=controlStateChanged; record=count,vector; interpolationmode=sample-hold);
        @statistic[dataState](title="data state"; type=enum; enum=WAIT_IDLE, IDLE, RECEIVE, HOLD, COLLIDE, DELAY_PENDING, PENDING, WAIT_MAC, TRANSMIT; source=dataStateChanged; record=count,vector; interpolationmode=sample-hold);
        @statistic[rxCmd](title="RX command"; type=enum; enum=NONE, BEACON, COMMIT; record=count,vector; interpolationmode=sample-hold);
        @statistic[txCmd](title="TX command"; type=enum; enum=NONE, BEACON, COMMIT; record=count,vector; interpolationmode=sample-hold);
        @statistic[packetPendingDelay](title="packet pending delay"; unit=s; record=count,vector; interpolationmode=none);
        @statistic[packetInterval](title="packet interval"; unit=s; record=count,vector; interpolationmode=none);
        @statistic[transmitOpportunityUsed](title="transmit opportunity used"; record=count,vector; interpolationmode=none);
        @display("i=block/rxtx");
    gates:
        input upperLayerIn @labels(EtherFrame);
        output upperLayerOut @labels(EtherFrame);
        input lowerLayerIn @labels(EtherFrame);
        output lowerLayerOut @labels(EtherFrame);
}

