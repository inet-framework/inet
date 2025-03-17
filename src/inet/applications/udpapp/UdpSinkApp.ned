//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.udpapp;

//
// Specializes the UdpApp framework to create a unidirectional UDP traffic sink
// that only receives and processes incoming packets without generating any outbound
// traffic. Designed for scenarios where only inbound traffic analysis is needed,
// such as measuring packet loss, delay, or throughput in one-way communication
// scenarios.
//
// The module disables the source component from the base UdpApp while retaining all
// the traffic processing capabilities. This allows for configuring various packet
// handling and measurement operations through the sink submodule while maintaining
// a simplified architecture focused solely on packet reception. Serves as a natural
// counterpart to UdpSourceApp in network testing and performance evaluation scenarios.
//
// @see ~UdpApp, ~UdpSourceApp, ~UdpSink, ~UdpSocketIo
//
module UdpSinkApp extends UdpApp
{
    parameters:
        source.typename = "";
        io.destPort = -1;
}
