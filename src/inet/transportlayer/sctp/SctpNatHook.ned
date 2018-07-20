package inet.transportlayer.sctp;


//
// SctpNatHook module.
//
simple SctpNatHook
{
    parameters:
        string interfaceTableModule;
        string networkProtocolModule;
        string routingTableModule;
        string natTableModule;
}
