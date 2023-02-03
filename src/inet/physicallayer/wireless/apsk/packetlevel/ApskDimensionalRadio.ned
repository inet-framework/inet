//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.apsk.packetlevel;

//
// This radio model provides a hypothetical radio that simply uses one of the
// well-known modulations without utilizing other techiques such as forward
// error correction, interleaving, spreading, etc. It must be used in
// conjunction with the ~ApskDimensionalRadioMedium model.
//
// @see ~ApskDimensionalRadioMedium, ~DimensionalAnalogModel,
// ~ApskTransmitter, ~ApskReceiver.
//
module ApskDimensionalRadio extends ApskRadio
{
    parameters:
        transmitter.typename = default("ApskTransmitter");
        transmitter.analogModel.typename = default("DimensionalTransmitterAnalogModel");
        receiver.typename = default("ApskReceiver");
        receiver.analogModel.typename = default("DimensionalReceiverAnalogModel");
}

