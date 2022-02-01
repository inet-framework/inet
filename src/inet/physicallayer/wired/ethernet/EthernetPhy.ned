//
// Copyright (C) 2018 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wired.ethernet;

simple EthernetPhy
{
    parameters:
        @display("i=block/rxtx");
    gates:
        input upperLayerIn @labels(EtherFrame);
        output upperLayerOut @labels(EtherFrame);
        inout phys @labels(EthernetSignal);
}

