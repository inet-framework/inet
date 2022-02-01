//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.transportlayer.rtp;

//
// RTCP end system.
//
simple Rtcp
{
    parameters:
        string interfaceTableModule;   // The path to the InterfaceTable module
        @display("i=block/control");
        @signal[packetReceived](type=cPacket);
        @statistic[packetReceived](title="packets received"; source=packetReceived; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[endToEndDelay](title="end-to-end delay"; source="dataAge(packetReceived)"; unit=s; record=histogram,vector; interpolationmode=none);

    gates:
        input rtpIn @labels(RtpInnerPacket);
        input udpIn @labels(UdpControlInfo/up);
        output rtpOut @labels(RtpInnerPacket);
        output udpOut @labels(UdpControlInfo/down);
}

