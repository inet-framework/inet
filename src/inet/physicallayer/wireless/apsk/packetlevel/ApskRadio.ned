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
// @see ~ApskScalarRadio, ~ScalarRadioMedium.
//
module ApskRadio extends FlatRadioBase
{
    parameters:
        string protocol = default("");
        transmitter.typename = default("ApskTransmitter");
        receiver.typename = default("ApskReceiver");
        @class(ApskRadio);
        @signal[packetDropped](type=inet::Packet);
        @statistic[packetDropped](title="packets dropped"; record=count,sum(packetBytes),vector(packetBytes); interpolationmode=none);
}

