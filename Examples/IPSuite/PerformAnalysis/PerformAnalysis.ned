//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//




import
    "Router",
    "BurstHost",
    "FlatNetworkConfigurator";


module PerformanceNetwork
    parameters:
        nodeNo : numeric const;
    submodules:
        configurator: FlatNetworkConfigurator;
            parameters:
                moduleTypes = "Router BurstHost",
                nonIPModuleTypes = "",
                networkAddress = "145.236.0.0",
                netmask = "255.255.0.0";
            display: "p=60,160;i=cogwheel";
        sender: BurstHost[nodeNo];
//            parameters:
//                burstPackets = packetNo,
//                packetSize = 1000;
//            parameters if index==0:
//                routingFile = "sender0.irt";
//            parameters if index==1:
//                routingFile = "sender1.irt";
//            parameters if index==2:
//                routingFile = "sender2.irt";
//            parameters if index==3:
//                routingFile = "sender3.irt";
//            parameters if index==4:
//                routingFile = "sender4.irt";
//            parameters if index==5:
//                routingFile = "sender5.irt";
//            parameters if index==6:
//                routingFile = "sender6.irt";
//            parameters if index==7:
//                routingFile = "sender7.irt";
//            parameters if index==8:
//                routingFile = "sender8.irt";
//            parameters if index==9:
//                routingFile = "sender9.irt";
            gatesizes:
                in[1],
                out[1];
            display: "p=100,285,row;i=comp,yellow,10";

        recip: BurstHost[nodeNo];
//            parameters:
//                burstPackets = 0,
//                packetSize = 10;
//            parameters if index==0:
//                routingFile = "recip0.irt";
//            parameters if index==1:
//                routingFile = "recip1.irt";
//            parameters if index==2:
//                routingFile = "recip2.irt";
//            parameters if index==3:
//                routingFile = "recip3.irt";
//            parameters if index==4:
//                routingFile = "recip4.irt";
//            parameters if index==5:
//                routingFile = "recip5.irt";
//            parameters if index==6:
//                routingFile = "recip6.irt";
//            parameters if index==7:
//                routingFile = "recip7.irt";
//            parameters if index==8:
//                routingFile = "recip8.irt";
//            parameters if index==9:
//                routingFile = "recip9.irt";
            gatesizes:
                in[1],
                out[1];
            display: "p=100,68,row;i=comp";

        router: Router;
//            parameters:
//                routingFile = "router.irt";
            gatesizes:
                in[nodeNo*2],
                out[nodeNo*2];
            display: "p=172,174;i=srouter";

    connections nocheck:
        for i=0..nodeNo-1 do
            sender[i].out[0] --> datarate 1000000 --> router.in[2*i];
            sender[i].in[0] <-- datarate 1000000 <-- router.out[2*i];
            recip[i].out[0] --> datarate 1000000 --> router.in[2*i+1];
            recip[i].in[0] <-- datarate 1000000 <-- router.out[2*i+1];
        endfor;

endmodule

network performanceNetwork : PerformanceNetwork
endnetwork

