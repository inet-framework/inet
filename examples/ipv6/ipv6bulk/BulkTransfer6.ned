//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
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





package inet.examples.ipv6.ipv6bulk;

import inet.networklayer.configurator.ipv6.Ipv6FlatNetworkConfigurator;
import inet.node.ipv6.Router6;
import inet.node.ipv6.StandardHost6;
import ned.TransmissionChannel;


network BulkTransfer6
{
    parameters:
        *.eth[*].bitrate = 10 Mbps;
    types:
        channel C extends TransmissionChannel
        {
            delay = 0.1us;
        }
    submodules:
        client1: StandardHost6 {
            parameters:
                @display("p=250,100;i=device/pc3");
        }
        client2: StandardHost6 {
            parameters:
                @display("p=550,100;i=device/pc3");
        }
        client3: StandardHost6 {
            parameters:
                @display("p=550,300;i=device/pc3");
        }
        server: StandardHost6 {
            parameters:
                @display("p=250,300;i=device/pc2");
        }
        router: Router6 {
            parameters:
                @display("p=400,200");
        }
        configurator: Ipv6FlatNetworkConfigurator {
            parameters:
                @display("p=100,100;is=s");
        }
    connections:
        client1.ethg++ <--> C <--> router.ethg++;
        client2.ethg++ <--> C <--> router.ethg++;
        client3.ethg++ <--> C <--> router.ethg++;
        client1.ethg++ <--> C <--> server.ethg++;
        server.ethg++ <--> C <--> router.ethg++;
}

