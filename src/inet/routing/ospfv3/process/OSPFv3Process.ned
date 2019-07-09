package inet.routing.ospfv3.process;

simple OSPFv3Process
{
    parameters:
        int processID;
        string routerID;
        xml interfaceConfig;
        string interfaceTableModule;
        string routingTableModule;
        string routingTableModule6;
    gates:
       input splitterIn @label(OSPFv3Packet/up); 
       output splitterOut @label(OSPFv3Packet/down);
}