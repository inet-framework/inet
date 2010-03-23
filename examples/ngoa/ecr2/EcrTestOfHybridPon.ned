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

//import ned.DatarateChannel;
import inet.networklayer.autorouting.FlatNetworkConfigurator;
//import inet.world.ChannelInstaller;
//import inet.nodes.inet.RouterWithEthernetInterface2;
import inet.nodes.inet.StandardHostWithEthernetInterface2;
import inet.nodes.inet.StandardHost2WithEthernetInterface2;
import inet.nodes.hybridpon.Olt;
import inet.nodes.hybridpon.Onu;
import inet.linklayer.hybridpon.LambdaSplitter;


//
// Defines a test network for equivalent circuit rate (ECR)-based
// study of a hybrid TDM/WDM-PON.
//
// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
// @date   2010-03-23
//
network EcrTestOfHybridPon
{
    parameters:
        int N;	// number of clients (subscribers)
        double uni_rate @unit(bps) = default(1Gbps);	// transmission rate of ONU Ethernet UNIs
        double uni_delay @unit(s) = default(0s);	// no delay at UNIs
        double distribution_rate @unit(bps) = default(1Gbps);	// transmission rate of distribution fiber (between Onu and LambdaSplitter)
        double distribution_delay @unit(s) = default(50us);	// default delay of 50 us corresponds to 10 km fiber
        double feeder_rate @unit(bps) = default(1Gbps);	// transmission rate of feeder fiber (between Olt and LambdaSplitter)
        double feeder_delay @unit(s) = default(50us);	// default delay of 50 us corresponds to 10 km fiber
        double backbone_rate @unit(bps) = default(100Gbps);	// transmission rate of backbone link (between Olt and server)
        double backbone_delay @unit(s) = default(5ms);	// default delay of 5 ms corresponds to RTT of 10 ms
        @display("bgb=519,372");
    submodules:
//        channelInstaller: ChannelInstaller {
//            parameters:
//                channelClass = "ThruputMeteringChannel";
//                channelAttrs = "format=u";
//                @display("p=62,33");
//        }
        configurator: FlatNetworkConfigurator {
            parameters:
                @display("p=160,33");
        }
        client[N]: StandardHost2WithEthernetInterface2 {
            parameters:
                @display("i=device/laptop;p=44,198");
        }
        onu[N]: Onu {
            @display("p=129,198");
        }
        awg: LambdaSplitter {
            parameters:
                @display("p=237,198");
        }
        olt: Olt {
            @display("p=353,198");
        }
        server: StandardHostWithEthernetInterface2 {
            parameters:
                @display("p=462,198;i=device/server_l");
        }
    connections:
        for i=0..N-1 {
            client[i].ethg++ <--> {  delay = uni_delay; datarate = uni_rate; } <--> onu[i].ethg++;
            onu[i].phyg <--> {  delay = distribution_delay; datarate = distribution_rate; } <--> awg.demuxg++;
            awg.muxg++ <--> {  delay = feeder_delay; datarate = feeder_rate; } <--> olt.phyg++;
        }
        olt.ethg++ <--> {  delay = backbone_delay; datarate = backbone_rate; } <--> server.ethg++;
}
