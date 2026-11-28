// SPDX-License-Identifier: LGPL-3.0-or-later
package inet.examples.mqttsn;
import inet.networks.base.NetworkBase;
import inet.node.inet.AdhocHost;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;

// Application UDP over an existing Wi-Fi/IP model. This is not a native ZigBee binding.
network MqttSnWirelessNetwork extends NetworkBase
{
    parameters:
        visualizer.typename = default("");
        **.ipv4.arp.typename = default("GlobalArp");
    submodules:
        radioMedium: Ieee80211ScalarRadioMedium { @display("p=100,100"); }
        sensor: AdhocHost { @display("p=20,20"); }
        gateway: AdhocHost { @display("p=50,20"); }
        broker: AdhocHost { @display("p=20,50"); }
        observer: AdhocHost { @display("p=50,50"); }
}
