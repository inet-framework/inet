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


package inet.examples.traffic_engineering.isp_traffic_control;

import inet.networklayer.autorouting.FlatNetworkConfigurator;
import inet.nodes.access.NgnAccessModem;
import inet.nodes.access.NgnAccessNode2;
import inet.nodes.access.NgnAccessSplitter;
import inet.nodes.inet.RouterWithEthernetInterface2;
import inet.nodes.inet.StandardHost3WithEthernetInterface2;
import inet.nodes.inet.StandardHost3WithEthernetInterface2WithDLThruputMeter;
import inet.nodes.inet.StandardServerWithEthernetInterface2;


//
// Defines an abstract model of next-generation network (NGN) shared access architecture
// based on VLAN-capable Ethernet switches to test various ISP traffic control schemes.
//
// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
// @date   2013-06-18
//
network NgnAccess
{
    parameters:
        int N = default(1);	// number of ONUs (subscribers)
        int n = default(1);	// number of hosts (users) per ONU (subscriber)
        double uni_rate @unit(bps) = default(100Mbps);	// transmission rate of ONU Ethernet UNIs
        double uni_delay @unit(s) = default(0s);	// no delay at UNIs
        double distribution_rate @unit(bps) = default(10Gbps);	// transmission rate of distribution fiber (between Onu and OLT)
        double distribution_delay @unit(s) = default(150us);	// default delay of 300 us corresponds to 30 km fiber;
        double backbone_rate @unit(bps) = default(100Gbps);	// transmission rate of backbone link (between Olt and server)
        double backbone_delay @unit(s) = default(4.7ms);	// default delay of 4.7 ms gives total RTT of 10 ms including delays in access
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
                @display("p=632,119;i=device/server_l");
            gates:
                ethg[1];
        }
        olt: NgnAccessNode2 {
            parameters:
                @display("p=388,119");
            gates:
                ethg[2];
        }
        accessSwitch: NgnAccessSplitter {
            parameters:
                @display("p=269,119");
            gates:
                ethg[N+1];
        }
        onu[N]: NgnAccessModem {
            parameters:
                @display("p=131,119");
                pvid = index+10;
            gates:
                ethg[n+1];
        }
        host[N*n]: StandardHost3WithEthernetInterface2 {
            parameters:
                @display("i=device/laptop;p=46,119");
            gates:
                ethg[1];
        }
        router: RouterWithEthernetInterface2 {
            @display("p=507,120");
        }
    connections:
        for i=0..N-1, for j=0..n-1 {
            host[i*n+j].ethg[0] <--> {  delay = uni_delay; datarate = uni_rate; } <--> onu[i].ethg[j];
        }
        for i=0..N-1 {
            onu[i].ethg[n] <--> {  delay = distribution_delay; datarate = distribution_rate; } <--> accessSwitch.ethg[i];
        }
        accessSwitch.ethg[N] <--> {  delay = distribution_delay; datarate = distribution_rate; } <--> olt.ethg[0];
        olt.ethg[1] <--> {  delay = backbone_delay1; datarate = backbone_rate; } <--> router.ethg++;
        router.ethg++ <--> {  delay = backbone_delay2; datarate = backbone_rate; } <--> server.ethg++;
}


//
// 'NgnAccess' network with hosts with DLThruputMeter.
//
// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
// @date   2013-07-15
//
network NgnAccessWithDLThruputMeter
{
    parameters:
        int N = default(1);	// number of ONUs (subscribers)
        int n = default(1);	// number of hosts (users) per ONU (subscriber)
        double uni_rate @unit(bps) = default(100Mbps);	// transmission rate of ONU Ethernet UNIs
        double uni_delay @unit(s) = default(0s);	// no delay at UNIs
        double distribution_rate @unit(bps) = default(10Gbps);	// transmission rate of distribution fiber (between Onu and OLT)
        double distribution_delay @unit(s) = default(150us);	// default delay of 300 us corresponds to 30 km fiber;
        double backbone_rate @unit(bps) = default(100Gbps);	// transmission rate of backbone link (between Olt and server)
        double backbone_delay @unit(s) = default(4.7ms);	// default delay of 4.7 ms gives total RTT of 10 ms including delays in access
        double backbone_delay1 @unit(s) = default(2.0ms);	// together with backbone_delay2 (i.e. 4.7ms), it gives total RTT of 10 ms including delays in access
        double backbone_delay2 @unit(s) = default(2.7ms);
        @display("bgb=709,233");
    submodules:
        configurator: FlatNetworkConfigurator {
            parameters:
                @display("p=52,42");
        }
        server: StandardServerWithEthernetInterface2 {
            parameters:
                @display("p=651,119;i=device/server_l");
            gates:
                ethg[1];
        }
        olt: NgnAccessNode2 {
            parameters:
                @display("p=388,119");
            gates:
                ethg[2];
        }
        accessSwitch: NgnAccessSplitter {
            parameters:
                @display("p=269,119");
            gates:
                ethg[N+1];
        }
        onu[N]: NgnAccessModem {
            parameters:
                @display("p=131,119");
                pvid = index+10;
            gates:
                ethg[n+1];
        }
        host[N*n]: StandardHost3WithEthernetInterface2WithDLThruputMeter {
            parameters:
                @display("i=device/laptop;p=46,119");
            gates:
                ethg[1];
        }
        router: RouterWithEthernetInterface2 {
            @display("p=507,120");
        }
    connections:
        for i=0..N-1, for j=0..n-1 {
            host[i*n+j].ethg[0] <--> {  delay = uni_delay; datarate = uni_rate; } <--> onu[i].ethg[j];
        }
        for i=0..N-1 {
            onu[i].ethg[n] <--> {  delay = distribution_delay; datarate = distribution_rate; } <--> accessSwitch.ethg[i];
        }
        accessSwitch.ethg[N] <--> {  delay = distribution_delay; datarate = distribution_rate; } <--> olt.ethg[0];
        olt.ethg[1] <--> {  delay = backbone_delay1; datarate = backbone_rate; } <--> router.ethg++;
		router.ethg++ <--> {  delay = backbone_delay2; datarate = backbone_rate; } <--> server.ethg++;
}
