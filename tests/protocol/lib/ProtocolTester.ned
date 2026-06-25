//
// Protocol Test Framework for INET -- Phase 0: observer / event normaliser.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

//
// Phase 0 of the protocol test framework: a network-wide observer that subscribes
// to the standard INET packet signals, normalises each emission into a packet
// event, and (in logEvents mode) prints a one-line trace. Place one instance at
// the top level of a network.
//
simple ProtocolTester
{
    parameters:
        string testName = default(""); // name of a registered ProtocolTest program to run; empty = observe/log only
        bool logEvents = default(testName == ""); // print a one-line trace per observed packet event
        @display("i=block/cogwheel;is=s");
        @class(inet::protocoltest::ProtocolTester);
}
