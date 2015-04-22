//
// Copyright (C) 2012 Opensim Ltd.
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

package inet.examples.inet.configurator;

import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.ethernet.Eth1G;
import inet.node.inet.AdhocHost;
import inet.node.inet.Router;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;

network ComplexConfiguratorNetwork
{
    parameters:
        @display("bgb=1138,534");
    submodules:
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=50,50");
        }
        radioMediumA: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=50,150");
        }
        radioMediumB: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=50,250");
        }
        source: AdhocHost {
            @display("p=50,271;i=device/laptop");
        }
        wirelessA1: AdhocHost {
            @display("p=201,230");
        }
        wirelessA2: AdhocHost {
            @display("p=334,304");
        }
        wiredA1: Router {
            @display("p=107,458");
        }
        wiredA2: Router {
            @display("p=398,458");
        }
        shared1: AdhocHost {
            @display("p=481,271;i=device/laptop");
        }
        shared2: AdhocHost {
            @display("p=674,271;i=device/laptop");
        }
        wirelessB1: AdhocHost {
            @display("p=822,197");
        }
        wirelessB2: AdhocHost {
            @display("p=925,307");
        }
        wiredB1: Router {
            @display("p=737,82");
        }
        wiredB2: Router {
            @display("p=977,82");
        }
        destination: AdhocHost {
            @display("p=1069,271;i=device/laptop");
        }
    connections:
        source.ethg++ <--> Eth1G <--> wiredA1.ethg++;
        wiredA1.ethg++ <--> Eth1G <--> wiredA2.ethg++;
        wiredA2.ethg++ <--> Eth1G <--> shared1.ethg++;
        shared1.ethg++ <--> Eth1G <--> shared2.ethg++;
        shared2.ethg++ <--> Eth1G <--> wiredB1.ethg++;
        wiredB1.ethg++ <--> Eth1G <--> wiredB2.ethg++;
        wiredB2.ethg++ <--> Eth1G <--> destination.ethg++;
}
