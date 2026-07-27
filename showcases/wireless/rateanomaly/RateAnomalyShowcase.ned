//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.showcases.wireless.rateanomaly;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.inet.StandardHost;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.contract.IIntegratedVisualizer;


network RateAnomalyShowcase
{
    parameters:
        int numHosts = default(5);
        @display("bgb=30,24");
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=3,2;is=s");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=3,6;is=s");
        }
        visualizer: <default(firstAvailableOrEmpty("IntegratedCanvasVisualizer"))> like IIntegratedVisualizer if typename != "" {
            @display("p=3,10;is=s");
        }
        server: StandardHost {
            @display("p=25,12");
        }
        accessPoint: AccessPoint {
            @display("p=17,12");
        }
        sta[numHosts]: WirelessHost {
            @display("p=9,12");
        }
    connections:
        accessPoint.ethg++ <--> Eth100M <--> server.ethg++;
}
