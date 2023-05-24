//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.contract.packetlevel;

//
// The receiver analog model describes the connecting piece between the technology-dependent
// part of the receivers, and the different analog medium models.
//
// @see ~IAnalogModel, ~ITransmitterAnalogModel
//
moduleinterface IReceiverAnalogModel
{
    parameters:
        @display("i=block/tunnel");
}

