//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.apsk.bitlevel;

import inet.physicallayer.wireless.common.base.packetlevel.SnirReceiverBase;
import inet.physicallayer.wireless.common.contract.bitlevel.IAnalogDigitalConverter;
import inet.physicallayer.wireless.common.contract.bitlevel.IDecoder;
import inet.physicallayer.wireless.common.contract.bitlevel.IDemodulator;
import inet.physicallayer.wireless.common.contract.bitlevel.ILayeredErrorModel;
import inet.physicallayer.wireless.common.contract.bitlevel.IPulseFilter;
import inet.physicallayer.wireless.common.contract.packetlevel.IReceiver;

//
// This receiver model is part of a simple hypothetical layered radio. It
// receives detailed transmissions that have separate representation for
// all simulated domains. The levelOfDetail parameter controls which domains
// are actually simulated, but all parameters relevant to the error model are
// expected to be set on the reception.
//
// With the highest level of detail, the reception contains the following:
//  - a separate packet to represent the PHY frame
//  - precise bit representation of the whole packet
//  - precise bit representation before descrambling, fec decoding, and deinterleaving
//  - precise symbol representation of the whole packet including physical header
//  - narrowband analog representation using scalar transmission power
//
// NOTE: the current implementation doesn't support pulse filtering and analog
// digital conversion.
//
// This receiver model supports descrambling, convolutional decoding, deinterleaving,
// and demodulating various amplitude and phase-shift modulations. The main
// purpose of this model is to provide a basic infrastructure for further
// development and experimentation.
//
// @see ~ApskDemodulator, ~ApskDecoder, ~ApskLayeredTransmitter.
//
module ApskLayeredReceiver extends SnirReceiverBase like IReceiver
{
    parameters:
        string levelOfDetail @enum("packet", "bit", "symbol", "sample") = default("symbol");
        double energyDetection @unit(dBm); // TODO @unit(W) + dBm/dBW <--> W
        double sensitivity @unit(dBm); // TODO @unit(W) + dBm/dBW <--> W
        double centerFrequency @unit(Hz);
        double bandwidth @unit(Hz);
        @display("i=block/wrx");
        @class(ApskLayeredReceiver);

    submodules:
        decoder: <default("ApskDecoder")> like IDecoder if typename != "" {
            @display("p=100,100");
        }
        demodulator: <default("ApskDemodulator")> like IDemodulator {
            @display("p=100,200");
        }
        pulseFilter: <default("")> like IPulseFilter if typename != "" {
            @display("p=100,300");
        }
        analogDigitalConverter: <default("")> like IAnalogDigitalConverter if typename != "" {
            @display("p=100,400");
        }
        errorModel: <default("ApskLayeredErrorModel")> like ILayeredErrorModel if typename != "" {
            @display("p=100,500");
        }
}

