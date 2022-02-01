//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.contract.packetlevel;

//
// The error model describes how the signal to noise ratio affects the amount of
// errors at the receiver.
//
moduleinterface IErrorModel
{
    parameters:
        @display("i=block/broadcast");
}

