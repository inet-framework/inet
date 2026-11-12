//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.prototype.cellular;

//
// One sector of a base station: one ~PanelAntenna per band, one ~CellularRadio per component
// carrier, and one ~GnbMac above all radios. The radio of carrier i uses panel[0] by default;
// a configuration with several bands sets `radio[i].antenna.antennaModule` to the panel of the
// band of carrier i.
//
module GnbSector
{
    parameters:
        int numCarriers = default(1); // Number of component carriers of the sector
        int numBands = default(1); // Number of bands, one panel antenna each
        double boresight @unit(deg) = default(0deg); // Azimuth of the panels' main lobe, relative to the orientation of the node
        @display("i=block/wrxtx");
    submodules:
        panel[numBands]: PanelAntenna {
            parameters:
                boresight = default(parent.boresight);
                mobilityModule = default("^.^.mobility");
                @display("p=100,100,row,100");
        }
        mac: GnbMac {
            parameters:
                @display("p=300,100");
        }
        radio[numCarriers]: CellularRadio {
            parameters:
                isBaseStation = true;
                antenna.antennaModule = default("^.^.panel[0]");
                transmitter.power = default(40W);
                @display("p=300,250,row,150");
        }
    connections allowunconnected:
        for i=0..numCarriers-1 {
            mac.lowerLayerOut++ --> radio[i].upperLayerIn;
            radio[i].upperLayerOut --> mac.lowerLayerIn++;
        }
}
