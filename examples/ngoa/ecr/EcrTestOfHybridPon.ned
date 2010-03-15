//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2010 Kyeong Soo (Joseph) Kim
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


package inet.examples.ngoa.ecr;

//import inet.examples.ethernet.lans.ethernetline;
import ned.DatarateChannel;
import inet.networklayer.autorouting.FlatNetworkConfigurator;
import inet.world.ChannelInstaller;
import inet.nodes.inet.Router;
import inet.nodes.inet.StandardHost;
import inet.nodes.inet.StandardHost2;
import inet.nodes.hybridpon.Olt;
import inet.nodes.hybridpon.Onu;
import inet.linklayer.hybridpon.LambdaSplitter;


network EcrTestOfHybridPon
{
    parameters:
        int n;
        double backbone_rate @unit(bps) = default(1Tbps);
        double backbone_delay @unit(s) = default(5ms);	// RTT = 10ms
        double distribution_rate @unit(bps) = default(100Mbps);
        double feeder_rate @unit(bps) = default(100Mbps);
        @display("bgb=597,372");
    types:
        channel fiber_10g extends DatarateChannel
        {
            delay = 1us;
            datarate = 10Gbps;
        }
    submodules:
        channelInstaller: ChannelInstaller {
            parameters:
                channelClass = "ThruputMeteringChannel";
                channelAttrs = "format=u";
                @display("p=36,28");
        }
        configurator: FlatNetworkConfigurator {
            parameters:
                @display("p=123,28");
        }
        server: StandardHost {
            parameters:
                @display("p=542,193;i=device/server_l");
        }
        router: Router {
            parameters:
                @display("p=442,193");
        }
        olt: Olt {
            @display("p=353,193");
        }
        awg: LambdaSplitter {
            parameters:
                @display("p=237,193");
        }
        onu[n]: Onu {
            @display("p=129,193");
        }
        client[n]: StandardHost2 {
            parameters:
                @display("i=device/laptop;p=44,193");
        }
    connections:
        for i=0..n-1 {
            client[i].ethg++ <--> onu[i].ethg++;
            onu[i].pong <--> fiber_10g <--> awg.demuxg++;
            awg.muxg++ <--> fiber_10g <--> olt.pong++;
        }
        olt.ethg++ <--> router.ethg++;
        router.pppg++ <--> {  delay = backbone_delay; datarate = backbone_rate; } <--> server.pppg++;
}
