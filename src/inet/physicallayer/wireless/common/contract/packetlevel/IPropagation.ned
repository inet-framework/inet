//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.contract.packetlevel;

//
// The propagation model describes how a radio signal propagates through space
// over time.
//
moduleinterface IPropagation
{
    parameters:
        @display("i=block/timer");
}

