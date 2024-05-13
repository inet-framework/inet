//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.base.packetlevel;

import inet.physicallayer.wireless.common.contract.packetlevel.ITransmitter;

//
// This module serves as a base module for narrowband transmitter models.
//
module NarrowbandTransmitterBase extends TransmitterBase like ITransmitter
{
    parameters:
        double preambleDuration @unit(s);  // Preamble duration
        int headerLength @unit(b);   // Number of header bits added to the length of the MAC frame
        double bitrate @unit(bps);         // The bitrate of the transmitter is used to compute the duration of the transmission
        double power @unit(W);             // Transmission power
        double centerFrequency @unit(Hz); // Center frequency of the band where the transmitter transmits on the medium
        double bandwidth @unit(Hz);        // Bandwidth of the band where the transmitter transmits on the medium
        double codeRate = default(1);
        string modulation;                 // "BPSK", "16-QAM", "256-QAM"
        analogModel.power = default(this.power);
        analogModel.defaultCenterFrequency = default(this.centerFrequency);
        analogModel.defaultBandwidth = default(this.bandwidth);
        @display("i=block/wtx");
}
