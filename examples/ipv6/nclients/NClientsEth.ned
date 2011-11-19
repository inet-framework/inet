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


package inet.examples.ipv6.nclients;

import ned.DatarateChannel;
import inet.nodes.ipv6.StandardHost6;
import inet.nodes.ipv6.Router6;
import inet.networklayer.autorouting.ipv6.FlatNetworkConfigurator6;


network NClientsEth
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
            datarate = 10Mbps;
        }
    submodules:
        configurator: FlatNetworkConfigurator6;
        r1: Router6;
        r2: Router6;
        r3: Router6;
        cli[n]: StandardHost6;
        srv: StandardHost6;
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

