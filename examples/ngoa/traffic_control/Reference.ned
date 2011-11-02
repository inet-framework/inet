//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2011 Kyeong Soo (Joseph) Kim
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


package inet.examples.ngoa.traffic_control;

import inet.networklayer.autorouting.FlatNetworkConfigurator;
import inet.nodes.ethernet.EtherSwitch2WithOutputQueue;
//import inet.nodes.ethernet.EtherSwitch3WithPPP2Interface;
import inet.nodes.inet.StandardHost3WithEthernetInterface2;
import inet.nodes.inet.StandardServerWithEthernetInterface2;


//
// Defines a reference (dedicated) access network architecture for a
// study of next-generation optical access (NGOA) architectures.
//
// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
// @date   2011-10-17
//
network Reference
{
    parameters:
        int N = default(1);	// number of ONUs (subscribers)
        int n = default(1);	// number of hosts (users) per ONU (subscriber)
        double uni_rate @unit(bps) = default(10Gbps);	// transmission rate of ONU Ethernet UNIs
        double uni_delay @unit(s) = default(0s);	// no delay at UNIs
        double distribution_rate @unit(bps) = default(10Gbps);	// transmission rate of distribution fiber (between Onu and OLT)
        double distribution_delay @unit(s) = default(300us);	// default delay of 300 us corresponds to 60 km fiber;
        														// note that there is no feeder fiber in dedicated access unlike PONs.
        double backbone_rate @unit(bps) = default(100Gbps);	// transmission rate of backbone link (between Olt and server)
        double backbone_delay @unit(s) = default(4.7ms);	// default delay of 4.7 ms gives total RTT of 10 ms including delays in access
        @display("bgb=519,184");
    submodules:
        configurator: FlatNetworkConfigurator {
            parameters:
                @display("p=52,42");
        }
        server: StandardServerWithEthernetInterface2 {
            parameters:
                @display("p=464,119;i=device/server_l");
			gates:
				ethg[1];
        }
        olt: EtherSwitch2WithOutputQueue {
//        olt: EtherSwitch3WithPPP2Interface {
            parameters:
                @display("p=300,119");
			gates:
				ethg[N+1];
        }
        onu[N]: EtherSwitch2WithOutputQueue {
            parameters:
                @display("p=131,119");
			gates:
			    ethg[n+1];
        }
        host[N*n]: StandardHost3WithEthernetInterface2 {
            parameters:
                @display("i=device/laptop;p=46,119");
			gates:
				ethg[1];
        }
    connections:
        for i=0..N-1, for j=0..n-1 {
			host[i*n+j].ethg[0] <--> {  delay = uni_delay; datarate = uni_rate; } <--> onu[i].ethg[j];
		}
		for i=0..N-1 {
			onu[i].ethg[n] <--> {  delay = distribution_delay; datarate = distribution_rate; } <--> olt.ethg[i];
        }
        olt.ethg[N] <--> {  delay = backbone_delay; datarate = backbone_rate; } <--> server.ethg[0];
}
