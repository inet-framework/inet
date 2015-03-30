// * --------------------------------------------------------------------------
// *
// *     //====//  //===== <===//===>  //====//
// *    //        //          //      //    //    SCTP Optimization Project
// *   //=====   //          //      //====//   ==============================
// *        //  //          //      //           University of Duisburg-Essen
// *  =====//  //=====     //      //
// *
// * --------------------------------------------------------------------------
// *
// *   Copyright (C) 2015 by Thomas Dreibholz
// *
// *   This program is free software: you can redistribute it and/or modify
// *   it under the terms of the GNU General Public License as published by
// *   the Free Software Foundation, either version 3 of the License, or
// *   (at your option) any later version.
// *
// *   This program is distributed in the hope that it will be useful,
// *   but WITHOUT ANY WARRANTY; without even the implied warranty of
// *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// *   GNU General Public License for more details.
// *
// *   You should have received a copy of the GNU General Public License
// *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
// *
// *   Contact: dreibh@simula.no

package inet.applications.netperfmeter;

import inet.applications.contract.INetPerfMeterApp;
import inet.node.inet.NodeBase;
import inet.transportlayer.contract.ISCTP;
import inet.transportlayer.contract.ITCP;
import inet.transportlayer.contract.IUDP;


//
// NetPerfMeter application host.
//
module NetPerfMeterHost extends NodeBase
{
    parameters:
        @display("i=device/pc2");
        int numNetPerfMeterApps = default(0);   // T.D. 17.11.09
        string tcpType = default(firstAvailable("TCP", "TCP_lwIP", "TCP_NSC", "TCP_None"));  // tcp implementation (e.g. ~TCP, ~TCP_lwIP, ~TCP_NSC) or ~TCPSpoof
        string udpType = default(firstAvailable("UDP", "UDP_None"));
        string sctpType = default(firstAvailable("SCTP", "SCTP_None"));
        forwarding = default(false);  // disable routing by default

    submodules:
        netPerfMeterApp[numNetPerfMeterApps]: <default("NetPerfMeter")> like INetPerfMeterApp {  // T.D. 17.11.09
            parameters:
                @display("p=262,36");
        }

        tcp: <tcpType> like ITCP if (numNetPerfMeterApps > 0) {
            parameters:
                @display("p=147,141");
        }
        udp: <udpType> like IUDP if (numNetPerfMeterApps > 0) {
            parameters:
                @display("p=329,141");
        }
        sctp: <sctpType> like ISCTP if (numNetPerfMeterApps > 0) {
            @display("p=527,141");
        }

    connections allowunconnected:
        for i=0..numNetPerfMeterApps-1 {
            netPerfMeterApp[i].tcpOut --> tcp.appIn++;
            netPerfMeterApp[i].tcpIn <-- tcp.appOut++;
            netPerfMeterApp[i].sctpOut --> sctp.from_appl++;
            netPerfMeterApp[i].sctpIn <-- sctp.to_appl++;
            netPerfMeterApp[i].udpOut --> udp.appIn++;
            netPerfMeterApp[i].udpIn <-- udp.appOut++;
        }

        tcp.ipOut --> networkLayer.transportIn++ if (numNetPerfMeterApps > 0);
        tcp.ipIn <-- networkLayer.transportOut++ if (numNetPerfMeterApps > 0);
        udp.ipOut --> networkLayer.transportIn++ if (numNetPerfMeterApps > 0);
        udp.ipIn <-- networkLayer.transportOut++ if (numNetPerfMeterApps > 0);
        sctp.to_ip --> networkLayer.transportIn++ if (numNetPerfMeterApps > 0);
        networkLayer.transportOut++ --> sctp.from_ip if (numNetPerfMeterApps > 0);
}
