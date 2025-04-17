package inet.routing.ospfv3.process;

import inet.common.SimpleModule;

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
    gates:
        input splitterIn @label(Ospfv3Packet/up);
        output splitterOut @label(Ospfv3Packet/down);
}

