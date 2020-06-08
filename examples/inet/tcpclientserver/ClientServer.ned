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

package inet.examples.inet.tcpclientserver;

import inet.applications.generic.ExtApp;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth1G;
import inet.node.inet.StandardHost;


network ClientServer
{
    parameters:
        double per = default(0);
    submodules:
        client1: StandardHost {
            parameters:
                @display("p=250,150;i=device/pc3");
        }
        server: StandardHost {
            parameters:
                @display("p=400,150;i=device/pc2");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,100;is=s");
        }
        extApp: ExtApp {
            @display("p=348,36");
        }
    connections:
        client1.ethg++ <--> Eth1G <--> server.ethg++;
}

