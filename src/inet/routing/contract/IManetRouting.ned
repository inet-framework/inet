package inet.routing.contract;

moduleinterface IManetRouting
{
    parameters:
        @display("i=block/app");
    gates:
        input ipIn;
        output ipOut;
}
