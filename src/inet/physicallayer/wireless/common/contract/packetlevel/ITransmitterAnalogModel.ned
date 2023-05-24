//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.contract.packetlevel;

//
// The transmitter analog model describes the process which produces the signal
// analog domain representation.
//
// @see ~IMediumAnalogModel, ~IReceiverAnalogModel
//
moduleinterface ITransmitterAnalogModel
{
    parameters:
        @display("i=block/tunnel");
}

