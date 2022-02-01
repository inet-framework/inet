//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.contract.packetlevel;

//
// The receiver model describes the physical process which converts electric
// signals into packets.
//
moduleinterface IReceiver
{
    parameters:
        @display("i=block/wrx");
}

