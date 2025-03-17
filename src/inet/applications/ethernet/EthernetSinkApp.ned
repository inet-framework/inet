//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.ethernet;

//
// Specializes the EthernetApp framework to create a unidirectional traffic sink
// that only receives and processes incoming packets over Ethernet without generating
// any outbound traffic. Designed for scenarios where only inbound traffic analysis
// is needed, such as measuring packet loss, delay, or throughput in one-way
// communication scenarios at the data link layer.
//
// The module disables the source component from the base EthernetApp while retaining
// all the traffic processing capabilities. This allows for configuring various packet
// handling and measurement operations through the sink submodule while maintaining a
// simplified architecture focused solely on packet reception. Serves as a natural
// counterpart to EthernetSourceApp in network testing and performance evaluation
// scenarios at the data link layer.
//
// @see ~EthernetApp, ~EthernetSourceApp, ~EthernetSocketIo
//
module EthernetSinkApp extends EthernetApp
{
    parameters:
        source.typename = "";
}
