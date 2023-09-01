//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wired.ethernet;

import inet.protocolelement.contract.IProtocolLayer;

simple NewEthernetPlca like IProtocolLayer
{
    parameters:
        bool plca_en @mutable = default(true);
        bool plca_reset @mutable = default(false);
        int plca_node_count;
        int local_nodeID;
        int max_bc = default(10);
        int delay_line_length = default(100);
        double to_interval @unit(s) = default(3.2us);
        double burst_interval @unit(s) = default(9.6us);
        @signal[curID](type=int);
        @signal[controlStateChanged](type=long);
        @signal[dataStateChanged](type=long);
        @signal[statusStateChanged](type=long);
        @signal[rxCmd](type=long);
        @signal[txCmd](type=long);
        @statistic[curID](title="current transmit opportunity ID"; record=vector; interpolationmode=sample-hold);
        @statistic[controlState](title="control state"; type=enum; enum=DISABLE, RESYNC, RECOVER, SEND_BEACON, SYNCING, WAIT_TO, EARLY_RECEIVE, COMMIT, YIELD, RECEIVE, TRANSMIT, BURST, ABORT, NEXT_TX_OPPORTUNITY; source=controlStateChanged; record=count,vector; interpolationmode=sample-hold);
        @statistic[dataState](title="data state"; type=enum; enum=NORMAL, WAIT_IDLE, IDLE, RECEIVE, HOLD, ABORT, COLLIDE, DELAY_PENDING, PENDING, WAIT_MAC, TRANSMIT, FLUSH; source=dataStateChanged; record=count,vector; interpolationmode=sample-hold);
        @statistic[statusState](title="status state"; type=enum; enum=INACTIVE, ACTIVE, HYSTERESIS; source=statusStateChanged; record=count,vector; interpolationmode=sample-hold);
        @statistic[rxCmd](title="RX command"; type=enum; enum=NONE, BEACON, COMMIT; record=count,vector; interpolationmode=sample-hold);
        @statistic[txCmd](title="TX command"; type=enum; enum=NONE, BEACON, COMMIT; record=count,vector; interpolationmode=sample-hold);
        @display("i=block/rxtx");
    gates:
        input upperLayerIn @labels(EtherFrame);
        output upperLayerOut @labels(EtherFrame);
        input lowerLayerIn @labels(EtherFrame);
        output lowerLayerOut @labels(EtherFrame);
}

