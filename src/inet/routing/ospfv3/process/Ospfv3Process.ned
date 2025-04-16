package inet.routing.ospfv3.process;

//
// Represents a single OSPFv3 routing process within a router. Handles the core
// functionality of the OSPFv3 protocol including neighbor discovery, database
// synchronization, shortest path calculation, and route installation. Multiple
// processes can run simultaneously on a router, each with its own process ID
// and router ID.
//
simple Ospfv3Process
{
    parameters:
        int processID;
        string routerID;
        xml interfaceConfig;
        string interfaceTableModule;
        string routingTableModule;
        string routingTableModule6;
    gates:
        input splitterIn @label(Ospfv3Packet/up);
        output splitterOut @label(Ospfv3Packet/down);
}

