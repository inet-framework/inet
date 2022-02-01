//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.test.moduletest.lib.ieee80211retransmissiontest;

import inet.applications.contract.IApp;

//
// A very simple MPDU generator module
//
simple MpduGen like IApp
{
    // List of packets genereted by this module.
    // S means short data packet (SendDataWithACKFrame)
    // L means long data packet that is preceded by an RTS frame (SendDataWithRtsCts)
    // Example: packets = "SLS"
    string packets;
    string packetName = default("MPDU");
    int longPacketSize @unit(B) = default(1700B);
    int shortPacketSize @unit(B) = default(500B);

    string interfaceTableModule;   // The path to the InterfaceTable module
    int localPort = default(-1);  // local port (-1: use ephemeral port)
    int destPort;
    string destAddress;
    string localAddress = default("");
    double startTime @unit(s) = default(0.05s);
    @display("i=block/app");
    @signal[packetSent](type=cPacket);
    @signal[packetReceived](type=cPacket);
    @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
    @statistic[packetSent](title="packets sent"; source=packetSent; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);

    gates:
        input socketIn @labels(UdpControlInfo/up);
        output socketOut @labels(UdpControlInfo/down);
}
