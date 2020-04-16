package inet.routing.ospfv3.process;

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

