//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.applications.ethernet;

//
// Specializes the EthernetApp framework to create a unidirectional traffic source
// that only transmits packets over Ethernet without processing any incoming traffic.
// Designed for scenarios where only outbound traffic generation is needed, such as
// testing network performance under load or implementing data link layer broadcast
// sources.
//
// The module disables the sink component from the base EthernetApp while retaining
// all the traffic generation capabilities. This allows for configuring various traffic
// patterns through the source submodule while maintaining a simplified architecture
// focused solely on packet transmission. Can be paired with EthernetSinkApp modules
// at the receiving end to create complete traffic flow scenarios at the data link layer.
//
// @see ~EthernetApp, ~EthernetSinkApp, ~EthernetSocketIo
//
module EthernetSourceApp extends EthernetApp
{
    parameters:
        sink.typename = "";
}
