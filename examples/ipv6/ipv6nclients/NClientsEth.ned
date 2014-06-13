//
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


package inet.examples.ipv6.ipv6nclients;

import inet.networklayer.configurator.ipv6.FlatNetworkConfigurator6;
import inet.node.ipv6.Router6;
import inet.node.ipv6.StandardHost6;
import ned.DatarateChannel;


network NClientsEth
{
    parameters:
        int n;
        @display("bgb=569,296");
    types:
        channel fiberline extends DatarateChannel
        {
            delay = 1us;
            datarate = 1Gbps;
        }
        channel ethernetline extends DatarateChannel
        {
            delay = 0.1us;
            datarate = 100Mbps;
        }
    submodules:
        configurator: FlatNetworkConfigurator6 {
            @display("p=73,70");
        }
        r1: Router6 {
            @display("p=245,173");
        }
        r2: Router6 {
            @display("p=338,121");
        }
        r3: Router6 {
            @display("p=430,75");
        }
        cli[n]: StandardHost6 {
            @display("p=56,236");
        }
        srv: StandardHost6 {
            @display("p=524,33");
        }
    connections:
        for i=0..n-1 {
            cli[i].ethg++ <--> ethernetline <--> r1.ethg++;
        }
        r1.ethg++ <--> ethernetline <--> r2.ethg++;
        r2.ethg++ <--> ethernetline <--> r3.ethg++;
        r3.ethg++ <--> ethernetline <--> srv.ethg++;

        //XXX
        // Mixing ethernet and ppp currently doesn't work, due to a bug in
        // FlatNetworkConfigurator6::determineGateIndex.
        //
        // r1.pppg++ <--> fiberline <--> r2.pppg++;
        // r2.pppg++ <--> fiberline <--> r3.pppg++;
        // r3.pppg++ <--> fiberline <--> srv.pppg++;
        //
}

