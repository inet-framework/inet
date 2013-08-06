//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2010-2011 Kyeong Soo (Joseph) Kim
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


package inet.examples.traffic_engineering.traffic_control;

import inet.networklayer.autorouting.FlatNetworkConfigurator;
import inet.nodes.inet.StandardHostWithEthernetInterface2;
import inet.nodes.inet.StandardHost3WithEthernetInterface2;
import inet.nodes.inet.StandardServerWithEthernetInterface2;
import inet.nodes.hybridpon.Olt;
import inet.nodes.hybridpon.Onu;
import inet.nodes.hybridpon.LambdaSplitter;


//
// Defines a hybrid TDM/WDM-PON architecture for a comparison
// study of next-generation optical access (NGOA) architectures.
//
// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
// @date   2011-03-14
//
network HybridPon
{
    parameters:
        int N = default(16);	// number of subscribers (ONUs)
        int n = default(1);	// number of hosts (users) per ONU (subscriber)
        double uni_rate @unit(bps) = default(10Gbps);	// transmission rate of ONU Ethernet UNIs
        double uni_delay @unit(s) = default(0s);	// no delay at UNIs
        double distribution_rate @unit(bps) = default(10Gbps);	// transmission rate of distribution fiber (between Onu and LambdaSplitter)
        double distribution_delay @unit(s) = default(150us);	// default delay of 150 us corresponds to 30 km fiber
        double feeder_rate @unit(bps) = default(10Gbps);	// transmission rate of feeder fiber (between Olt and LambdaSplitter)
        double feeder_delay @unit(s) = default(150us);	// default delay of 150 us corresponds to 30 km fiber
        double backbone_rate @unit(bps) = default(100Gbps);	// transmission rate of backbone link (between Olt and server)
        double backbone_delay @unit(s) = default(4.7ms);	// default delay of 4.7 ms gives total RTT of 10 ms including delays in feeder and distribution fiber
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
        olt: Olt {
            @display("p=355,119");
			gates:
				ethg[1];
				phyg[N];
        }
        awg: LambdaSplitter {
            parameters:
                @display("p=239,119");
            gates:
                demuxg[N];
                muxg[N];
        }
        onu[N]: Onu {
            @display("p=131,119");
			gates:
				ethg[n];            
        }
        host[N*n]: StandardHost3WithEthernetInterface2 {
            parameters:
                @display("i=device/laptop;p=46,119");
			gates:
				ethg[1];
        }
    connections:
        for i=0..N-1, for j=0..n-1 {
			host[i*n+j].ethg[0] <--> { delay = uni_delay; datarate = uni_rate; } <--> onu[i].ethg[j];
		}
		for i=0..N-1 {
		    onu[i].phyg <--> { delay = distribution_delay; datarate = distribution_rate; } <--> awg.demuxg[i];
            awg.muxg[i] <--> { delay = feeder_delay; datarate = feeder_rate; } <--> olt.phyg[i];
        }
        olt.ethg[0] <--> { delay = backbone_delay; datarate = backbone_rate; } <--> server.ethg[0];
}
