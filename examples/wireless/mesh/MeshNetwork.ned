// SPDX-License-Identifier: LGPL-3.0-or-later
package inet.examples.wireless.mesh;

import inet.common.scenario.ScenarioManager;
import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.node.inet.StandardHost;
import inet.physicallayer.wireless.common.contract.packetlevel.IRadioMedium;
import inet.physicallayer.wireless.ieee80211.radiotap.Ieee80211PcapRecorder;

// Ordinary IPv4 hosts connected through native Layer-2 mesh forwarding.
network MeshNetwork
{
    parameters:
        int numStations = default(3);
        @display("bgb=650,350");
    submodules:
        configurator: Ipv4NetworkConfigurator;
        scenarioManager: ScenarioManager;
        radioMedium: <default("Ieee80211ScalarRadioMedium")> like IRadioMedium;
        station[numStations]: StandardHost;
        txCapture[numStations]: Ieee80211PcapRecorder;
        rxCapture[numStations]: Ieee80211PcapRecorder;
}
