//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wired.ethernet;

import inet.protocolelement.contract.IProtocolLayer;

simple SimplifiedEthernetPlca like IProtocolLayer
{
    parameters:
        int plca_node_count;
        int local_nodeID;
        int max_bc = default(1);
        double to_interval @unit(s) = default(3.2us); // 32 bits
        double burst_interval @unit(s) = default(10us); // 100 bits
        double beacon_duration @unit(s) = default(2us); // 20 bits
        @signal[curID](type=int);
        @signal[stateChanged](type=int);
        @statistic[curID](title="current transmit opportunity ID"; record=vector; interpolationmode=sample-hold);
        @statistic[state](title="state"; type=enum; enum=DISABLED, SENDING_BEACON, SENDING_COMMIT, SENDING_DATA, WAITING_BEACON, WAITING_COMMIT, WAITING_DATA, WAITING_CRS_OFF, WAITING_TO, RECEIVING_BEACON, RECEIVING_COMMIT, RECEIVING_DATA, END_TO; source=stateChanged; record=count,vector; interpolationmode=sample-hold);
        @display("i=block/rxtx");
    gates:
        input upperLayerIn @labels(EtherFrame);
        output upperLayerOut @labels(EtherFrame);
        input lowerLayerIn @labels(EtherFrame);
        output lowerLayerOut @labels(EtherFrame);
}

