//
// Copyright (C) 2011 Zoltan Bojthe
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

package inet.examples.inet.pcaprecorder;

import inet.common.misc.NetAnimTrace;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.inet.StandardHost;
import ned.DatarateChannel;


network PcapRecorderTest
{
    @display("bgb=306,193");
    types:
        channel C extends DatarateChannel
        {
            datarate = 10Mbps;
            delay = 0.1us;
        }
    submodules:
        client0: StandardHost {
            parameters:
                @display("p=164,36;i=device/pc3");
        }
        client1: StandardHost {
            parameters:
                @display("p=164,136;i=device/pc3");
        }
        server: StandardHost {
            parameters:
                @display("p=260,87;i=device/pc2");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=46,28");
        }
        netAnimTrace: NetAnimTrace {
            @display("p=46,105");
        }
    connections:
        client0.pppg++ <--> C <--> server.pppg++;
        client1.ethg++ <--> C <--> server.ethg++;
}

