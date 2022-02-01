//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.unitdisk;

import inet.physicallayer.wireless.common.contract.packetlevel.ITransmitter;

//
// This transmitter model produces transmissions that are parameterized with
// communication range, interference range, and detection range. It also
// supports an ideal communication channel with configuring the range parameters
// to infinity.
//
// @see ~UnitDiskReceiver, ~UnitDiskRadio, ~UnitDiskRadioMedium,
// ~UnitDiskAnalogModel.
//
module UnitDiskTransmitter like ITransmitter
{
    parameters:
        double preambleDuration @unit(s) = default(0s);                  // preamble added before the header
        int headerLength @unit(b) = default(96b);                     // number of header bits added to the length of the MAC frame
        double bitrate @unit(bps);                                       // the bitrate of the transmitter is used to compute the duration of the transmission
        double communicationRange @unit(m);                              // the range where the reception of transmissions produced by this transmitter is possible
        double interferenceRange @unit(m) = default(communicationRange); // the range where transmissions produced by this transmitter interferes with other transmissions
        double detectionRange @unit(m) = default(interferenceRange);     // the range where transmissions produced by this transmitter can be detected at all
        @class(UnitDiskTransmitter);
        @display("i=block/wtx");
}
