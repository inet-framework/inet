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

import inet.linklayer.contract.IMACProtocol;
import inet.physicallayer.radio.contract.IRadio;
import inet.power.contract.IPowerConsumer;


module WirelessNic like IWirelessNic
{
    parameters:
        string interfaceTableModule;
        string powerSourceModule;
        string macType;
        string radioType;
        string powerConsumerType = default("");
        *.interfaceTableModule = default(absPath(interfaceTableModule));
        *.powerSourceModule = default(absPath(powerSourceModule));
        @display("i=block/ifcard;bgb=250,300");
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input radioIn @labels(IRadioFrame);
    submodules:
        mac: <macType> like IMACProtocol {
            parameters:
                @display("p=150,100");
        }
        radio: <radioType> like IRadio {
            parameters:
                @display("p=150,200");
        }
        powerConsumer: <powerConsumerType> like IPowerConsumer if powerConsumerType != "" {
            parameters:
                @display("p=50,50");
        }
    connections:
        radioIn --> { @display("m=s"); } --> radio.radioIn;
        radio.upperLayerIn <-- mac.lowerLayerOut;
        radio.upperLayerOut --> mac.lowerLayerIn;
        upperLayerIn --> { @display("m=n"); } --> mac.upperLayerIn;
        upperLayerOut <-- { @display("m=n"); } <-- mac.upperLayerOut;
}
