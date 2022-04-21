//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.showcases.measurement.flow;

import inet.networks.base.WiredNetworkBase;
import inet.node.contract.IEthernetNetworkNode;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.StandardHost;
import inet.protocolelement.measurement.IMeasurementLayer;

module MyStandardHost extends StandardHost
{
    submodules:
        measurementLayer: <default("")> like IMeasurementLayer {
            @display("p=450,250");
        }
    connections:
        udp.ipOut --> { @reconnect; } --> measurementLayer.upperLayerIn if hasUdp;
        measurementLayer.lowerLayerOut --> { @reconnect; } --> tn.in[0] if hasUdp;
        udp.ipIn <-- { @reconnect; } <-- measurementLayer.upperLayerOut if hasUdp;
        measurementLayer.lowerLayerIn <-- { @reconnect; } <-- tn.out[0] if hasUdp;
}

network FlowMeasurementShowcase extends WiredNetworkBase
{
    submodules:
        client1: <default("StandardHost")> like IEthernetNetworkNode {
            @display("p=300,150");
        }
        client2: <default("StandardHost")> like IEthernetNetworkNode {
            @display("p=300,350");
        }
        switch1: EthernetSwitch {
            @display("p=500,250");
        }
        switch2: EthernetSwitch {
            @display("p=700,250");
        }
        server1: <default("StandardHost")> like IEthernetNetworkNode {
            @display("p=900,150");
        }
        server2: <default("StandardHost")> like IEthernetNetworkNode {
            @display("p=900,350");
        }
    connections:
        client1.ethg++ <--> Eth100M <--> switch1.ethg++;
        client2.ethg++ <--> Eth100M <--> switch1.ethg++;
        switch2.ethg++ <--> Eth100M <--> server1.ethg++;
        switch2.ethg++ <--> Eth100M <--> server2.ethg++;
        switch1.ethg++ <--> Eth100M <--> switch2.ethg++;
}

