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

import inet.physicallayer.base.packetlevel.SnirReceiverBase;
import inet.physicallayer.contract.bitlevel.IAnalogDigitalConverter;
import inet.physicallayer.contract.bitlevel.IDecoder;
import inet.physicallayer.contract.bitlevel.IDemodulator;
import inet.physicallayer.contract.bitlevel.ILayeredErrorModel;
import inet.physicallayer.contract.bitlevel.IPulseFilter;
import inet.physicallayer.contract.packetlevel.IReceiver;

//
// This module implements an IEEE 802.11 OFDM receiver.
// The implemenation details are based on the following standard:
// IEEE Std 802.11-2012 PART 11: WIRELESS LAN MAC AND PHY SPECIFICATIONS
//
// Please check the ~Ieee80211LayeredOfdmTransmitter documentation first, since
// this receiver module is symmetric to it.
//
module Ieee80211LayeredOfdmReceiver extends SnirReceiverBase like IReceiver
{
    parameters:
        bool isCompliant = default(true);
        string levelOfDetail @enum("packet", "bit","symbol","sample") = default("symbol");
        double energyDetection @unit(dBm); // TODO: @unit(W) + dBm/dBW <--> W
        double sensitivity @unit(dBm); // TODO: @unit(W) + dBm/dBW <--> W
        double centerFrequency @unit(Hz);
        double bandwidth @unit(Hz);
        double channelSpacing @unit(Hz);
        @class(Ieee80211LayeredOfdmReceiver);

        @display("i=block/rx");
    submodules:
        errorModel: <default("")> like ILayeredErrorModel if typename != "" {
            @display("p=300,400");
        }
        dataDecoder: <default("")> like IDecoder if typename != "" {
            @display("p=100,100");
        }
        signalDecoder: <default("")> like IDecoder if typename != "" {
            @display("p=300,100");
        }
        signalDemodulator: <default("")> like IDemodulator if typename != "" {
            @display("p=300,200");
        }
        dataDemodulator: <default("")> like IDemodulator if typename != "" {
            @display("p=100,200");
        }
        pulseFilter: <default("")> like IPulseFilter if typename != "" {
            @display("p=100,300");
        }
        analogDigitalConverter: <default("")> like IAnalogDigitalConverter if typename != "" {
            @display("p=100,400");
        }
}

