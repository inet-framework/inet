//
// Copyright (C) 2013 Kyeong Soo (Joseph) Kim
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


package inet.examples.traffic_engineering.isp_traffic_shaping2;

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
// Defines an abstract model of next-generation network (NGN) shared access architecture
// based on VLAN-capable Ethernet switches with token bucket filter (TBF) as traffic shaper.
//
// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
// @date   2013-02-20
//
network NgnAccessUnbalanced
{
    parameters:
        int N1 = default(1);	// number of ONUs for group 1 (subscribers)
        int N2 = default(1);	// number of ONUs for group 2 (subscribers)
        int n1 = default(1);	// number of hosts (users) per ONU for group 1 (subscriber)
        int n2 = default(1);	// number of hosts (users) per ONU for group 2 (subscriber)
        double uni_rate @unit(bps) = default(100Mbps);	// transmission rate of ONU Ethernet UNIs
        double uni_delay @unit(s) = default(0s);	// no delay at UNIs
        double distribution_rate @unit(bps) = default(10Gbps);	// transmission rate of distribution fiber (between Onu and OLT)
        double distribution_delay @unit(s) = default(150us);	// default delay of 300 us corresponds to 30 km fiber;
        // double feeder_rate @unit(bps) = default(10Gbps);	// transmission rate of feeder fiber (between Olt and LambdaSplitter)
        // double feeder_delay @unit(s) = default(150us);	// default delay of 150 us corresponds to 30 km fiber
        double backbone_rate @unit(bps) = default(100Gbps);	// transmission rate of backbone link (between Olt and server)
        // double backbone_delay @unit(s) = default(4.7ms);	// default delay of 4.7 ms gives total RTT of 10 ms including delays in access
        double backbone_delay1 @unit(s) = default(2.0ms);	// together with backbone_delay2 (i.e. 4.7ms), it gives total RTT of 10 ms including delays in access
        double backbone_delay2 @unit(s) = default(2.7ms);
        @display("bgb=684,233");
    submodules:
        configurator: FlatNetworkConfigurator {
            parameters:
                @display("p=52,42");
        }
        server: StandardServerWithEthernetInterface2 {
            parameters:
                @display("p=631,136;i=device/server_l");
            gates:
                ethg[1];
        }
        olt: NgnAccessNode {
            parameters:
                @display("p=387,136");
            gates:
                ethg[2];
        }
        accessSwitch: NgnAccessSplitter {
            parameters:
                @display("p=268,136");
            gates:
                ethg[N1+N2+1];
        }
        onu1[N1]: NgnAccessModem {
            parameters:
                @display("p=130,106");
                pvid = index+10;
            gates:
                ethg[n1+1];
        }
        onu2[N2]: NgnAccessModem {
            parameters:
                @display("p=130,166");
                pvid = index+10+N1;
            gates:
                ethg[n2+1];
        }
        host1[N1*n1]: StandardHost3WithEthernetInterface2 {
            parameters:
                @display("i=device/laptop;p=45,106");
            gates:
                ethg[1];
        }
        host2[N2*n2]: StandardHost3WithEthernetInterface2 {
            parameters:
                @display("i=device/laptop;p=45,166");
            gates:
                ethg[1];
        }
        router: RouterWithEthernetInterface2 {
            @display("p=506,137");
        }
    connections:
        for i=0..N1-1, for j=0..n1-1 {
            host1[i*n1+j].ethg[0] <--> {  delay = uni_delay; datarate = uni_rate; } <--> onu1[i].ethg[j];
        }
        for i=0..N2-1, for j=0..n2-1 {
            host2[i*n2+j].ethg[0] <--> {  delay = uni_delay; datarate = uni_rate; } <--> onu2[i].ethg[j];
        }
        for i=0..N1-1 {
            onu1[i].ethg[n1] <--> {  delay = distribution_delay; datarate = distribution_rate; } <--> accessSwitch.ethg[i];
        }
        for i=0..N2-1 {
            onu2[i].ethg[n2] <--> {  delay = distribution_delay; datarate = distribution_rate; } <--> accessSwitch.ethg[i+N1];
        }
        accessSwitch.ethg[N1+N2] <--> {  delay = distribution_delay; datarate = distribution_rate; } <--> olt.ethg[0];
        olt.ethg[1] <--> {  delay = backbone_delay1; datarate = backbone_rate; } <--> router.ethg++;
        router.ethg++ <--> {  delay = backbone_delay2; datarate = backbone_rate; } <--> server.ethg++;
}
