
package inet.routing.ospfv3;

simple Ospfv3Splitter
{
    parameters:                     //these are parameteters which need to be filled in omnetpp.ini file
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

