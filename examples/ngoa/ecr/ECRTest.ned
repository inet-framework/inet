//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2009 Kyeong Soo (Joseph) Kim
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


package inet.examples.ngoa.ecr;

import inet.examples.ethernet.lans.ethernetline;
import inet.networklayer.autorouting.FlatNetworkConfigurator;
import inet.nodes.inet.Router;
import inet.nodes.inet.StandardHost;
import inet.nodes.inet.StandardHost2;
import inet.world.ChannelInstaller;


//
// Implementation of equivalent circuit rate (ECR)-based quantitative
// analysis framework for next-generation optical access architectures.
//
// Initial results based on abstract architecture models were reported at
// <a href="http://www.omnet-workshop.org/2010/">OMNeT++ 2010 workshop</a>
// in March 2010 [1].
//
// References:
// -# Kyeong Soo (Joseph) Kim, "An equivalent circuit rate-based study
//  of next-generation optical access architectures," OMNeT++ 2010 workshop,
//  Malaga, Spain, Mar. 19, 2010.
//
// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
// @date   2010-03-25
//
network ECRTest
{
    parameters:
        int n;
        double backbone_rate @unit(bps) = default(1Tbps);
        double backbone_delay @unit(s) = default(5ms);	// RTT = 10ms
        double distribution_rate @unit(bps) = default(100Mbps);
        double feeder_rate @unit(bps) = default(100Mbps);
    submodules:
        channelInstaller: ChannelInstaller {
            parameters:
                channelClass = "ThruputMeteringChannel";
                channelAttrs = "format=u";
                @display("p=98,50");
        }
        configurator: FlatNetworkConfigurator {
            parameters:
                @display("p=185,50");
        }
        r1: Router {
            parameters:
                @display("p=185,194");
        }
        r2: Router {
            parameters:
                @display("p=304,194");
        }
        cli[n]: StandardHost2 {
            parameters:
                @display("i=device/laptop;p=67,194");
        }
        srv: StandardHost {
            parameters:
                @display("p=439,194;i=device/server_l");
        }
    connections:
        for i=0..n-1 {
//            cli[i].pppg++ <--> fiberline_100M <--> r1.pppg++;
			cli[i].pppg++ <--> {datarate=distribution_rate;} <--> r1.pppg++;
        }
//        r1.pppg++ <--> fiberline_100M <--> r2.pppg++;
        r1.pppg++ <--> {datarate=feeder_rate;} <--> r2.pppg++;
//        r2.pppg++ <--> fiberline_1T_10ms_rtt <--> srv.pppg++;
		r2.pppg++ <--> {delay=backbone_delay; datarate=backbone_rate;} <--> srv.pppg++;
}
