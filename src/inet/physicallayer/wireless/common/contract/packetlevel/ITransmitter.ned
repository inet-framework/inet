//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.contract.packetlevel;

//
// The transmitter model describes the physical process which converts packets
// into electric signals.
//
moduleinterface ITransmitter
{
    parameters:
        @display("i=block/wtx");
}

