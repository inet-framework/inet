//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.contract.packetlevel;

//
// The path loss model  describes the reduction of power as the signal
// propagates through space.
//
moduleinterface IPathLoss
{
    parameters:
        @display("i=block/control");
}

