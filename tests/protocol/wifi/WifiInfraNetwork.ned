//
// WiFi conformance suite -- infrastructure network: one AP and two STAs on a shared
// 802.11 radio medium, with full (non-simplified) management so the real
// Beacon -> Probe -> Auth -> Assoc handshake happens. The per-generation PHY
// (opMode/band/antennas) is set per [Config] in omnetpp.ini, so this one network
// serves every WiFi generation. The ProtocolTester observes the MAC frame sequence.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
package inet.protocoltest.wifi;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.WirelessHost;
import inet.node.wireless.AccessPoint;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.protocoltest.ProtocolTester;

network WifiInfraNetwork
{
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=50,50");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=50,120");
        }
        tester: ProtocolTester {
            @display("p=50,190");
        }
        ap: AccessPoint {
            @display("p=300,150");
        }
        sta1: WirelessHost {
            @display("p=200,260");
        }
        sta2: WirelessHost {
            @display("p=400,260");
        }
}
