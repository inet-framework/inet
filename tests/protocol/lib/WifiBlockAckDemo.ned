//
// Protocol Test Framework for INET -- WiFi A-MSDU aggregation + Block Ack demo network.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.AdhocHost;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211RadioMedium;

//
// Two ad-hoc 802.11n hosts; host1 bursts UDP traffic to host2 so the MAC sets up a
// Block Ack agreement and aggregates QoS data (A-MSDU). The ProtocolTester observes the
// resulting frame sequence at the MAC layer.
//
network WifiBlockAckDemo
{
    submodules:
        configurator: Ipv4NetworkConfigurator {
            @display("p=50,50");
        }
        radioMedium: Ieee80211RadioMedium {
            @display("p=50,100");
        }
        tester: ProtocolTester {
            @display("p=50,150");
        }
        host1: AdhocHost {
            @display("p=288,110");
        }
        host2: AdhocHost {
            @display("p=480,258");
        }
}
