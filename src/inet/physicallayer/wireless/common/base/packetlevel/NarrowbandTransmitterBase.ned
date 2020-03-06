//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.common.base.packetlevel;

import inet.physicallayer.wireless.common.contract.packetlevel.ITransmitter;

//
// This module servces as a base module for narrowband transmitter models.
//
module NarrowbandTransmitterBase like ITransmitter
{
    parameters:
        double preambleDuration @unit(s);  // preamble duration
        int headerLength @unit(b);   // number of header bits added to the length of the MAC frame
        double bitrate @unit(bps);         // the bitrate of the transmitter is used to compute the duration of the transmission
        double power @unit(W);             // transmission power
        double centerFrequency @unit(Hz); // center frequency of the band where the transmitter transmits on the medium
        double bandwidth @unit(Hz);        // bandwidth of the band where the transmitter transmits on the medium
        double codeRate = default(1);
        string modulation;                 // "BPSK", "16-QAM", "256-QAM"
        @display("i=block/wtx");
}
