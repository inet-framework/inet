//
// Copyright (C) 2008 Irene Ruengeler
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

package inet.examples.sctp.multihomed;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import ned.DatarateChannel;


network multi
{
    parameters:
        double testTimeout @unit(s) = default(0s);
        bool testing = default(false);
    types:
        channel ppp1line extends DatarateChannel
        {
            delay = 10ms;
            datarate = 1 Mbps;
        }
        channel ppp2line extends DatarateChannel
        {
            delay = 0ms;
            datarate = 1 Gbps;
        }
    submodules:
        cli1: StandardHost {
            parameters:
                forwarding = false;
                @display("p=61,142;i=device/laptop");
            gates:
                pppg[1];
        }
        srv1: StandardHost {
            parameters:
                forwarding = false;
                @display("p=426,135;i=device/server2");
            gates:
                pppg[1];

        }
        router1: Router {
            parameters:
                @display("p=188,77;i=abstract/router");
            gates:
                pppg[2];
        }
        router2: Router {
            parameters:
                @display("p=323,75;i=abstract/router");
            gates:
                pppg[2];
        }
        router3: Router {
            parameters:
                @display("p=188,192;i=abstract/router");
            gates:
                pppg[2];
        }
        router4: Router {
            parameters:
                @display("p=328,188;i=abstract/router");
            gates:
                pppg[2];
        }
        configurator: IPv4NetworkConfigurator {
            addStaticRoutes = false;
            config = xml("<config>"+
                    "<interface hosts='cli1' towards='router1' address='10.1.1.1' netmask='255.255.255.0' />"+
                    "<interface hosts='cli1' towards='router3' address='10.2.1.1' netmask='255.255.255.0' />"+
                    "<interface hosts='srv1' towards='router2' address='10.1.3.1' netmask='255.255.255.0' />"+
                    "<interface hosts='srv1' towards='router4' address='10.2.3.1' netmask='255.255.255.0' />"+
                    "<interface hosts='router1' towards='cli1' address='10.1.1.254' netmask='255.255.255.0' />"+
                    "<interface hosts='router1' towards='router2' address='10.1.2.254' netmask='255.255.255.0' />"+
                    "<interface hosts='router2' towards='router1' address='10.1.2.253' netmask='255.255.255.0' />"+
                    "<interface hosts='router2' towards='srv1' address='10.1.3.254' netmask='255.255.255.0' />"+
                    "<interface hosts='router3' towards='cli1' address='10.2.1.254' netmask='255.255.255.0' />"+
                    "<interface hosts='router3' towards='router4' address='10.2.2.254' netmask='255.255.255.0' />"+
                    "<interface hosts='router4' towards='router3' address='10.2.2.253' netmask='255.255.255.0' />"+
                    "<interface hosts='router4' towards='srv1' address='10.2.3.254' netmask='255.255.255.0' />"+
                    "<route hosts='cli1' destination='10.1.0.0' netmask='/16' gateway='router1' />"+
                    "<route hosts='cli1' destination='10.2.0.0' netmask='/16' gateway='router3' />"+
                    "<route hosts='srv1' destination='10.1.0.0' netmask='/16' gateway='router2' />"+
                    "<route hosts='srv1' destination='10.2.0.0' netmask='/16' gateway='router4' />"+
                    "<route hosts='router1' destination='10.1.1.1' netmask='/32' gateway='10.1.1.1' />"+
                    "<route hosts='router1' destination='*' gateway='10.1.2.253' />"+
                    "<route hosts='router2' destination='10.1.3.1' netmask='/32' gateway='10.1.3.1' />"+
                    "<route hosts='router2' destination='*' gateway='10.1.2.254' />"+
                    "<route hosts='router3' destination='10.2.1.1' netmask='/32' gateway='10.2.1.1' />"+
                    "<route hosts='router3' destination='*' gateway='10.2.2.253' />"+
                    "<route hosts='router4' destination='10.2.3.1' netmask='/32' gateway='10.2.3.1' />"+
                    "<route hosts='router4' destination='*' gateway='10.2.2.254' />"+
                    "</config>");
        }
    connections:
        cli1.pppg++ <--> ppp2line <--> router1.pppg++;
        router1.pppg++ <--> ppp1line <--> router2.pppg++;
        router2.pppg++ <--> ppp2line <--> srv1.pppg++;
        cli1.pppg++ <--> ppp2line <--> router3.pppg++;
        router3.pppg++ <--> ppp1line <--> router4.pppg++;
        router4.pppg++ <--> ppp2line <--> srv1.pppg++;
}

