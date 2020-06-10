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

package inet.examples.wireless.configurator;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.EtherLink;
import inet.node.inet.AdhocHost;
import inet.node.inet.Router;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.contract.IIntegratedVisualizer;


network ComplexConfiguratorNetwork
{
    parameters:
        *.eth[*].bitrate = 1 Gbps;
    submodules:
        visualizer: <default("IntegratedCanvasVisualizer")> like IIntegratedVisualizer if hasVisualizer() {
            parameters:
                @display("p=100,400;is=s");
        }
        configurator: Ipv4NetworkConfigurator {
            parameters:
                @display("p=100,100;is=s");
        }
        radioMediumA: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,200;is=s");
        }
        radioMediumB: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,300;is=s");
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
        source.ethg++ <--> EtherLink <--> wiredA1.ethg++;
        wiredA1.ethg++ <--> EtherLink <--> wiredA2.ethg++;
        wiredA2.ethg++ <--> EtherLink <--> shared1.ethg++;
        shared1.ethg++ <--> EtherLink <--> shared2.ethg++;
        shared2.ethg++ <--> EtherLink <--> wiredB1.ethg++;
        wiredB1.ethg++ <--> EtherLink <--> wiredB2.ethg++;
        wiredB2.ethg++ <--> EtherLink <--> destination.ethg++;
}
