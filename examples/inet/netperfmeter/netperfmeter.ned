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
// *   Copyright (C) 2009-2015 by Thomas Dreibholz
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
// *   Contact: dreibh@iem.uni-due.de

package inet.examples.inet.netperfmeter;

import inet.applications.netperfmeter.NetPerfMeterHost;
import inet.common.queue.FIFOQueue;
import inet.common.queue.IOutputQueue;
import inet.common.queue.REDDropper;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.inet.Router;
import ned.DatarateChannel;


module REDQueue like IOutputQueue
{
    gates:
        input in;
        output out;

    submodules:
        red: REDDropper {
            numGates = 1;
            wq = default(0.002);
            minths = default("30");
            maxths = default("90");
            maxps = default("0.10");
            pkrates = default("8333.333");
        }
        fifo: FIFOQueue;
    connections:
        in --> red.in[0];
        red.out[0] --> fifo.in++;
        fifo.out --> out;
}


channel coreChannel extends DatarateChannel
{
    datarate = default(1 Mbps);
    delay = default(0ms);
    ber = default(0);
    per = default(0);
    @display("ls=black,3");
}

channel accessChannel extends DatarateChannel
{
    datarate = default(1 Gbps);
    delay = default(0.1ms);
    ber = default(0);
    per = default(0);
    @display("ls=blue,5");
}


network testNetwork
{
    // ====== System Parameters =============================================
    parameters:
        int systems = default(3);
        @display("bgi=maps/world,s");

    // ====== Network Components ============================================
    submodules:
        // ------ Routing Auto-Configuration --------------------------------
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=850,75;i=abstract/penguin_l,gold");
        }

        // ------ Routers -----------------------------------------------------
        westernBorderRouter: Router {
            parameters:
                @display("p=500,150,c,150;i=abstract/router_l,gold");
            gates:
                pppg[1 + systems];
        }
        westernCoreRouter: Router {
            parameters:
                @display("p=700,150,c,150;i=abstract/router_l,gold");
            gates:
                pppg[2];
        }
        easternCoreRouter: Router {
            parameters:
                @display("p=1000,150,c,150;i=abstract/router_l,gold");
            gates:
                pppg[2];
        }
        easternBorderRouter: Router {
            parameters:
                @display("p=1200,150,c,150;i=abstract/router_l,gold");
            gates:
                pppg[1 + systems];
        }

        // ------ Systems -----------------------------------------------------
        clientMain[systems]: NetPerfMeterHost {
            parameters:
                forwarding = false;
                @display("p=50,50,c,200;i=device/laptop_l,green");
            gates:
                pppg[1];
        }
        serverMain[systems]: NetPerfMeterHost {
            parameters:
                forwarding = false;
                @display("p=1650,50,c,200;i=device/server_l,green");
            gates:
                pppg[1];
        }

    // ====== Connections ====================================================
    connections:
        westernCoreRouter.pppg[0] <--> coreChannel { @display("ls=blue,3"); } <--> easternCoreRouter.pppg[0];

        westernBorderRouter.pppg[0] <--> accessChannel { @display("ls=blue,5"); } <--> westernCoreRouter.pppg[1];
        easternBorderRouter.pppg[0] <--> accessChannel { @display("ls=blue,5"); } <--> easternCoreRouter.pppg[1];

        for s=0..systems-1 {
            clientMain[s].pppg[0] <--> accessChannel { @display("ls=green,5"); } <--> westernBorderRouter.pppg[1 + s];
            serverMain[s].pppg[0] <--> accessChannel { @display("ls=green,5"); } <--> easternBorderRouter.pppg[1 + s];
        }
}
