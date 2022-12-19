//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.ethernet.modular;

import inet.linklayer.ethernet.LayeredEthernetInterface;
import inet.linklayer.ethernet.contract.IEthernetMacLayer;
import inet.physicallayer.wired.ethernet.IEthernetPhyLayer;

//
// This module represents an Ethernet network interface with cut-through support.
//
module EthernetCutthroughInterface extends LayeredEthernetInterface
{
    parameters:
        string macTableModule;
        *.bitrate = default(this.bitrate);
        *.macTableModule = default(this.macTableModule);
        phyLayer.typename = default("EthernetStreamThroughPhyLayer");
        phyLayer.cutthroughSource.typename = default("EthernetCutthroughSource");
        phyLayer.cutthroughSink.typename = default("EthernetCutthroughSink");
}

