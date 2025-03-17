//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.ieee8022;

//
// Specializes the Ieee8022LlcApp framework to create a unidirectional traffic sink
// that only receives and processes incoming packets over the IEEE 802.2 Logical Link
// Control (LLC) layer without generating any outbound traffic. Designed for scenarios
// where only inbound traffic analysis is needed, such as measuring packet loss, delay,
// or throughput in one-way communication scenarios at the data link layer.
//
// The module disables the source component from the base Ieee8022LlcApp while retaining
// all the traffic processing capabilities. This allows for configuring various packet
// handling and measurement operations through the sink submodule while maintaining a
// simplified architecture focused solely on packet reception. Serves as a natural
// counterpart to Ieee8022LlcSourceApp in network testing and performance evaluation
// scenarios at the data link layer.
//
// @see ~Ieee8022LlcApp, ~Ieee8022LlcSourceApp, ~Ieee8022LlcSocketIo
//
module Ieee8022LlcSinkApp extends Ieee8022LlcApp
{
    parameters:
        source.typename = "";
}
