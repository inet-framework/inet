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


network NClientsPPP
{
    parameters:
        int n;
        @display("bgb=628,413");
    types:
        channel fiberline extends DatarateChannel
        {
            delay = 1us;
            datarate = 512Mbps;
        }
    submodules:
        configurator: FlatNetworkConfigurator6 {
            @display("p=63,72");
        }
        r1: Router6 {
            @display("p=219,245");
        }
        r2: Router6 {
            @display("p=336,178");
        }
        r3: Router6 {
            @display("p=458,107");
        }
        cli[n]: StandardHost6 {
            @display("p=63,334");
        }
        srv: StandardHost6 {
            @display("p=576,46");
        }
    connections:
        for i=0..n-1 {
            cli[i].pppg++ <--> fiberline <--> r1.pppg++;
        }
        r1.pppg++ <--> fiberline <--> r2.pppg++;
        r2.pppg++ <--> fiberline <--> r3.pppg++;
        r3.pppg++ <--> fiberline <--> srv.pppg++;
}

