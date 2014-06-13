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

import inet.common.lifecycle.LifecycleController;
import inet.common.misc.ThruputMeteringChannel;
import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.ethernet.EtherSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;


network NClients
{
    parameters:
        int n;
        @display("bgb=593,356");
    types:
        channel fiberline extends ThruputMeteringChannel
        {
            delay = 1us;
            datarate = 512Mbps;
            thruputDisplayFormat = "u";
        }
        channel ethernetline extends ThruputMeteringChannel
        {
            delay = 0.1us;
            datarate = 10Mbps;
            thruputDisplayFormat = "u";
        }
    submodules:
        r1: Router {
            @display("p=232,281");
        }
        r2: Router {
            @display("p=324,219");
        }
        r3: Router {
            @display("p=433,250");
        }
        s: EtherSwitch {
            @display("p=153,221");
        }
        cli[n]: StandardHost {
            @display("i=device/laptop;p=51,269,r");
        }
        srv: StandardHost {
            @display("p=544,194;i=device/server_l");
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=75,47");
        }
        scenarioManager: ScenarioManager {
            @display("p=75,101");
            script = xml("<script>\n" +
                "<at t='3s'><tell module='lifecycleController' operation='NodeShutdownOperation' target='r2'/></at>\n" +
                "<at t='6s'><tell module='lifecycleController' operation='NodeStartOperation' target='r2'/></at>\n" +
                "<at t='9s'><tell module='lifecycleController' operation='NodeShutdownOperation' target='r2'/></at>\n" +
                "<at t='12s'><tell module='lifecycleController' operation='NodeStartOperation' target='r2'/></at>\n" +
                "<at t='15s'><tell module='lifecycleController' operation='NodeShutdownOperation' target='cli[0]'/></at>\n" +
                "<at t='18s'><tell module='lifecycleController' operation='NodeStartOperation' target='cli[0]'/></at>\n" +
                "</script>");
        }
        lifecycleController: LifecycleController {
            @display("p=75,155");
        }
    connections:
        for i=0..n-1 {
            cli[i].ethg++ <--> ethernetline <--> s.ethg++;
        }
        s.ethg++ <--> ethernetline <--> r1.ethg++;
        r1.pppg++ <--> ethernetline <--> r2.pppg++;
        r2.pppg++ <--> ethernetline <--> r3.pppg++;
        r3.pppg++ <--> ethernetline <--> srv.pppg++;
}

