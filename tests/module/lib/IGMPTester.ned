simple IGMPTester
{
    parameters:
        string interfaceTableModule;
        int mtu = default(1500);
    gates:
        input upperLayerIn;    //igmpIn
        output upperLayerOut;  //igmpOut
}

