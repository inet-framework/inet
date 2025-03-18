
//TODO review
package inet.routing.ospfv3;

import inet.common.SimpleModule;

//
// Routes OSPFv3 packets between the network layer and the appropriate OSPFv3
// process modules. Manages multiple routing processes, directing incoming packets
// to the correct process based on configuration. Handles both IPv4 and IPv6
// routing tables, allowing OSPFv3 to support dual-stack environments.
//
simple Ospfv3Splitter extends SimpleModule
{
    parameters:
        @class(Ospfv3Splitter);                     // these are parameters which need to be filled in the omnetpp.ini file
        xml ospfv3RoutingConfig;
        xml ospfv3IntConfig;

        string interfaceTableModule;
        string routingTableModule;
        string routingTableModule6;
    gates:
        input ipIn @labels(Ipv6ControlInfo/up);
        output ipOut @labels(Ipv6ControlInfo/down);
        //        input ipIn @labels(Ospfv3Packet/up);
        //        output ipOut @labels(Ospfv3Packet/down);
        input processIn[];
        output processOut[];
}
