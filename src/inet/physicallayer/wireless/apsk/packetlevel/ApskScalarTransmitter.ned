//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.apsk.packetlevel;

import inet.physicallayer.wireless.common.base.packetlevel.NarrowbandTransmitterBase;

//
// This transmitter model produces transmissions that have scalar transmission
// power in their analog representation and the configured modulation.
//
// @see ~ApskScalarReceiver, ~ApskScalarRadio, ~ApskScalarRadioMedium.
//
module ApskScalarTransmitter extends NarrowbandTransmitterBase
{
    parameters:
        modulation = default("BPSK");
        @class(ApskScalarTransmitter);
}

