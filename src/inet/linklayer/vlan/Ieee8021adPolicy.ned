//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


package inet.linklayer.vlan;

import inet.protocolelement.contract.IProtocolLayer;
import inet.queueing.base.PacketProcessorBase;

//
// Applies the VLAN policy of a VLAN-aware bridge of an IEEE 802.1ad network,
// configured by the port type of each port. Place it in the `vlanPolicy`
// submodule of ~BridgingLayer, which sees every frame between the relay and
// the network interfaces.
//
// Each port type is a fixed setting of the port parameters of IEEE 802.1Q-2018
// (Acceptable Frame Types, PVID, member set, untagged set; clauses 6.9, 8.6.2,
// 8.6.4 and 8.8.2), with ingress filtering always enabled:
//
// - `access`: a port of a C-VLAN bridge in one C-VLAN. It accepts untagged and
//   priority-tagged frames and classifies them into the VLAN `vid`, drops
//   C-tagged frames, and sends the frames of `vid` untagged.
// - `trunk`: a port of a C-VLAN bridge in the C-VLANs `vids`. It accepts and
//   sends C-tagged frames of these VLANs only.
// - `cnp`: a Customer Network Port of an S-VLAN bridge with a port-based
//   service interface (IEEE 802.1Q-2018 15.3). It is an `access` port in the
//   S-VLAN `vid`: it accepts frames without an S-tag or with an S priority tag,
//   and sends the frames of `vid` without an S-tag. A C-tag of the customer
//   stays in the payload.
// - `pnp`: a Provider Network Port of an S-VLAN bridge in the S-VLANs `vids`.
//   It is a `trunk` port with S-tags. A Customer Network Port with an S-tagged
//   service interface (15.5) and no VLAN id translation is configured as `pnp`.
//
// The `ports` parameter maps the name of each network interface to its port:
// `{"type" : "access", "vid" : 10}` or `{"type" : "trunk", "vids" : [10, 20]}`.
// The interface name '*' matches every interface that has no entry of its
// own. Every port of the relay must be covered. For example:
//
// `{"eth0" : {"type" : "access", "vid" : 10}, "eth1" : {"type" : "trunk", "vids" : [10, 20]}}`
//
// `access` and `trunk` require `vlanTagType = "c"`, `cnp` and `pnp` require
// `vlanTagType = "s"`. The VLAN ids are from 1 to 4094.
//
// A frame sent untagged leaves without any tag of `vlanTagType`: the module
// removes the PCP and drop eligible requests of the tag too, so that no priority
// tag appears. A frame that the bridge itself sends, without a VLAN, is sent
// untagged on `access` and `cnp` ports and dropped on `trunk` and `pnp` ports.
// Frames whose tags the port does not accept are dropped.
//
// The module does not support VLAN id translation, hybrid ports, and the
// C-tagged service interface of a Provider Edge Bridge (Customer Edge Port,
// 15.4). For these, and for hosts, see ~VlanPolicyLayer.
//
simple Ieee8021adPolicy extends PacketProcessorBase like IProtocolLayer
{
    parameters:
        string interfaceTableModule; // Relative module path of the interface table
        string vlanTagType @enum("c","s") = default("c"); // The VLAN tag whose tags the module uses: "c" for the C-tag (C-VLAN bridge), "s" for the S-tag (S-VLAN bridge)
        object ports; // Maps network interface name to its port, a map with the field 'type' ("access", "trunk", "cnp" or "pnp") and 'vid' (access, cnp) or 'vids' (trunk, pnp); '*' as interface name matches all interfaces
        displayStringTextFormat = default("dropped {numDroppedPackets}/{numProcessedPackets} pk");
        @class(Ieee8021adPolicy);
        @display("i=block/filter");
        @signal[packetDropped](type=inet::Packet);
        // the statistical value is the dropped packet
        @statistic[droppedPackets](title="dropped packets"; source=packetDropped; record=count; unit=pk);
    gates:
        input upperLayerIn;
        output upperLayerOut;
        input lowerLayerIn;
        output lowerLayerOut;
}
