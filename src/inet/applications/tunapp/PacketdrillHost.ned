//
// Copyright (C) 2015 Irene Ruengeler
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

package inet.applications.tunapp;

import inet.transportlayer.contract.IUDP;
import inet.linklayer.tun.TunInterface;
import inet.applications.contract.ITunApp;
import inet.node.inet.NodeBase;


//
// IPv4 host with UDP layer and a packetdrill application.
// The application has a direct connection to the TunInterface.
//
module PacketdrillHost extends NodeBase
{
    parameters:
        @display("i=device/pc2;bgl=4;bgb=588,447");
        int numUdpTunApps = default(0);    // no of UDP packetdrill apps.
        bool hasUdpTun = default(numUdpTunApps>0);
        string udpType = default(firstAvailableOrEmpty("UDP"));
    submodules:
        tunApp[numTunInterfaces] : PDApp {
            parameters:
                @display("p=280,40");
        }
        udp: <udpType> like IUDP if hasUdpTun {
            parameters:
                @display("p=330,140");
        }

    connections allowunconnected:
        for i=0..numUdpTunApps-1 {
            tunApp[i].udpOut --> udp.appIn++ if hasUdpTun;
            tunApp[i].udpIn <-- udp.appOut++ if hasUdpTun;
        }

        udp.ipOut --> networkLayer.transportIn++ if hasUdpTun;
        udp.ipIn <-- networkLayer.transportOut++ if hasUdpTun;

        for i=0..numTunInterfaces-1 {
            tunApp[i].tunOut --> tun[i].appIn;
            tunApp[i].tunIn <-- tun[i].appOut;
        }
}
