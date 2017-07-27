import inet.common.lifecycle.NodeStatus;

simple TestProtocol
{
}

simple TestRadio
{
}

simple TestMobility
{
}

module TestNode
{
    parameters:
        @networkNode;
        @display("bgb=224,247");
    submodules:
        status: NodeStatus {
            @display("p=50,50");
        }
        mobility: TestMobility {
            @display("p=163,30");
        }
        protocol: TestProtocol {
            @display("p=163,112");
        }
        radio: TestRadio {
            @display("p=163,202");
        }
}
