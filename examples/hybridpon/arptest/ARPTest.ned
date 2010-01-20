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
    @display("bgb=687,326;bgl=2");
    types:
        channel fiberline extends DatarateChannel
        {
            delay = 1us;
            datarate = 512Mbps;
        }
    submodules:
        client: StandardHost {
            @display("p=42,95;i=device/laptop_l");
        }
        host1: StandardHost {
            @display("p=42,172;i=device/pc");
        }
        host2: StandardHost {
            @display("p=42,257;i=device/pc");
        }
        olt: Olt {
            @display("i=device/router;p=324,161");
        }
        onu1: Onu {
            @display("i=device/pc2;p=171,95");
        }
        onu2: Onu {
            @display("i=device/pc2;p=171,218");
        }
        awg: LambdaSplitter {
            @display("i=abstract/opticalswitch_s;p=256,161");
        }
        //        switch: EtherSwitch {
        //            @display("p=202,156");
        //        }
        net: Router {
            @display("p=510,161");
        }
        router: Router {
            @display("p=410,161");
        }
        server: StandardHost {
            @display("p=634,161;i=device/server_l");
        }
        configurator: FlatNetworkConfigurator {
            @display("p=541,265");
        }
    connections:
        client.ethg++ <--> onu1.ethg++;
        host1.ethg++ <--> onu2.ethg++;
        host2.ethg++ <--> onu2.ethg++;
        onu1.pong <--> awg.demuxg++;
        onu2.pong <--> awg.demuxg++;
        awg.muxg <--> olt.pong;
        olt.ethg <--> router.ethg++;
        router.pppg++ <--> fiberline <--> net.pppg++;
        net.pppg++ <--> fiberline <--> server.pppg++;
}

