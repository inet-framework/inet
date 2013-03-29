//
// Copyright (C) 2012 Kyeong Soo (Joseph) Kim
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


package inet.examples.scfr.jts;

import inet.networklayer.autorouting.FlatNetworkConfigurator;
import inet.nodes.access.NgnAccessModem;
import inet.nodes.access.NgnAccessNode;
import inet.nodes.access.NgnAccessSplitter;
import inet.nodes.inet.RouterWithEthernetInterface2;
import inet.nodes.inet.StandardHost3WithEthernetInterface2;
import inet.nodes.inet.StandardHost3WithEthernetInterface2WithDLDelayMeter;
import inet.nodes.inet.StandardServerWithEthernetInterface2;
import inet.util.BurstMeter;


//
// Defines a network for simulation of xKALP source clock frequency recovery (SCFR) algorithm.
//
// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
// @date   2012-12-24
//
network JTSTest
{
    parameters:
        int N = default(1);	// number of ONUs (subscribers)
        int n = default(1);	// number of hosts (users) per ONU (subscriber)
        double uni_rate @unit(bps) = default(100Mbps);	// transmission rate of ONU Ethernet UNIs
        double uni_delay @unit(s) = default(0s);	// no delay at UNIs
        double distribution_rate @unit(bps) = default(10Gbps);	// transmission rate of distribution fiber (between Onu and OLT)
        double distribution_delay @unit(s) = default(150us);	// default delay of 300 us corresponds to 30 km fiber;
        //double feeder_rate @unit(bps) = default(10Gbps);	// transmission rate of feeder fiber (between Olt and LambdaSplitter)
        //double feeder_delay @unit(s) = default(150us);	// default delay of 150 us corresponds to 30 km fiber
        double backbone_rate @unit(bps) = default(100Gbps);	// transmission rate of backbone link (between Olt and server)
        //double backbone_delay @unit(s) = default(4.7ms);	// default delay of 4.7 ms gives total RTT of 10 ms including delays in access
        double backbone_delay1 @unit(s) = default(1.0ms);	// together with backbone_delay2/3/4 (i.e. 4.7ms), it gives total RTT of 10 ms including delays in access
        double backbone_delay2 @unit(s) = default(1.0ms);
        double backbone_delay3 @unit(s) = default(1.7ms);
        double backbone_delay4 @unit(s) = default(1.0ms);
        @display("bgb=830,317");
    submodules:
        configurator: FlatNetworkConfigurator {
            parameters:
                @display("p=52,42");
        }
        server: StandardServerWithEthernetInterface2 {
            parameters:
                @display("p=772,170;i=device/server_l");
            gates:
                ethg[1];
        }
        olt: NgnAccessNode {
            parameters:
                @display("p=353,169");
            gates:
                ethg[2];
        }
        accessSwitch: NgnAccessSplitter {
            parameters:
                @display("p=246,169");
            gates:
                ethg[N+1];
        }
        onu[N]: NgnAccessModem {
            parameters:
                @display("p=128,169");
                pvid = index+10;
            gates:
                ethg[n+1];
        }
        host[N*n]: StandardHost3WithEthernetInterface2 {
            parameters:
                @display("i=device/laptop;p=43,169");
            gates:
                ethg[1];
        }
        router1: RouterWithEthernetInterface2 {
            @display("p=454,170");
        }
        router2: RouterWithEthernetInterface2 {
            @display("p=563,127");
        }
        router3: RouterWithEthernetInterface2 {
            @display("p=662,213");
        }
    connections:
        for i=0..N-1, for j=0..n-1 {
            host[i*n+j].ethg[0] <--> {  delay = uni_delay; datarate = uni_rate; } <--> onu[i].ethg[j];
        }
        for i=0..N-1 {
            onu[i].ethg[n] <--> {  delay = distribution_delay; datarate = distribution_rate; } <--> accessSwitch.ethg[i];
        }
        accessSwitch.ethg[N] <--> {  delay = distribution_delay; datarate = distribution_rate; } <--> olt.ethg[0];
        olt.ethg[1] <--> {  delay = backbone_delay1; datarate = backbone_rate; } <--> router1.ethg++;
        router1.ethg++ <--> {  delay = backbone_delay2; datarate = backbone_rate; } <--> router2.ethg++;
        router2.ethg++ <--> {  delay = backbone_delay3; datarate = backbone_rate; } <--> router3.ethg++;
        router3.ethg++ <--> {  delay = backbone_delay4; datarate = backbone_rate; } <--> server.ethg++;
}
