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
        input upperLayerIn;    // mldIn
        output upperLayerOut;  // mldOut
}
