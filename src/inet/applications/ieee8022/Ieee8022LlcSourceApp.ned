//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.ieee8022;

//
// Specializes the Ieee8022LlcApp framework to create a unidirectional traffic source
// that only transmits packets over the IEEE 802.2 Logical Link Control (LLC) layer
// without processing any incoming traffic. Designed for scenarios where only outbound
// traffic generation is needed, such as testing network performance under load or
// implementing data link layer broadcast sources.
//
// The module disables the sink component from the base Ieee8022LlcApp while retaining
// all the traffic generation capabilities. This allows for configuring various traffic
// patterns through the source submodule while maintaining a simplified architecture
// focused solely on packet transmission. Can be paired with Ieee8022LlcSinkApp modules
// at the receiving end to create complete traffic flow scenarios at the data link layer.
//
// @see ~Ieee8022LlcApp, ~Ieee8022LlcSinkApp, ~Ieee8022LlcSocketIo
//
module Ieee8022LlcSourceApp extends Ieee8022LlcApp
{
    parameters:
        sink.typename = "";
}
