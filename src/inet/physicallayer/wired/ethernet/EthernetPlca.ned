//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wired.ethernet;

import inet.protocolelement.contract.IProtocolLayer;

//
// This module implements the Ethernet Phyisical Layer Collision Avoidance (PLCA)
// protocol defined in the IEEE 802.3cg-2019 standard.
//
// The PLCA protocol is implemented with the several limitations:
//  - status state machine is not implemented (plca_status)
//  - PLCA mode cannot be enabled/disabled dynamically (plca_en), it's always active
//  - PLCA cannot be reset (plca_reset)
//  - special handling of local_nodeID 255 is missing
//  - beacons are assumed to be never lost (no invalid_beacon_timer)
//  - data state machine cannot abort transmissions
//  - mixed operation with CSMA/CD nodes on the same multidrop link is not supported
//
// The upper layer must be connected to a module implementing the ~IEthernetCsmaMac
// C++ interface. The lower layer must be connected to a module implementing the
// ~IEthernetCsmaPhy C++ interface.
//
// @see ~EthernetCsmaMac, ~EthernetCsmaPhy, ~EthernetPlcaInterface
//
simple EthernetPlca like IProtocolLayer
{
    parameters:
        // Maximum number of PLCA nodes on the mixing segment receiving transmit opportunities before the node
        // with local_nodeID = 0 generates a new BEACON.
        // In 802.3cg: Valid range is 0 to 255, inclusive. The default value is 8.
        int plca_node_count;

        // ID representing the PLCA transmit opportunity number assigned to the node.
        // In 802.3cg: Value range is 0 to 255, inclusive. The default value is 255.
        int local_nodeID;

        // Maximum number of additional packets the node is allowed to transmit in a single burst.
        // In 802.3cg: Valid range is 0 to 255, inclusive. The default value is 0.
        int max_bc = default(0);

        // The maximum number of nibbles that the PLCA RS variable delay line can hold.
        // In 802.3cg: implementation dependent constant, Value: up to 99
        int delay_line_length = default(100);

        // The transmit opportunity timer should be set equal across the mixing segment for PLCA to work properly.
        // In 802.3cg: Valid range is 1 to 255, inclusive. The default value is 32.
        // The timer value needs to meet Equation (148–1). The to_timer_length should be set equal across the
        // mixing segment for PLCA to work properly.
        //      to_timer > 2 * max(tpropdelay) +
        //                      max(TX_EN sampled to MDI output) +
        //                      max(MDI input to CRS asserted) +
        //                      max(MDI input to CRS deasserted) - min(MDI input to CRS deasserted) +
        //                      max(MII propagation delay)
        // where tpropdelay is the propagation delay between any two nodes on the mixing segment, and
        // the delay specifications are the maxima and minima for the PHY type on the mixing segment
        // (for 10BASE-T1S, see 147.11).
        // Duration: integer number between 1 and 255, expressed in bit times.
        // Tolerance: 100 ppm.
        int to_timer_length @unit(b) = default(32b);

        // This timer determines how long to wait for the MAC to send a new packet before yielding the transmit
        // opportunity. For PLCA burst mode to work properly this timer should be set greater than one IPG.
        // In 802.3cg: Valid range is 0 to 255, inclusive. The default value is 128.
        int burst_timer_length @unit(b) = default(128b);

        // Times the duration of the BEACON signal.
        // In 802.3cg 148.4.4.4: 20 bit +/- 1/2 bit
        int beacon_timer_length @unit(b) = default(20b);

        // Timer for detecting received BEACONs.
        // In 802.3cg 148.4.4.4: 22 bit +/- 1 bit
        int beacon_det_timer_length @unit(b) = default(22b);

        // Defines the time the PLCA Data state diagram waits in the DELAY_PENDING state before switching to PENDING state.
        // In 802.3cg: Duration: 512 bit times. Tolerance: +/- 1⁄2 bit time.
        int pending_timer_length @unit(b) = default(512b);

        // Defines the maximum time the PLCA Data state machine is allowed to stay in WAIT_MAC state.
        // In 802.3cg: Duration: 288 bit times. Tolerance: +/- 1⁄2 bit time.
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

