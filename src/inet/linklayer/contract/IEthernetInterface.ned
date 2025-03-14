//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.contract;

//
// Module interface for Ethernet network interfaces. Ethernet interfaces are
// wired interfaces that support various data rates including 10Mbps (classic Ethernet),
// 100Mbps (Fast Ethernet), 1Gbps (Gigabit Ethernet), 10Gbps, 40Gbps, and 100Gbps.
// Ethernet interfaces connect to other nodes via wired connections and can operate
// in full-duplex mode, allowing simultaneous transmission and reception of data.
//
// @see EthernetInterface, LayeredEthernetInterface, EthernetPlcaInterface, EthernetCutthroughInterface, EthernetHost, EthernetSwitch
//
moduleinterface IEthernetInterface extends IWiredInterface
{
    parameters:
        double bitrate @unit(bps);
        @display("i=block/ifcard");
}
