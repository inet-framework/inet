//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.contract.packetlevel;

//
// The background noise model describes the thermal noise, the cosmic background
// noise, and other random fluctuations of the electromagnetic field that affect
// the quality of the communication channel.
//
moduleinterface IBackgroundNoise
{
    parameters:
        @display("i=block/mac");
}

