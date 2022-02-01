//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.contract.packetlevel;

//
// This module interface provides an abstraction for the interface of different
// physical layers.
//
moduleinterface IPhysicalLayer
{
    parameters:
        @display("i=block/fork");
    gates:
        input upperLayerIn @labels(ILinkLayerFrame/down);
        output upperLayerOut @labels(ILinkLayerFrame/up);
}

