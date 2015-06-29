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
import inet.linklayer.ieee80211.thenewmac.sap.Ieee80211NewMacMacSap;


module Ieee80211NewNic like IWirelessNic
{
    parameters:
        string interfaceTableModule;
        *.interfaceTableModule = default(absPath(interfaceTableModule));

    gates:
        input radioIn @labels(IRadioFrame); // to receive radio frames
        input upperLayerIn;    // from upper layers
        output upperLayerOut;  // to upper layers

    submodules:
        mac: Ieee80211NewMac;
        sap: Ieee80211NewMacMacSap;

    connections allowunconnected:
        mac.macSap <--> sap.macSap;
        upperLayerIn --> sap.upperLayerIn;
        upperLayerOut <-- sap.upperLayerOut;
}
