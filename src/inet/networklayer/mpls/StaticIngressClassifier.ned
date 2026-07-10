//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

package inet.networklayer.mpls;

import inet.common.SimpleModule;

//
// Signaling-free, statically configured ingress classifier for MPLS.
//
// Unlike ~RsvpClassifier (programmed at runtime by ~RsvpTe) or ~LdpClassifier
// (delegating to ~Ldp's live FEC state), this module's push-mapping table is
// loaded once, at startup, from static XML configuration and never changes
// afterwards. Combined with ~MplsRouterBase and static ~LibTable XML
// configuration (for the transit/egress LSRs' swap/pop entries), this makes
// it possible to build a fully static LSP network with NO signaling protocol
// running at all.
//
// The XML config file has the following format. The root element is
// <fectable>, which may contain any number of <fecentry> elements. Each
// <fecentry> is a leaf element with the following attributes:
// - dest: destination prefix's network address (e.g. "10.0.0.0")
// - netmask: the prefix's netmask (e.g. "255.0.0.0")
// - label: the label to push (integer, >= 0)
// - interface: the outgoing interface name (e.g. "ppp0")
//
// <pre>
// <fectable>
//     <fecentry dest="10.0.0.0" netmask="255.0.0.0" label="17" interface="ppp0"/>
// </fectable>
// </pre>
//
// On a lookup, the entry with the longest matching netmask wins (standard
// longest-prefix match); if no entry matches, the classifier reports "no
// mapping", and ~Mpls falls back to regular L3 routing for that packet.
//
simple StaticIngressClassifier extends SimpleModule like IIngressClassifier
{
    parameters:
        @class(StaticIngressClassifier);
        string interfaceTableModule; // The path to the InterfaceTable module; used to resolve the XML config's interface names to interface ids
        xml config = default(xml("<fectable/>"));
        @display("i=block/classifier");
}
