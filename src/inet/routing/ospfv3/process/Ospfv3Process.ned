package inet.routing.ospfv3.process;

import inet.common.SimpleModule;

//
// Represents a single OSPFv3 routing process within a router. Handles the core
// functionality of the OSPFv3 protocol including neighbor discovery, database
// synchronization, shortest path calculation, and route installation. Multiple
// processes can run simultaneously on a router, each with its own process ID
// and router ID.
//
simple Ospfv3Process extends SimpleModule
{
    parameters:
        @class(Ospfv3Process);
        int processID;
        string routerID;
        xml interfaceConfig;
        string interfaceTableModule;
        string routingTableModule;
        string routingTableModule6;
        string checksumMode @enum("declared", "computed") = default("declared"); // checksum mode for outgoing OSPFv3 packets and originated LSAs ("computed" yields RFC 5340 2.5 / RFC 2328 12.1.7 checksums, e.g. for pcap/interop)
    gates:
        input splitterIn @label(Ospfv3Packet/up) @interface[inet::queueing::IPassivePacketSink];
        output splitterOut @label(Ospfv3Packet/down);
}

