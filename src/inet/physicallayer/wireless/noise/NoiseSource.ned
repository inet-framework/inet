//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.physicallayer.wireless.noise;

import inet.mobility.contract.IMobility;
import inet.physicallayer.wireless.common.contract.packetlevel.IAntenna;
import inet.physicallayer.wireless.common.contract.packetlevel.IRadio;
import inet.physicallayer.wireless.common.contract.packetlevel.ITransmitter;

module NoiseSource like IRadio
{
    parameters:
        string radioMediumModule = default("radioMedium"); // module path of the medium module where this radio communicates
        volatile double sleepInterval @unit(s);
        *.mobilityModule = default(absPath(".mobility"));
        @networkNode;
        @class(NoiseSource);
        @display("i=block/wtx");
    gates:
        input upperLayerIn @labels(ILinkLayerFrame/down) @loose;
        output upperLayerOut @labels(ILinkLayerFrame/up) @loose;
        input radioIn @labels(IWirelessSignal) @loose;
    submodules:
        mobility: <default("StationaryMobility")> like IMobility {
            parameters:
                @display("p=100,100");
        }
        antenna: <default("IsotropicAntenna")> like IAntenna {
            parameters:
                @display("p=100,200");
        }
        transmitter: <default("NoiseScalarTransmitter")> like ITransmitter {
            parameters:
                @display("p=100,300");
        }
}

