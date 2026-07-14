package inet.test.moduletest.lib;

//
// Scriptable MLDv2 SSM tester NIC's logic, the IPv6/MLD counterpart of ~IGMPTester.
// See TestMld.cc.
//
simple TestMld
{
    parameters:
        string interfaceTableModule;
        int mtu = default(1500);
        string checksumMode @enum("declared","computed") = default("declared");
    gates:
        input upperLayerIn @interface[inet::queueing::IPassivePacketSink](protocol=ipv6; service=request);    // mldIn; stands in for the Ipv6 module's transportIn from Mldv2's point of view
        output upperLayerOut;  // mldOut
}
