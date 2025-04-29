//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.apsk.bitlevel;

import inet.common.CompoundModule;
import inet.physicallayer.wireless.common.contract.packetlevel.ITransmitter;
import inet.physicallayer.wireless.common.contract.bitlevel.IDigitalAnalogConverter;
import inet.physicallayer.wireless.common.contract.bitlevel.IEncoder;
import inet.physicallayer.wireless.common.contract.bitlevel.IModulator;
import inet.physicallayer.wireless.common.contract.bitlevel.IPulseShaper;

//
// This transmitter model is part of a simple hypothetical layered radio. It
// produces detailed transmissions that have a separate representation for
// all simulated domains. The `levelOfDetail` parameter controls which domains
// are actually simulated, but all parameters relevant to the error model are
// always set on the transmission.
//
// With the highest level of detail, the produced transmission contains the
// following:
//  - a separate packet to represent the PHY frame
//  - precise bit representation of the entire packet
//  - precise bit representation after scrambling, FEC encoding, and interleaving
//  - precise symbol representation of the entire packet including the physical header
//  - narrowband analog representation using scalar transmission power
//
// NOTE: the current implementation doesn't support pulse shaping and digital
// analog conversion.
//
// This transmitter model supports scrambling, convolutional coding, interleaving,
// and modulating various amplitude and phase-shift modulations. The main purpose
// of this model is to provide a basic infrastructure for further development and
// experimentation.
//
// @see ~ApskEncoder, ~ApskModulator, ~ApskLayeredReceiver.
//
module ApskLayeredTransmitter extends CompoundModule like ITransmitter
{
    parameters:
        string levelOfDetail @enum("packet", "bit", "symbol", "sample") = default("symbol");
        double bitrate @unit(bps); // Net bitrate of the transmitter
        double power @unit(W); // Scalar transmission power for the entire signal duration
        double centerFrequency @unit(Hz); // Center frequency of the narrowband carrier signal
        double bandwidth @unit(Hz); // Bandwidth
        double preambleDuration @unit(s) = default(0s);
        @display("i=block/wtx");
        @class(ApskLayeredTransmitter);

    submodules:
        encoder: <default("ApskEncoder")> like IEncoder if typename != "" {
            @display("p=100,100");
        }
        modulator: <default("ApskModulator")> like IModulator {
            @display("p=100,200");
        }
        pulseShaper: <default("")> like IPulseShaper if typename != "" {
            @display("p=100,300");
        }
        digitalAnalogConverter: <default("")> like IDigitalAnalogConverter if typename != "" {
            @display("p=100,400");
        }
}

