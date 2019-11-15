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

package inet.node.packetdrill;

import inet.applications.packetdrill.PacketDrillApp;
import inet.node.inet.StandardHost;
import inet.transportlayer.contract.ISctp;
import inet.transportlayer.contract.ITcp;
import inet.transportlayer.contract.IUdp;

//
// IPv4 host with SCTP, TCP, UDP layers and a packetdrill application.
// The application has a direct connection to the ~TunInterface.
//
module PacketDrillHost extends StandardHost
{
    parameters:
        @display("i=device/pc2;bgl=4");
        int numTcpTunApps = default(0);    // no of TCP packetdrill apps.
        int numUdpTunApps = default(0);    // no of UDP packetdrill apps.
        int numSctpTunApps = default(0);   // no of SCTP packetdrill apps.
        hasTcp = default(true || numTcpTunApps>0);
        hasUdp = default(true || numUdpTunApps>0);
        hasSctp = default(false || numSctpTunApps>0);
    submodules:
        tunApp[numTunInterfaces]: PacketDrillApp {
            parameters:
                interface = "tun"+string(index);
                @display("p=825,76,row,150");
        }

    connections allowunconnected:
        for i=0..numTunInterfaces-1 {
            tunApp[i].socketOut --> at.in++;
            tunApp[i].socketIn <-- at.out++;
        }
}

