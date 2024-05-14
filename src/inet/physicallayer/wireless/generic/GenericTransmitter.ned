//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.generic;

import inet.physicallayer.wireless.common.base.packetlevel.TransmitterBase;
import inet.physicallayer.wireless.common.contract.packetlevel.ITransmitter;

//
// This transmitter is part of the generic radio architecture.
//
// @see ~GenericReceiver, ~GenericRadio
//
module GenericTransmitter extends TransmitterBase like ITransmitter
{
    parameters:
        double preambleDuration @unit(s) = default(0s); // Preamble added before the PHY header.
        int headerLength @unit(b) = default(96b); // Number of bits in the PHY header.
        double bitrate @unit(bps); // The bitrate of the transmitter is used to compute the duration of the PHY header and the data part.
        @class(GenericTransmitter);
        @display("i=block/wtx");
}
