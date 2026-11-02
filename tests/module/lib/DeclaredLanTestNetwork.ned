package inet.test.moduletest.lib;

import inet.common.MessageDispatcher;
import inet.linklayer.contract.IEthernetLayer;
import inet.linklayer.contract.IMacForwardingTable;
import inet.linklayer.ethernet.modular.LayeredEthernetInterface;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.configurator.contract.IL3NetworkConfigurator;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EthernetSwitch;
import inet.node.inet.Router;
import inet.node.inet.StandardHost;
import inet.node.inet.WirelessHost;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.protocolelement.contract.IProtocolLayer;

//
// A learning bridge inside a node: its ports are the node's bridge interfaces,
// registered in the bridge's own interface table, so the node's network layer
// does not see them.
//
module PortBridge
{
    parameters:
        @display("i=block/switch");
        **.interfaceTableModule = default(absPath(".interfaceTable"));
        **.macTableModule = default(absPath(".macTable"));
        ethernet.*.promiscuous = true;
    gates:
        input lowerLayerIn[];
        output lowerLayerOut[];
    submodules:
        macTable: <default("MacForwardingTable")> like IMacForwardingTable;
        interfaceTable: InterfaceTable;
        bridging: <default("BridgingLayer")> like IProtocolLayer;
        bl: MessageDispatcher;
        ethernet: <default("EthernetLayer")> like IEthernetLayer;
        li: MessageDispatcher;
    connections allowunconnected:
        bridging.lowerLayerOut --> bl.in++;
        bl.out++ --> bridging.lowerLayerIn;
        bl.out++ --> ethernet.upperLayerIn;
        ethernet.upperLayerOut --> bl.in++;
        ethernet.lowerLayerOut --> li.in++;
        li.out++ --> ethernet.lowerLayerIn;
        for i=0..sizeof(lowerLayerOut)-1 {
            li.out++ --> lowerLayerOut[i];
            lowerLayerIn[i] --> li.in++;
        }
}

//
// A router that routes on its eth[] interfaces and bridges its bridgeg[] gates:
// the gates' interfaces are ports of a bridge inside the node (~PortBridge),
// and they are not interfaces of the node's network layer.
//
module BridgingRouter extends Router
{
    gates:
        inout bridgeg[];
    submodules:
        bridgePort[sizeof(bridgeg)]: LayeredEthernetInterface {
            interfaceTableModule = absPath("^.bridge.interfaceTable");
            bitrate = default(100Mbps);
        }
        bridge: PortBridge;
    connections:
        for i=0..sizeof(bridgeg)-1 {
            bridge.lowerLayerOut++ --> bridgePort[i].upperLayerIn;
            bridgePort[i].upperLayerOut --> bridge.lowerLayerIn++;
            bridgePort[i].phys <--> bridgeg[i];
        }
}

//
// A LAN behind a router-and-bridge node: router, hostA and hostB are on the
// bridged gates of rb, which also routes between transit and hostC on its own
// interfaces. The bridged LAN reaches the rest of the network through router.
//
network DeclaredLanTestNetwork
{
    submodules:
        configurator: <default("Ipv4NetworkConfigurator")> like IL3NetworkConfigurator;
        router: Router;
        transit: Router;
        rb: BridgingRouter;
        hostA: StandardHost;
        hostB: StandardHost;
        hostC: StandardHost;
    connections:
        router.ethg++ <--> Eth100M <--> rb.bridgeg++;
        hostA.ethg++ <--> Eth100M <--> rb.bridgeg++;
        hostB.ethg++ <--> Eth100M <--> rb.bridgeg++;
        router.ethg++ <--> Eth100M <--> transit.ethg++;
        transit.ethg++ <--> Eth100M <--> rb.ethg++;
        rb.ethg++ <--> Eth100M <--> hostC.ethg++;
}

//
// Interfaces that no wire joins with the LAN behind rb's bridged gate: two
// wireless hosts on one radio medium, hostD on a switch with hostE, and hostF
// wired directly to hostG.
//
network DeclaredLanInterfacesTestNetwork
{
    submodules:
        configurator: <default("Ipv4NetworkConfigurator")> like IL3NetworkConfigurator;
        radioMedium: Ieee80211ScalarRadioMedium;
        rb: BridgingRouter;
        hostA: StandardHost;
        mobile[2]: WirelessHost;
        switch: EthernetSwitch;
        hostD: StandardHost;
        hostE: StandardHost;
        hostF: StandardHost;
        hostG: StandardHost;
    connections:
        hostA.ethg++ <--> Eth100M <--> rb.bridgeg++;
        hostD.ethg++ <--> Eth100M <--> switch.ethg++;
        hostE.ethg++ <--> Eth100M <--> switch.ethg++;
        hostF.ethg++ <--> Eth100M <--> hostG.ethg++;
}
