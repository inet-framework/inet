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




package inet.examples.inet.shutdownrestart;

import inet.common.misc.ThruputMeteringChannel;
import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.EtherSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import ned.TransmissionChannel;


network NClients
{
    parameters:
        int n;
        *.eth[*].bitrate = 10 Mbps;
    types:
        channel pppline extends ThruputMeteringChannel
        {
            delay = 0.1us;
            datarate = 10Mbps;
            thruputDisplayFormat = "u";
        }
        channel ethernetline extends TransmissionChannel
        {
            delay = 0.1us;
        }
    submodules:
        r1: Router {
            @display("p=500,150");
        }
        r2: Router {
            @display("p=650,150");
        }
        r3: Router {
            @display("p=800,150");
        }
        s: EtherSwitch {
            @display("p=350,150");
        }
        cli[n]: StandardHost {
            @display("i=device/laptop;p=250,100,col,100");
        }
        srv: StandardHost {
            @display("p=950,150;i=device/server");
        }
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,100;is=s");
        }
        scenarioManager: ScenarioManager {
            @display("p=100,200;is=s");
        }
    connections:
        for i=0..n-1 {
            cli[i].ethg++ <--> ethernetline <--> s.ethg++;
        }
        s.ethg++ <--> ethernetline <--> r1.ethg++;
        r1.pppg++ <--> pppline <--> r2.pppg++;
        r2.pppg++ <--> pppline <--> r3.pppg++;
        r3.pppg++ <--> pppline <--> srv.pppg++;
}

