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
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// *
// *   Contact: dreibh@iem.uni-due.de

package inet.examples.inet.netperfmeter;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.queueing.filter.RedDropper;
import inet.queueing.queue.CompoundPacketQueueBase;
import inet.queueing.queue.PacketQueue;
import ned.DatarateChannel;

module REDQueue extends CompoundPacketQueueBase
{
    submodules:
        red: RedDropper {
            wq = default(0.002);
            minth = default(30);
            maxth = default(90);
            maxp = default(0.10);
            pkrate = default(8333.333);
        }
        fifo: PacketQueue;
    connections:
        in --> red.in;
        red.out --> fifo.in;
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
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=850,75;i=abstract/penguin_l,gold");
        }

        // ------ Routers -----------------------------------------------------
        westernBorderRouter: Router {
            parameters:
                @display("p=500,150,c,150;i=abstract/router_l,gold");
            gates:
                pppg[1 + parent.systems];
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
                pppg[1 + parent.systems];
        }

        // ------ Systems -----------------------------------------------------
        clientMain[systems]: StandardHost {
            parameters:
                forwarding = false;
                @display("p=50,50,c,200;i=device/laptop_l,green");
            gates:
                pppg[1];
        }
        serverMain[systems]: StandardHost {
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
