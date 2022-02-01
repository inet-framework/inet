//
// Copyright (C) 2014 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.ieee80211.bitlevel;

import inet.physicallayer.wireless.common.base.packetlevel.SnirReceiverBase;
import inet.physicallayer.wireless.common.contract.bitlevel.IAnalogDigitalConverter;
import inet.physicallayer.wireless.common.contract.bitlevel.IDecoder;
import inet.physicallayer.wireless.common.contract.bitlevel.IDemodulator;
import inet.physicallayer.wireless.common.contract.bitlevel.ILayeredErrorModel;
import inet.physicallayer.wireless.common.contract.bitlevel.IPulseFilter;
import inet.physicallayer.wireless.common.contract.packetlevel.IReceiver;

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
        double energyDetection @unit(dBm); // TODO @unit(W) + dBm/dBW <--> W
        double sensitivity @unit(dBm); // TODO @unit(W) + dBm/dBW <--> W
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

