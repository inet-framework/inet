package inet.tests.connectionManager;

import inet.tests.TestObject;

// Common phy layer test interface.
moduleinterface ITestPhyLayer
{
    parameters:
        bool usePropagationDelay; //Should transmission delay be simulated?
    gates:
        input radioIn;
        input upperLayerIn;
        output upperLayerOut;
}

simple TestPhyLayer extends TestObject
{
    parameters:
        bool usePropagationDelay;
    gates:
        input radioIn;
        input upperLayerIn;
        output upperLayerOut;
}

// Physical layer test object which broadcasts packets and expects at least
// one answer.
simple ConnectedBCNodePhyLayer extends TestPhyLayer like ITestPhyLayer
{
    parameters:
        @class(ConnectedBCNodePhyLayer);
        usePropagationDelay = false; // no transmission delay
}

// Physical layer test object which broadcasts packets and expects no answer
// (bacause of missing connectivity).
simple NotConnectedBCNodePhyLayer extends TestPhyLayer like ITestPhyLayer
{
    parameters:
        @class(NotConnectedBCNodePhyLayer);
        usePropagationDelay = false; // no transmission delay
}

// Physical layer test object which expects to receive and answers at
// least one broadcast.
simple ConnectedRNodePhyLayer extends TestPhyLayer like ITestPhyLayer
{
    parameters:
        @class(ConnectedRNodePhyLayer);
        usePropagationDelay = false; // no transmission delay
}

// Physical layer test object which expects not to receive
// broadcasts (bacause of missing connectivity).
simple NotConnectedRNodePhyLayer extends TestPhyLayer like ITestPhyLayer
{
    parameters:
        @class(NotConnectedRNodePhyLayer);
        usePropagationDelay = false; // no transmission delay
}

