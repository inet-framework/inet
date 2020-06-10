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




package inet.examples.inet.kidsnw1;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import ned.DatarateChannel;


network KIDSNw1
{
    types:
        channel pppline extends DatarateChannel
        {
            delay = normal(0.00015s, 0.00005s);
            datarate = 10Mbps;
        }
        channel fiberrouterline extends DatarateChannel
        {
            delay = normal(0.000001s, 0.0000001s);
            datarate = 512Mbps;
        }
    submodules:
        host1: StandardHost {
            parameters:
                @display("p=100,400;i=device/pc2");
        }
        host2: StandardHost {
            parameters:
                @display("p=300,400;i=device/pc2");
        }
        host3: StandardHost {
            parameters:
                @display("p=500,400;i=device/pc2");
        }
        // First Hop Router
        fhrouter1: Router {
            parameters:
                @display("p=200,300");
        }
        fhrouter2: Router {
            parameters:
                @display("p=500,300");
        }
        // InteriorRouter
        irouter: Router {
            parameters:
                @display("p=200,200");
        }
        // Border Gateway Router
        bgrouter3: Router {
            parameters:
                @display("p=350,100");
        }
        bgrouter4: Router {
            parameters:
                @display("p=500,200");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,100;is=s");
        }
    connections:
        // host connections
        host1.pppg++ <--> pppline <--> fhrouter1.pppg++;
        host2.pppg++ <--> pppline <--> fhrouter1.pppg++;
        host3.pppg++ <--> pppline <--> fhrouter2.pppg++;

        // router connections
        fhrouter1.pppg++ <--> fiberrouterline <--> irouter.pppg++;
        irouter.pppg++ <--> fiberrouterline <--> bgrouter3.pppg++;
        bgrouter3.pppg++ <--> fiberrouterline <--> bgrouter4.pppg++;
        bgrouter4.pppg++ <--> fiberrouterline <--> fhrouter2.pppg++;
}
