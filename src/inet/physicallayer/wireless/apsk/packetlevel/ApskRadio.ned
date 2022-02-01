//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.apsk.packetlevel;

import inet.physicallayer.wireless.common.base.packetlevel.FlatRadioBase;

//
// This radio model provides a hypothetical radio that simply uses one of the
// well-known modulations without utilizing other techiques such as forward
// error correction, interleaving, spreading, etc.
//
// @see ~ApskScalarRadio, ~ApskScalarRadioMedium.
//
module ApskRadio extends FlatRadioBase
{
    parameters:
        string protocol = default("");
        string signalAnalogRepresentation @enum("scalar", "dimensional") = default("scalar");
        transmitter.typename = default(signalAnalogRepresentation == "scalar" ? "ApskScalarTransmitter" : signalAnalogRepresentation == "dimensional" ? "ApskDimensionalTransmitter" : "");
        receiver.typename = default(signalAnalogRepresentation == "scalar" ? "ApskScalarReceiver" : signalAnalogRepresentation == "dimensional" ? "ApskDimensionalReceiver" : "");
        @class(ApskRadio);
        @signal[packetDropped](type=inet::Packet);
        @statistic[packetDropped](title="packets dropped"; record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
}

