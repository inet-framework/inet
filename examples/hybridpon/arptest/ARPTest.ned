//
// Copyright (C) 2006 Andras Varga
//
// This library is free software, you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//


package inet.examples.hybridpon.arptest;

import inet.networklayer.autorouting.FlatNetworkConfigurator;
import inet.linklayer.hybridpon.LambdaSplitter;
import inet.nodes.hybridpon.Olt;
import inet.nodes.hybridpon.Onu;
import inet.nodes.inet.Router;
import inet.nodes.inet.StandardHost;
import ned.DatarateChannel;


network ARPTest
{
    parameters:
        int numOnus = 2;
	    @display("bgb=611,326;bgl=2");
    types:
        channel fiberline extends DatarateChannel
        {
            delay = 1us;
            datarate = 512Mbps;
        }
        channel fiber_10g extends DatarateChannel
        {
            delay = 1us;
            datarate = 10Gbps;
        }
    submodules:
        client: StandardHost {
            parameters:
                @display("p=42,95;i=device/laptop");
        }
        host1: StandardHost {
            parameters:
                @display("p=42,172;i=device/pc");
        }
        host2: StandardHost {
            parameters:
                @display("p=42,257;i=device/pc");
        }
        olt: Olt {
            parameters:
                @display("p=324,161");
        }
        onu1: Onu {
            parameters:
                @display("p=171,95");
        }
        onu2: Onu {
            parameters:
                @display("p=171,218");
        }
        awg: LambdaSplitter {
            parameters:
                @display("p=256,161");
        }
        //                switch: EtherSwitch {
        //                    @display("p=202,156");
        //                }
        //        net: Router {
        //            @display("p=510,161");
        //        }
        router: Router {
            parameters:
                @display("p=410,161");
        }
        server: StandardHost {
            parameters:
                @display("p=552,161;i=device/server_l");
        }
        configurator: FlatNetworkConfigurator {
            parameters:
                @display("p=318,30");
        }
    connections:
        client.ethg++ <--> onu1.ethg++;
        host1.ethg++ <--> onu2.ethg++;
        host2.ethg++ <--> onu2.ethg++;
        onu1.pong <--> fiber_10g <--> awg.demuxg++;
        onu2.pong <--> fiber_10g <--> awg.demuxg++;
        for i=0..numOnus-1 {
        	awg.muxg++ <--> fiber_10g <--> olt.pong++;
        }
        olt.ethg++ <--> router.ethg++;
        router.pppg++ <--> fiberline <--> server.pppg++;
}
