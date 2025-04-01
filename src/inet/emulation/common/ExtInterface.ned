//
// Copyright (C) 2020 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.emulation.common;

import inet.linklayer.contract.IExternalInterface;
import inet.networklayer.common.NetworkInterface;

//
// A base for external network interfaces, network interface
// modules that connect the simulation to the network stack of the host OS.
//
// Each external interface module is associated with a specific network device
// on the host OS. The host OS device is identified by the "namespace" and
// "device" parameters. The "namespace" parameter is optional; when empty, the
// default namespace is used.
//
// External interfaces can be mentally visualized as composed of an upper and a
// lower part, one of which is the host OS device and the other is part of the
// simulation. Therefore, there are two types of external interfaces, which are
// also reflected in their names:
//
// - In `ExtLower` interfaces, the lower part is a host OS device, and the upper
//   part is in the simulation. Therefore, an `ExtLower` interface module has
//   `upperLayerIn` and `upperLayerOut` gates for connecting to the upper layers.
//   The device is typically a "veth" (virtual Ethernet) device. (Using real
//   interfaces like "eth0" is also possible, but it needs special care to
//   prevent incoming packets from being replied to by both the simulation and the
//   kernel or other processes on the host OS.)
//
// - In `ExtUpper` interfaces, the upper part is a host OS device, and the
//   lower part is the simulated one. Therefore, an `ExtUpper` interface
//   module typically has gates for connecting to the simulated network.
//   The device is typically a "tap" (virtual TUN/TAP) device, which allows
//   the interception of the traffic of the host OS and routing them into the simulation.
//
// `ExtLower` interfaces are in general useful for connecting simulated apps to
// a real network, while `ExtUpper` interfaces are suitable for running a real
// networked application over a simulated network.
//
//
module ExtInterface extends NetworkInterface like IExternalInterface
{
    parameters:
        bool isWireless = default(false);
        string interfaceTableModule; // Module path of the interface table module
        string device; // Name of the attached OS device (e.g. tap0, veth0, etc.)
        string namespace = default("");  // Name of the OS network namespace (optional)
        string copyConfiguration @enum("","copyToExt","copyFromExt") = default(""); // Copy interface configuration such as MAC address, IP address, and MTU from/to the external interface.
        string address @mutable = default("auto");
        bool broadcast = default(true);
        bool multicast = default(true);
        bool pointToPoint = default(false);
        int mtu @unit(B) = default(4470B);
        *.interfaceTableModule = default(absPath(this.interfaceTableModule));
        *.device = default(this.device);
        *.namespace = default(this.namespace);
        @class(ExtInterface);
    gates:
        input upperLayerIn @loose;
        output upperLayerOut @loose;
}

