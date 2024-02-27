//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.configurator;

//
// This module allows to configure network scenarios at layer 2.
// The ~Stp and ~Rstp related parameters such as link cost, port priority
// and the "is-edge" flag can be configured with XML files.
//
// This module is based on ~Ipv4NetworkConfigurator. The optional selector
// attributes: @hosts, @names, @towards, @among are also comes with this
// configurator and each of them behaves similarly to its ~Ipv4NetworkConfigurator equivalent.
// @ports selector added to configuring per-port parameters in Ieee8021dInterfaceData.
//
// The following example configures port 5 (if it exists) on all switches to cost 19 and
// priority 32768:
// <pre>
// <config>
//   <interface hosts='**' ports='5' cost='19' priority='32768'/>
// </config>
// <pre>
//
// For more information about the usage of the selector attributes see ~Ipv4NetworkConfigurator.
//
simple L2NetworkConfigurator
{
    parameters:
        @display("i=block/cogwheel");
        xml config = default(xml("<config><interface hosts='**' ports='**' cost='19' priority='128' edge='false'/></config>"));
}

