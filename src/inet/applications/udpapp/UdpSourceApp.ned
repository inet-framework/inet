//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.udpapp;

//
// Specializes the UdpApp framework to create a unidirectional UDP traffic source
// that only transmits packets without processing any incoming traffic. Designed
// for scenarios where only outbound traffic generation is needed, such as testing
// network performance under load, simulating one-way data streams, or implementing
// UDP-based broadcast/multicast sources.
//
// The module disables the sink component from the base UdpApp while retaining all
// the traffic generation capabilities. This allows for configuring various traffic
// patterns through the source submodule while maintaining a simplified architecture
// focused solely on packet transmission. Can be paired with UdpSink or UdpSinkApp
// modules at the receiving end to create complete traffic flow scenarios.
//
// @see ~UdpApp, ~UdpSink, ~UdpSinkApp, ~UdpSocketIo
//
module UdpSourceApp extends UdpApp
{
    parameters:
        sink.typename = "";
}
