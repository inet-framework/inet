//
// Copyright (C) 2011 OpenSim Ltd
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


package inet.examples.pcaptrafgen;

import inet.networklayer.autorouting.ipv4.FlatNetworkConfigurator;
import inet.nodes.inet.Router;
import inet.nodes.inet.StandardHost;
import ned.DatarateChannel;


network PcapTrafGenTest
{
    parameters:
        int n;
    types:
        channel fiberline extends DatarateChannel
        {
            delay = 1us;
            datarate = 512Mbps;
        }
        channel ethernetline extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
        }
    submodules:
        configurator: FlatNetworkConfigurator {
            parameters:
                networkAddress = "192.168.1.1";
                @display("p=185,50");
        }
        cli[n]: StandardHost {
            parameters:
                @display("i=device/laptop");
        }
        srv: StandardHost {
            parameters:
                @display("p=544,194;i=device/server_l");
        }
        r1: Router {
            parameters:
                @display("p=186,250");
        }
        r2: Router {
            parameters:
                @display("p=304,220");
        }
        r3: Router {
            parameters:
                @display("p=433,250");
        }
        generator: StandardHost {
            parameters:
                networkLayer.trafficGeneratorType = "PcapTrafficGenerator";
                IPForward = true;
                @display("p=186,152");
        }
    connections:
        for i=0..n-1 {
            cli[i].pppg++ <--> ethernetline <--> r1.pppg++;
        }
        generator.pppg++ <--> ethernetline <--> r2.pppg++;
        r1.pppg++ <--> ethernetline <--> r2.pppg++;
        r2.pppg++ <--> ethernetline <--> r3.pppg++;
        r3.pppg++ <--> ethernetline <--> srv.pppg++;
}

