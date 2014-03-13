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

package inet.nodes.aodv;

import inet.networklayer.manetrouting.aodv.AODVRouting;
import inet.nodes.inet.WirelessHost;


//
// TODO
//
module AODVRouter extends WirelessHost
{
    parameters:
        @display("i=device/cellphone");
        IPForward = true;
        wlan[*].mgmtType = default("Ieee80211MgmtAdhoc");
    submodules:
        aodv: AODVRouting {
            @display("p=558,307");
        }
    connections:
        aodv.ipOut --> networkLayer.transportIn++;
        aodv.ipIn <-- networkLayer.transportOut++;
}
