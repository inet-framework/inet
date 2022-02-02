//
// Copyright (C) 2008 Irene Ruengeler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//

package inet.examples.sctp.multihomed;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
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
                @display("p=250,200;i=device/laptop");
            gates:
                pppg[1];
        }
        srv1: StandardHost {
            parameters:
                forwarding = false;
                @display("p=600,200;i=device/server2");
            gates:
                pppg[1];

        }
        router1: Router {
            parameters:
                @display("p=350,100;i=abstract/router");
            gates:
                pppg[2];
        }
        router2: Router {
            parameters:
                @display("p=500,100;i=abstract/router");
            gates:
                pppg[2];
        }
        router3: Router {
            parameters:
                @display("p=350,300;i=abstract/router");
            gates:
                pppg[2];
        }
        router4: Router {
            parameters:
                @display("p=500,300;i=abstract/router");
            gates:
                pppg[2];
        }
        configurator: Ipv4NetworkConfigurator {
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
            @display("p=100,100;is=s");
        }
    connections:
        cli1.pppg++ <--> ppp2line <--> router1.pppg++;
        router1.pppg++ <--> ppp1line <--> router2.pppg++;
        router2.pppg++ <--> ppp2line <--> srv1.pppg++;
        cli1.pppg++ <--> ppp2line <--> router3.pppg++;
        router3.pppg++ <--> ppp1line <--> router4.pppg++;
        router4.pppg++ <--> ppp2line <--> srv1.pppg++;
}

