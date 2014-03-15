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


package inet.examples.test.perf_monitor_test;

import inet.networklayer.autorouting.FlatNetworkConfigurator;
import inet.nodes.access.NgnAccessModem;
import inet.nodes.access.NgnAccessNode;
import inet.nodes.access.NgnAccessSplitter;
import inet.nodes.inet.RouterWithEthernetInterface2;
import inet.nodes.inet.StandardHost3WithEthernetInterface2WithDLDelayMeter;
import inet.nodes.inet.StandardServerWithEthernetInterface2;
import inet.util.BurstMeter;


//
// Defines an abstract model of next-generation network (NGN) shared access architecture
// based on VLAN-capable Ethernet switches with token bucket filter (TBF) as traffic shaper
// for testing of various performance monitors.
//
// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
// @date   2012-03-04
//
network PerformanceMonitorTest
{
    parameters:
        double uni_rate @unit(bps) = default(100Mbps);	// transmission rate of ONU Ethernet UNIs
        double uni_delay @unit(s) = default(0s);	// no delay at UNIs
        double distribution_rate @unit(bps) = default(10Gbps);	// transmission rate of distribution fiber (between Onu and OLT)
        double distribution_delay @unit(s) = default(150us);	// default delay of 150 us corresponds to 30 km fiber (60 km is the max. logical reach of GPON)
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
        olt: NgnAccessNode {
            parameters:
                @display("p=388,119");
            gates:
                ethg[2];
        }
        accessSwitch: NgnAccessSplitter {
            parameters:
                @display("p=269,119");
            gates:
                ethg[2];
        }
        onu: NgnAccessModem {
            parameters:
                @display("p=131,119");
                pvid = 10;
            gates:
                ethg[2];
        }
        host: StandardHost3WithEthernetInterface2WithDLDelayMeter {
            parameters:
                @display("i=device/laptop;p=46,119");
            gates:
                ethg[1];
        }
        router: RouterWithEthernetInterface2 {
            @display("p=507,120");
        }
        burstMeter: BurstMeter {
            @display("i=block/timer_vs;p=572,167;is=vs");
        }
    connections:
        host.ethg[0] <--> {  delay = uni_delay; datarate = uni_rate; } <--> onu.ethg[0];
        onu.ethg[1] <--> {  delay = distribution_delay; datarate = distribution_rate; } <--> accessSwitch.ethg[0];
        accessSwitch.ethg[1] <--> {  delay = distribution_delay; datarate = distribution_rate; } <--> olt.ethg[0];
        olt.ethg[1] <--> {  delay = backbone_delay1; datarate = backbone_rate; } <--> router.ethg++;
        router.ethg$o++ --> {  delay = backbone_delay2; datarate = backbone_rate; } --> server.ethg$i++;
        router.ethg$i++ <-- {  delay = 0.5*backbone_delay2; datarate = backbone_rate; } <-- burstMeter.out;
        burstMeter.in <-- {  delay = 0.5*backbone_delay2; datarate = backbone_rate; } <-- server.ethg$o++;
}
