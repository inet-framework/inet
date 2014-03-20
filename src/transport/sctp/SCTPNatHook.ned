package inet.transport.sctp;

import inet.base.IHook;


//
// SCTPNatHook module.
//
simple SCTPNatHook like IHook
{
    parameters:
        string interfaceTableModule;
        string networkProtocolModule;
        string routingTableModule;
    gates:
        input in;
        output out;
}