//
// Copyright (C) 2003 CTIE, Monash University
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



package inet.examples.ethernet.test;

import ned.DatarateChannel;
import inet.nodes.ethernet.EtherHost;


//
// Sample Ethernet LAN: two hosts directly connected to each other
// via twisted pair.
//
module TwoHosts
{
    parameters:
        double speed @unit(bps) = default(100Mbps);
        bool isduplex = default(true);
    types:
        channel C extends DatarateChannel
        {
            delay = 0s;
            datarate = speed;
        }
    submodules:
        hostA: EtherHost {
            parameters:
                @display("p=70,70");
        }
        hostB: EtherHost {
            parameters:
                @display("p=190,70");
        }
    connections:
        hostA.ethg <--> C <--> hostB.ethg;
}

network SpeedTest
{
    submodules:
        Hosts1000F: TwoHosts {
            parameters:
                speed = 1000Mbps;
                isduplex = true;
                @display("p=70,70");
        }
        Hosts100F: TwoHosts {
            parameters:
                speed = 100Mbps;
                isduplex = true;
                @display("p=70,140");
        }
        Hosts100H: TwoHosts {
            parameters:
                speed = 100Mbps;
                isduplex = false;
                @display("p=190,140");
        }
        Hosts10F: TwoHosts {
            parameters:
                speed = 10Mbps;
                isduplex = true;
                @display("p=70,210");
        }
        Hosts10H: TwoHosts {
            parameters:
                speed = 10Mbps;
                isduplex = false;
                @display("p=190,210");
        }
}
