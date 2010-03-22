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


package inet.examples.ngoa.ecr2;

//import inet.examples.ethernet.lans.ethernetline;
import ned.DatarateChannel;
import inet.networklayer.autorouting.FlatNetworkConfigurator;
import inet.world.ChannelInstaller;
//import inet.nodes.inet.RouterWithEthernetInterface2;
import inet.nodes.inet.StandardHostWithEthernetInterface2;
import inet.nodes.inet.StandardHost2WithEthernetInterface2;
import inet.nodes.hybridpon.OltWithEtherMAC2;
import inet.nodes.hybridpon.OnuWithEtherMAC2;
import inet.linklayer.hybridpon.LambdaSplitter;


network EcrTestOfHybridPon
{
    parameters:
        int n;
        double uni_delay @unit(s) = default(0s);
        double uni_rate @unit(bps) = default(1Gbps);
        double distribution_delay @unit(s) = default(50us);	// delay corresponding to 10 km fiber
        double distribution_rate @unit(bps) = default(1Gbps);
        double feeder_delay @unit(s) = default(50us);	// delay corresponding to 10 km fiber
        double feeder_rate @unit(bps) = default(1Gbps);
        double backbone_rate @unit(bps) = default(10Gbps);
        double backbone_delay @unit(s) = default(5ms);	// RTT = 10ms
        @display("bgb=519,372");
    types:
//        channel fiber_10g extends DatarateChannel
//        {
//            delay = 1us;
//            datarate = 10Gbps;
//        }
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
        client[n]: StandardHost2WithEthernetInterface2 {
            parameters:
                @display("i=device/laptop;p=44,198");
        }
        onu[n]: OnuWithEtherMAC2 {
            @display("p=129,198");
        }        
        awg: LambdaSplitter {
            parameters:
                @display("p=237,198");
        }
        olt: OltWithEtherMAC2 {
            @display("p=353,198");
        }
        //        router: RouterWithEthernetInterface2 {
        //            parameters:
        //                @display("p=442,193");
        //        }
        server: StandardHostWithEthernetInterface2 {
            parameters:
                @display("p=462,198;i=device/server_l");
        }        
    connections:
        for i=0..n-1 {
            client[i].ethg++ <--> {delay = uni_delay; datarate = uni_rate;} <--> onu[i].ethg++;
            onu[i].pong <--> {delay = distribution_delay; datarate = distribution_rate; } <--> awg.demuxg++;
            awg.muxg++ <--> {delay = feeder_delay; datarate = feeder_rate;} <--> olt.pong++;
        }
        olt.ethg++ <--> {delay = backbone_delay; datarate = backbone_rate;} <--> server.ethg++;
}
