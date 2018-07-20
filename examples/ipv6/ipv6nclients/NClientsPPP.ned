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

import inet.networklayer.configurator.ipv6.Ipv6FlatNetworkConfigurator;
import inet.node.ipv6.Router6;
import inet.node.ipv6.StandardHost6;
import ned.DatarateChannel;


network NClientsPPP
{
    parameters:
        int n;
    types:
        channel fiberline extends DatarateChannel
        {
            delay = 1us;
            datarate = 512Mbps;
        }
    submodules:
        configurator: Ipv6FlatNetworkConfigurator {
            @display("p=100,100;is=s");
        }
        r1: Router6 {
            @display("p=400,100");
        }
        r2: Router6 {
            @display("p=550,100");
        }
        r3: Router6 {
            @display("p=700,100");
        }
        cli[n]: StandardHost6 {
            @display("p=250,100,col,100");
        }
        srv: StandardHost6 {
            @display("p=850,100");
        }
    connections:
        for i=0..n-1 {
            cli[i].pppg++ <--> fiberline <--> r1.pppg++;
        }
        r1.pppg++ <--> fiberline <--> r2.pppg++;
        r2.pppg++ <--> fiberline <--> r3.pppg++;
        r3.pppg++ <--> fiberline <--> srv.pppg++;
}

