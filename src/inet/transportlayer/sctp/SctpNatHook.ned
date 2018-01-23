package inet.transportlayer.sctp;


//
// SCTPNatHook module.
//
simple SCTPNatHook
{
    parameters:
        string interfaceTableModule;
        string networkProtocolModule;
        string routingTableModule;
        string natTableModule;
}
