//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.configurator.common;

//
// Provides centralized configuration for layer 2 network scenarios, particularly
// for spanning tree protocols (STP/RSTP) and other link layer parameters. This module
// uses XML-based configuration to set port costs, priorities, and other interface
// attributes across the network.
//
// The configurator operates on a network-wide basis, analyzing the topology and
// applying configurations to multiple devices simultaneously. It inherits the powerful
// selector mechanism from ~Ipv4NetworkConfigurator, allowing precise targeting of
// specific hosts or interfaces.
//
// Key features:
// - XML-based configuration for layer 2 parameters
// - Support for spanning tree protocol configuration (STP/RSTP)
// - Flexible node and interface selection using patterns
// - Port-specific parameter configuration
//
// The configuration uses selector attributes to target specific elements:
// - @hosts: Selects host modules by name pattern
// - @names: Selects interfaces by name
// - @towards: Selects interfaces facing specific nodes
// - @among: Selects interfaces between specified node groups
// - @ports: Selects specific ports on devices
//
// Example configuration:
// <pre>
// <config>
//   <interface hosts='**' ports='5' cost='19' priority='32768'/>
// </config>
// </pre>
//
// This example sets port 5 on all switches to have a cost of 19 and priority of 32768,
// which affects spanning tree protocol path selection.
//
// @see ~Ipv4NetworkConfigurator, ~Stp, ~Rstp
//
simple L2NetworkConfigurator
{
    parameters:
        @display("i=block/cogwheel");
        xml config = default(xml("<config><interface hosts='**' ports='**' cost='19' priority='128' edge='false'/></config>"));
}
