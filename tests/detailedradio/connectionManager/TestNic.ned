package inet.tests.connectionManager;

module TestNic
{
    parameters:
        string connectionManagerName; //name of the ConnectionManager module
        string phyLayerType; //type of the physical layer
        @display("bgb=101,180,white,,;bgp=10,10");
    gates:
        input radioIn; // radioIn gate for sendDirect
    submodules:
        phy: <phyLayerType> like ITestPhyLayer {
            parameters:
                @display("p=60,150;i=prot3");
        }
    connections allowunconnected:

        radioIn --> phy.radioIn;

}

