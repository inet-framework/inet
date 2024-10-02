package inet.test.moduletest.lib;

simple IGMPTester
{
    parameters:
        string interfaceTableModule;
        int mtu = default(1500);
        string crcMode @enum("declared","computed") = default("declared");
    gates:
        input upperLayerIn @interface[inet::queueing::IPassivePacketSink](pdu=igmp);    //igmpIn
        output upperLayerOut;  //igmpOut
}

