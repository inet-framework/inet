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

import inet.applications.contract.IApp;
import inet.common.MessageDispatcher;
import inet.node.base.TransportLayerNodeBase;


//
// NetPerfMeter application host.
//
module NetPerfMeterHost extends TransportLayerNodeBase
{
    parameters:
        @figure[submodules];
        @display("i=device/pc2");
        int numNetPerfMeterApps = default(0);   // T.D. 17.11.09
        forwarding = default(false);  // disable routing by default
        *.routingTableModule = default("^.ipv4.routingTable");

    submodules:
        netPerfMeterApp[numNetPerfMeterApps]: <default("NetPerfMeter")> like IApp {  // T.D. 17.11.09
            parameters:
                @display("p=375,76,row,150");
        }

        at: MessageDispatcher {
            parameters:
                @display("p=750,146;b=1000,5,,,,1");
        }

    connections allowunconnected:
        for i=0..numNetPerfMeterApps-1 {
            netPerfMeterApp[i].socketOut --> at.in++;
            netPerfMeterApp[i].socketIn <-- at.out++;
        }

        at.out++ --> udp.appIn if hasUdp;
        at.in++ <-- udp.appOut if hasUdp;

        at.out++ --> tcp.appIn if hasTcp;
        at.in++ <-- tcp.appOut if hasTcp;

        at.out++ --> sctp.appIn if hasSctp;
        at.in++ <-- sctp.appOut if hasSctp;

        at.out++ --> tn.in++;
        at.in++ <-- tn.out++;
}

