//
// Copyright (C) 2014 OpenSim Ltd
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.physicallayer.apskradio.bitlevel;

import inet.physicallayer.contract.packetlevel.IReceiver;
import inet.physicallayer.contract.bitlevel.IAnalogDigitalConverter;
import inet.physicallayer.contract.bitlevel.IDecoder;
import inet.physicallayer.contract.bitlevel.IDemodulator;
import inet.physicallayer.contract.bitlevel.ILayeredErrorModel;
import inet.physicallayer.contract.bitlevel.IPulseFilter;

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
// See also ~APSKDemodulator, ~APSKDecoder, and ~APSKLayeredTransmitter.
//
module APSKLayeredReceiver like IReceiver
{
    parameters:
        string levelOfDetail @enum("packet", "bit", "symbol", "sample") = default("symbol");
        string errorModelType = default("APSKLayeredErrorModel");
        string decoderType = default("APSKDecoder");
        string demodulatorType = default("APSKDemodulator");
        string pulseFilterType = default("");
        string analogDigitalConverterType = default("");
        double energyDetection @unit(dBm); // TODO: @unit(W) + dBm/dBW <--> W
        double sensitivity @unit(dBm); // TODO: @unit(W) + dBm/dBW <--> W
        double carrierFrequency @unit(Hz);
        double bandwidth @unit(Hz);
        double snirThreshold @unit(dB);
        @display("i=block/wrx");
        @class(APSKLayeredReceiver);

    submodules:
        decoder: <decoderType> like IDecoder if decoderType != "" {
            @display("p=100,50");
        }
        demodulator: <demodulatorType> like IDemodulator {
            @display("p=100,150");
        }
        pulseFilter: <pulseFilterType> like IPulseFilter if pulseFilterType != "" {
            @display("p=100,250");
        }
        analogDigitalConverter: <analogDigitalConverterType> like IAnalogDigitalConverter if analogDigitalConverterType != "" {
            @display("p=100,350");
        }
        errorModel: <errorModelType> like ILayeredErrorModel if errorModelType != "" {
            @display("p=100,450");
        }
}
