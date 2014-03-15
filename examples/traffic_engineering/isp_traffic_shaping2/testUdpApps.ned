//
// Copyright (C) 2012 Kyeong Soo (Joseph) Kim
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


package inet.examples.traffic_engineering.isp_traffic_shaping2;

import inet.networklayer.autorouting.FlatNetworkConfigurator;
import inet.nodes.access.NgnAccessModem;
import inet.nodes.access.NgnAccessNode;
import inet.nodes.access.NgnAccessSplitter;
import inet.nodes.inet.RouterWithEthernetInterface2;
import inet.nodes.inet.StandardHost3WithEthernetInterface2;
import inet.nodes.inet.StandardServerWithEthernetInterface2;
import inet.nodes.inet.UDPBurstSourceWithEthernetInterface2;
import inet.util.BurstMeter;


//
// Defines a test network for UDP applications.
//
// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
// @date   2010-09-12
//
network testUdpApps
{
    parameters:
        int numWcts = default(1);
        double uni_rate @unit(bps) = default(100Mbps);
        double uni_delay @unit(s) = default(0s);
        double distribution_rate @unit(bps) = default(100Mbps);
        double distribution_delay @unit(s) = default(150us);
        // double backbone_rate @unit(bps) = default(100Gbps);
        double backbone_rate @unit(bps) = default(100Mbps);
        double backbone_delay1 @unit(s) = default(2.0ms);
        double backbone_delay2 @unit(s) = default(2.7ms);
        @display("bgb=732,234");
    submodules:
        configurator: FlatNetworkConfigurator {
            parameters:
                @display("p=52,42");
        }
        wct_source[numWcts]: UDPBurstSourceWithEthernetInterface2 {
            parameters:
                destAddresses = "wct_sink[" + string(index) + "]";
                @display("p=641,120;i=device/server_l");
            gates:
                ethg[1];
        }
        router: RouterWithEthernetInterface2 {
            @display("p=507,119");
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
                ethg[numWcts+1];
        }
        onu[numWcts]: NgnAccessModem {
            parameters:
                @display("p=131,119");
                pvid = index+10;
            gates:
                ethg[2];
        }
        wct_sink[numWcts]: StandardHost3WithEthernetInterface2 {
            parameters:
                @display("i=device/laptop;p=46,119");
            gates:
                ethg[1];
        }
        burstMeter1: BurstMeter {
            @display("i=block/timer_vs;p=325,179;is=vs");
        }
        burstMeter2: BurstMeter {
            @display("i=block/timer_vs;p=449,179;is=vs");
        }
    connections:
        for i=0..numWcts-1 {
            wct_sink[i].ethg[0] <--> {  datarate = uni_rate; delay = uni_delay; } <--> onu[i].ethg[0];
            onu[i].ethg[1] <--> {  datarate = distribution_rate; delay = distribution_delay; } <--> accessSwitch.ethg[i];
        }
        accessSwitch.ethg$o[numWcts] --> {  datarate = distribution_rate; delay = distribution_delay; } --> olt.ethg$i[0];
        accessSwitch.ethg$i[numWcts] <-- {  datarate = distribution_rate; delay = 0.5*distribution_delay; } <-- burstMeter1.out;
        burstMeter1.in <-- {  datarate = distribution_rate; delay = 0.5*distribution_delay; } <-- olt.ethg$o[0];
        olt.ethg$o[1] --> {  datarate = backbone_rate; delay = backbone_delay1; } --> router.ethg$i++;
        olt.ethg$i[1] <-- {  datarate = backbone_rate; delay = 0.5*backbone_delay1; } <-- burstMeter2.out;
        burstMeter2.in <-- {  datarate = backbone_rate; delay = 0.5*backbone_delay1; } <-- router.ethg$o++;
        for i=0..numWcts-1 {
            router.ethg++ <--> {  datarate = backbone_rate; delay = backbone_delay2; } <--> wct_source[i].ethg[0];
        }
}
