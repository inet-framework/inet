//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.prototype.cellular;

import inet.mobility.contract.IMobility;
import inet.physicallayer.wireless.common.contract.packetlevel.IAntenna;

//
// A UE with carrier aggregation: one antenna per band, one ~CellularRadio per component
// carrier, and one ~UeMac above all radios. The radio of carrier i uses panel[0] by default; a
// configuration with several bands sets `radio[i].antenna.antennaModule` to the antenna of the
// band of carrier i.
//
module Ue
{
    parameters:
        int numCarriers = default(1); // Number of component carriers; the same as at the base station sectors
        int numBands = default(1); // Number of bands, one antenna each
        @display("i=device/cellphone");
    submodules:
        mobility: <default("StationaryMobility")> like IMobility {
            @display("p=100,100");
        }
        panel[numBands]: <default("IsotropicAntenna")> like IAntenna {
            parameters:
                mobilityModule = default("^.mobility");
                @display("p=100,250,row,100");
        }
        mac: UeMac {
            parameters:
                @display("p=300,100");
        }
        radio[numCarriers]: CellularRadio {
            parameters:
                isBaseStation = false;
                antenna.antennaModule = default("^.^.panel[0]");
                transmitter.power = default(0.2W);
                @display("p=300,250,row,150");
        }
    connections allowunconnected:
        for i=0..numCarriers-1 {
            mac.lowerLayerOut++ --> radio[i].upperLayerIn;
            radio[i].upperLayerOut --> mac.lowerLayerIn++;
        }
}
