//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.ipapp;

//
// Specializes the IpApp framework to create a unidirectional IP traffic source
// that only transmits packets without processing any incoming traffic. Designed
// for scenarios where only outbound traffic generation is needed, such as testing
// network performance under load, simulating one-way data streams, or implementing
// IP-based broadcast sources.
//
// The module disables the sink component from the base IpApp while retaining all
// the traffic generation capabilities. This allows for configuring various traffic
// patterns through the source submodule while maintaining a simplified architecture
// focused solely on packet transmission. Can be paired with IpSink or IpSinkApp
// modules at the receiving end to create complete traffic flow scenarios.
//
// @see ~IpApp, ~IpSinkApp, ~IpSocketIo
//
module IpSourceApp extends IpApp
{
    parameters:
        sink.typename = "";
}
