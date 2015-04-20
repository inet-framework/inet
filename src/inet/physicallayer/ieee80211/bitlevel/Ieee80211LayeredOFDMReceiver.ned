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

package inet.physicallayer.ieee80211.bitlevel;

import inet.physicallayer.contract.packetlevel.IReceiver;
import inet.physicallayer.contract.bitlevel.IAnalogDigitalConverter;
import inet.physicallayer.contract.bitlevel.IDecoder;
import inet.physicallayer.contract.bitlevel.IDemodulator;
import inet.physicallayer.contract.bitlevel.ILayeredErrorModel;
import inet.physicallayer.contract.bitlevel.IPulseFilter;

//
// This module implements an IEEE 802.11 OFDM receiver.
// The implemenation details are based on the following standard:
// IEEE Std 802.11-2012 PART 11: WIRELESS LAN MAC AND PHY SPECIFICATIONS

// Please check the @see ~Ieee80211LayeredOFDMTransmitter documentation first, since
// this receiver module is symmetric to it.
//
//
module Ieee80211LayeredOFDMReceiver like IReceiver
{
    parameters:
        bool isCompliant = default(true);
        string levelOfDetail @enum("packet", "bit","symbol","sample") = default("symbol");
        string errorModelType = default("");
        string signalDecoderType = default("");
        string dataDecoderType = default("");
        string signalDemodulatorType = default("");
        string dataDemodulatorType = default("");
        string pulseFilterType = default("");
        string analogDigitalConverterType = default("");
        double energyDetection @unit(dBm); // TODO: @unit(W) + dBm/dBW <--> W
        double sensitivity @unit(dBm); // TODO: @unit(W) + dBm/dBW <--> W
        double carrierFrequency @unit(Hz);
        double bandwidth @unit(Hz);
        double snirThreshold @unit(dB);
        double channelSpacing @unit(Hz);
        @class(Ieee80211LayeredOFDMReceiver);

        @display("i=block/rx");
    submodules:
        errorModel: <errorModelType> like ILayeredErrorModel if errorModelType != "" {
            @display("p=240,350");
        }
        dataDecoder: <dataDecoderType> like IDecoder if dataDecoderType != "" {
            @display("p=100,50");
        }
        signalDecoder: <signalDecoderType> like IDecoder if signalDecoderType != "" {
            @display("p=240,50");
        }
        signalDemodulator: <signalDemodulatorType> like IDemodulator if signalDemodulatorType != "" {
            @display("p=240,150");
        }
        dataDemodulator: <dataDemodulatorType> like IDemodulator if dataDemodulatorType != "" {
            @display("p=100,150");
        }
        pulseFilter: <pulseFilterType> like IPulseFilter if pulseFilterType != "" {
            @display("p=100,250");
        }
        analogDigitalConverter: <analogDigitalConverterType> like IAnalogDigitalConverter if analogDigitalConverterType != "" {
            @display("p=100,350");
        }
}
