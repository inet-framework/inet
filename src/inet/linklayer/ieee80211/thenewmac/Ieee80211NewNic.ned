//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

package inet.linklayer.ieee80211.thenewmac;

import inet.linklayer.contract.IWirelessNic;
import inet.linklayer.ieee80211.thenewmac.sap.Ieee80211MacPhySap;
import inet.linklayer.ieee80211.thenewmac.sap.Ieee80211MacSmMlmeSap;
import inet.linklayer.ieee80211.thenewmac.sap.Ieee80211NewMacMacSap;
import inet.physicallayer.contract.packetlevel.IRadio;


module Ieee80211NewNic like IWirelessNic
{
    parameters:
        string interfaceTableModule;
        string radioType;
        *.interfaceTableModule = default(absPath(interfaceTableModule));

        @display("bgb=276,347");
    gates:
        input radioIn @labels(IRadioFrame); // to receive radio frames
        input upperLayerIn;    // from upper layers
        output upperLayerOut;  // to upper layers

    submodules:
        radio: <radioType> like IRadio {
            parameters:
                @display("p=119,260");
        }
        mac: Ieee80211NewMac {
            @display("p=119,42");
        }
        sap: Ieee80211NewMacMacSap {
            @display("p=194,142");
        }
        mlmeSap: Ieee80211MacSmMlmeSap {

            @display("p=44,142");
        }
        phySap: Ieee80211MacPhySap {
            @display("p=119,142");
        }
    connections allowunconnected:
        radioIn --> { @display("m=s"); } --> radio.radioIn;

        mac.mlmeSap <--> mlmeSap.smMlmeSap;
        mac.macSap <--> sap.macSap;
        mac.phySapRx <--> phySap.phySapRx;
        mac.phySapTx <--> phySap.phySapTx;

        upperLayerIn --> sap.upperLayerIn;
        upperLayerOut <-- sap.upperLayerOut;
        radio.upperLayerIn <-- phySap.lowerLayerOut;
        radio.upperLayerOut --> phySap.lowerLayerIn;
}
