//
// Copyright (C) 2023 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wired.ethernet;

//
// Implements the Ethernet CSMA/CD PHY protocol. It supports building
// both Ethernet CSMA/CD and Ethernet PLCA network interfaces.
//
// This module detects the carrier signal and collisions on the channel. Frames
// are received correctly if there are no overlapping transmissions or receptions.
// It also supports sending special Ethernet signals: jam, commit, and beacon.
//
// The upper layer must be connected to a module implementing the ~IEthernetCsmaMac
// C++ interface. There are two such modules: ~EthernetCsmaMac and ~EthernetPlca.
// The former can be used to build an Ethernet CSMA/CD network interface, the
// latter can be used to build an Ethernet PLCA network interface.
//
// @see ~EthernetCsmaPhy, ~EthernetPlca, ~EthernetPlcaInterface
//
simple EthernetCsmaPhy
{
    parameters:
        // emitted when the state of the state machine changes, the value is one of IDLE, TRANSMITTING, RECEIVING, COLLISION, CRS_ON
        @signal[stateChanged](type=int);
        // emitted when the type of the received signal changes, the value is one of NONE, BEACON, COMMIT, DATA, JAM
        @signal[receivedSignalType](type=int);
        // emitted when the type of the transmitted signal changes, the value is one of NONE, BEACON, COMMIT, DATA, JAM
        @signal[transmittedSignalType](type=int);
        // emitted at the reception start, the value is the Ethernet signal being received
        @signal[receptionStarted](type=inet::physicallayer::EthernetSignalBase);
        // emitted at the reception end, the value is the Ethernet signal being received
        @signal[receptionEnded](type=inet::physicallayer::EthernetSignalBase);
        // emitted at the transmission start, the value is the Ethernet signal being transmitted
        @signal[transmissionStarted](type=inet::physicallayer::EthernetSignalBase);
        // emitted at the transmission end, the value is the Ethernet signal being transmitted
        @signal[transmissionEnded](type=inet::physicallayer::EthernetSignalBase);
        // emitted when the bus usage changes, the value is 1 if DATA transmission or reception is in progress, 0 otherwise
        @signal[busUsed](type=int);
        // the time evolution of the state of the state machine
        @statistic[state](title="state"; type=enum; enum=IDLE, TRANSMITTING, RECEIVING, COLLISION, CRS_ON; source=stateChanged; record=count,vector; interpolationmode=sample-hold);
        @statistic[receivedSignalType](title="received signal type"; type=enum; enum=NONE, BEACON, COMMIT, DATA, JAM; record=count,vector; interpolationmode=sample-hold);
        @statistic[transmittedSignalType](title="transmitted signal type"; type=enum; enum=NONE, BEACON, COMMIT, DATA, JAM; record=count,vector; interpolationmode=sample-hold);
        @statistic[transmitting](title="transmitting state"; type=int; source=count(transmissionStarted) - count(transmissionEnded); record=vector; interpolationmode=sample-hold);
        @statistic[throughput](title="throughput"; unit=bps; source=throughput(transmissionEnded); record=vector,histogram; interpolationmode=linear);
        @statistic[propagationTime](title="propagation time"; unit=s);
        @statistic[transmissionTime](title="transmission time"; unit=s; source=packetDuration(transmissionEnded); record=vector,histogram; interpolationmode=none);
        // the time evolution of the bus used signal
        @statistic[busUsed](title="bus used"; record=count,vector; interpolationmode=sample-hold);
        // periodically calculated average bus usage, the value 1 means there was a DATA signal on the bus during the whole period, 0 means there was no DATA signal
        @statistic[busUtilization](title="bus utilization"; source=utilization(busUsed); record=count,vector; interpolationmode=linear);
        @display("i=block/rxtx");
    gates:
        input upperLayerIn @labels(EtherFrame);
        output upperLayerOut @labels(EtherFrame);
        inout phys @labels(EthernetSignal);
}

