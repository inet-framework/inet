//
// Copyright (C) 2010 Kyeong Soo (Joseph) Kim
// Copyright (C) 2004 Andras Varga
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

import inet.networklayer.autorouting.FlatNetworkConfigurator;
//import inet.world.ChannelInstaller;
import inet.nodes.ethernet.EtherSwitch3;
import inet.nodes.inet.StandardHost3WithEthernetInterface2;
import inet.nodes.inet.StandardServerWithEthernetInterface2;


//
// Defines a test network for equivalent circuit rate (ECR)-based
// study of dedicated access network models.
//
// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
// @date   2010-03-26
//
network EcrTestOfDedicatedAccess
{
    parameters:
        int N = default(16);	// number of ONUs (subscribers)
        int n = default(1);	// number of hosts (users) per ONU (subscriber)
        double uni_rate @unit(bps) = default(10Gbps);	// transmission rate of ONU Ethernet UNIs
        double uni_delay @unit(s) = default(0s);	// no delay at UNIs
        double distribution_rate @unit(bps) = default(10Gbps);	// transmission rate of distribution fiber (between Onu and LambdaSplitter)
        double distribution_delay @unit(s) = default(300us);	// default delay of 300 us corresponds to 60 km fiber;
        														// note that there is no feeder fiber in dedicated access unlike PONs.
//        double feeder_rate @unit(bps) = default(10Gbps);	// transmission rate of feeder fiber (between Olt and LambdaSplitter)
//        double feeder_delay @unit(s) = default(150us);	// default delay of 150 us corresponds to 30 km fiber
        double backbone_rate @unit(bps) = default(1Tbps);	// transmission rate of backbone link (between Olt and server)
        double backbone_delay @unit(s) = default(4.7ms);	// default delay of 4.7 ms gives total RTT of 10 ms including delays in access
        @display("bgb=519,184");
    submodules:
        //        channelInstaller: ChannelInstaller {
        //            parameters:
        //                channelClass = "ThruputMeteringChannel";
        //                channelAttrs = "format=u";
        //                @display("p=98,50");
        //        }
        configurator: FlatNetworkConfigurator {
            parameters:
                @display("p=52,42");
        }

        server: StandardServerWithEthernetInterface2 {
            parameters:
                @display("p=464,119;i=device/server_l");
        }
        olt: EtherSwitch3 {
            parameters:
                @display("p=300,119");
        }
        onu[N]: EtherSwitch3 {
            parameters:
                @display("p=131,119");
        }
        host[N*n]: StandardHost3WithEthernetInterface2 {
            parameters:
                @display("i=device/laptop;p=46,119");
        }
    connections:
        for i=0..N-1, for j=0..n-1 {
			host[i*n+j].ethg++ <--> {  delay = uni_delay; datarate = uni_rate; } <--> onu[i].ethg++;
			onu[i].ethg++ <--> {  delay = distribution_delay; datarate = distribution_rate; } <--> olt.ethg++;
        }
        olt.ethg++ <--> {  delay = backbone_delay; datarate = backbone_rate; } <--> server.ethg++;
}
