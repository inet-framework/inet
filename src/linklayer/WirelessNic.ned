//
// Copyright (C) 2006 Andras Varga
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


package inet.linklayer;

import inet.linklayer.contract.ILinkLayer;
import inet.physicallayer.radio.contract.IRadio;

module WirelessNic like IWirelessNic
{
    parameters:
        string macType;
        string radioType;
        @display("i=block/ifcard;bgb=102,286");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input radioIn @labels(IRadioFrame);
    submodules:
        mac: <macType> like ILinkLayer {
            parameters:
                @display("p=50,100");
        }
        radio: <radioType> like IRadio {
            parameters:
                @display("p=50,200");
        }
    connections:
        radioIn --> { @display("m=s"); } --> radio.radioIn;
        radio.upperLayerIn <-- mac.lowerLayerOut;
        radio.upperLayerOut --> mac.lowerLayerIn;
        upperLayerIn --> { @display("m=n"); } --> mac.upperLayerIn;
        upperLayerOut <-- { @display("m=n"); } <-- mac.upperLayerOut;
}
