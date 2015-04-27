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

package inet.examples.sctp.advancedmultipath;

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
   int netID = default(0);        // NOTE: nedID==0 => shared by all networks!
   datarate  = default(1 Mbps);
   delay     = default(0ms);
   ber       = default(0);
   per       = default(0);
   @display("ls=black,3");
}

channel accessChannel extends DatarateChannel
{
   int netID = default(1);        // NOTE: nedID>0 => separate network!
   datarate  = default(1 Gbps);
   delay     = default(0.1ms);
   ber       = default(0);
   per       = default(0);
   @display("ls=blue,5");
}


network testNetwork
{
    // ====== System Parameters =============================================
    parameters:
        int networks          = default(2);
        int systems           = default(1);
        bool sharedBottleneck = default(false);
    @display("bgi=maps/world,s");

    // ====== Network Components ============================================
   submodules:
      // ------ Routing Auto-Configuration --------------------------------
      configurator: IPv4NetworkConfigurator {
         parameters:
            @display("p=850,75;i=abstract/penguin_l,gold");
      }

      // ------ Routers -----------------------------------------------------
      westernBorderRouter[networks] : Router {
         parameters:
            @display("p=500,150,c,150;i=abstract/router_l,gold");
         gates:
            pppg[1 + systems];
      }
      westernCoreRouter[networks] : Router {
         parameters:
            @display("p=700,150,c,150;i=abstract/router_l,gold");
         gates:
            pppg[1 + networks];
      }
      easternCoreRouter[networks] : Router {
         parameters:
            @display("p=1000,150,c,150;i=abstract/router_l,gold");
         gates:
            pppg[1 + networks];
      }
      easternBorderRouter[networks] : Router {
         parameters:
            @display("p=1200,150,c,150;i=abstract/router_l,gold");
         gates:
            pppg[1 + systems];
      }

      // ------ Systems -----------------------------------------------------
      clientMain[systems] : NetPerfMeterHost {
         parameters:
            @display("p=50,50,c,200;i=device/laptop_l,green");
         gates:
            pppg[networks];
      }
      serverMain[systems] : NetPerfMeterHost {
         parameters:
            @display("p=1650,50,c,200;i=device/server_l,green");
         gates:
            pppg[networks];
      }

   // ====== Connections ====================================================
   connections allowunconnected:
      for n=0..networks-1 {
         westernCoreRouter[n].pppg[0]
            <--> coreChannel { netID = 0; @display("ls=red,3"); }
            <--> easternCoreRouter[n].pppg[0];

         // ------ Disjoint paths -------------------------
         westernBorderRouter[n].pppg[0]
            <--> accessChannel { netID = 1000 + n; @display("ls=blue,5"); }
            <--> westernCoreRouter[n].pppg[1] if sharedBottleneck == false;
         easternBorderRouter[n].pppg[0]
            <--> accessChannel { netID = 1000 + n; @display("ls=blue,5"); }
            <--> easternCoreRouter[n].pppg[1] if sharedBottleneck == false;

         // ------ Shared Bottleneck paths ----------------
         westernBorderRouter[n].pppg[0]
            <--> accessChannel { netID = 1000 + n; @display("ls=blue,5"); }
            <--> westernCoreRouter[0].pppg++ if sharedBottleneck == true;
         easternBorderRouter[n].pppg[0]
            <--> accessChannel { netID = 1000 + n; @display("ls=blue,5"); }
            <--> easternCoreRouter[0].pppg++ if sharedBottleneck == true;
      }

      for n=0..networks-1,
         for s=0..systems-1 {
            clientMain[s].pppg[n]
               <--> accessChannel { netID = 0; @display("ls=green,5"); }
               <--> westernBorderRouter[n].pppg++;
            serverMain[s].pppg[n]
               <--> accessChannel { netID = 0; @display("ls=green,5"); }
               <--> easternBorderRouter[n].pppg++;
      }
}
