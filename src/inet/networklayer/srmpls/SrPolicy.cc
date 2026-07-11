//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/srmpls/SrPolicy.h"

#include <omnetpp/cstringtokenizer.h>

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/XMLUtils.h"
#include "inet/networklayer/common/L3Tools.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/mpls/LibTable.h"

namespace inet {

Define_Module(SrPolicy);

using namespace xmlutils;

void SrPolicy::initialize(int stage)
{
    SimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        ift.reference(this, "interfaceTableModule", true);
        sr.reference(this, "segmentRoutingModule", true);
        WATCH(policyTable);
    }
    else if (stage == INITSTAGE_NETWORK_LAYER) {
        // By this stage all interfaces are registered in ift (mirrors StaticIngressClassifier's
        // own timing), AND SegmentRouting has already parsed its sidTable (INITSTAGE_LOCAL, a
        // strictly earlier stage run by ALL modules before ANY module's INITSTAGE_NETWORK_LAYER
        // -- no dependency on submodule declaration order needed here).
        readPoliciesFromXML(par("policies").xmlValue());
    }
}

void SrPolicy::handleMessage(cMessage *msg)
{
    throw cRuntimeError("SrPolicy does not process messages, but received '%s'", msg->getName());
}

// IIngressClassifier implementation (method invoked by MPLS)

bool SrPolicy::lookupLabel(Packet *packet, LabelOpVector& outLabel, int& outInterfaceId)
{
    // Dual-stack (Workstream F3): see ~StaticIngressClassifier::lookupLabel() for the same
    // generic-L3Address dispatch (via PacketProtocolTag + L3Tools::peekNetworkProtocolHeader()).
    const Protocol *protocol = packet->getTag<PacketProtocolTag>()->getProtocol();
    L3Address destAddress = peekNetworkProtocolHeader(packet, *protocol)->getDestinationAddress();

    // longest-prefix match: the entry with the most specific (longest) prefix length wins.
    // policyTable may hold a MIX of IPv4 and IPv6 entries; see
    // ~StaticIngressClassifier::lookupLabel() for why cross-family entries must be skipped
    // before calling L3Address::matches() (it would throw).
    const PolicyEntry *best = nullptr;
    for (auto& entry : policyTable) {
        if (entry.dest.getType() != destAddress.getType())
            continue;
        if (!destAddress.matches(entry.dest, entry.prefixLength))
            continue;

        if (!best || entry.prefixLength > best->prefixLength)
            best = &entry;
    }

    if (!best)
        return false;

    if (!resolveSegmentList(best->segments, outLabel, outInterfaceId))
        return false;

    // Workstream F3 (6PE-lite): a node-SID label is shared by ALL traffic destined to that
    // node, of either family -- SegmentRouting installs exactly one, family-agnostic PHP POP
    // LIB entry per node-SID (see SegmentRouting.cc), so unlike Phase 1's per-FEC LIB entries
    // (LDP/RSVP-TE/static XML, where one entry = one FEC = one family), the popping router's
    // own LIB entry cannot carry a payload protocol that is correct for BOTH families sharing
    // the label. Baking payloadProtocol into that shared entry was therefore never viable for
    // SR: it would silently mislabel the OTHER family the moment both rode the same node-SID.
    //
    // RFC 3032's IPv6 Explicit NULL label exists for exactly this kind of demultiplexing.
    // Push it as the INNERMOST label -- beneath every node:/adj: segment resolveSegmentList()
    // just produced -- so it rides through every transit PHP pop untouched: Mpls::popLabel()
    // only stamps a payload protocol when the popped label is bottom-of-stack, and a non-
    // bottom pop leaves the packet mpls-tagged and simply forwarded via the resolved
    // outInterfaceId (Mpls::processMplsPacketFromL2's "still mpls" branch) -- no transit
    // router needs to know or care that IPv6 is underneath. Only once this label reaches the
    // bottom of the stack, at the true egress, does Mpls's existing IPV6_EXPLICIT_NULL_LABEL
    // handling (Workstream F3 Phase 2, unused until now) pop it and correctly stamp
    // Protocol::ipv6 -- regardless of what the egress's OWN (also family-agnostic) node-SID
    // LIB entry would have defaulted to.
    //
    // IPv4 destinations need no such marker: LibTable's payloadProtocol already defaults to
    // ipv4, so the family-agnostic node-SID POP entries are correct as-is; this block is
    // therefore a no-op for every existing (IPv4-only) policy/example.
    if (destAddress.getType() == L3Address::IPv6 && !outLabel.empty()) {
        LabelOp explicitNull;
        explicitNull.optcode = PUSH_OPER;
        explicitNull.label = IPV6_EXPLICIT_NULL_LABEL;
        outLabel.insert(outLabel.begin(), explicitNull);
    }

    return true;
}

bool SrPolicy::resolveSegmentList(const std::vector<Segment>& segments, LabelOpVector& outLabel, int& outInterfaceId)
{
    ASSERT(!segments.empty()); // guaranteed by readPoliciesFromXML: an empty <policy> is rejected at load time

    // First-segment canonicalization (see .ned doc): determine the outgoing interface for
    // segments[0], and whether pushing its label would be pointless.
    int firstInterfaceId;
    bool skipFirst;

    if (segments[0].kind == Segment::ADJ) {
        // An adj: segment is, by construction (see .ned doc's Limitations section), always one
        // of THIS router's own local links: "crossing" it IS exactly what forwarding out
        // `interfaceId` already achieves, so as the first segment it is always dropped from the
        // pushed stack -- its LIB entry lives on THIS SAME router (SegmentRouting installed it
        // here), and ingress-classified traffic never consults the LIB at all (see
        // Mpls::tryLabelAndForwardDatagram), so pushing it would leave a label on the wire
        // that no router downstream would ever pop.
        skipFirst = true;
        firstInterfaceId = segments[0].interfaceId;
    }
    else { // Segment::NODE
        bool directlyConnected;
        if (!sr->resolveNextHop(segments[0].nodeRouter, firstInterfaceId, directlyConnected))
            return false; // segments[0]'s node is currently unreachable: no LSP available right now

        // If this router is itself the penultimate hop toward segments[0]'s node (PHP, RFC 8660
        // Section 2), pushing segments[0]'s label would be pointless: the very next router
        // expects to receive the packet with it already popped (SegmentRouting installs no LIB
        // entry for its OWN node SID, exactly for this reason -- see SegmentRouting.cc).
        skipFirst = directlyConnected;
    }

    size_t firstIndex = skipFirst ? 1 : 0;

    if (firstIndex == segments.size()) {
        // The entire policy collapsed to nothing (a single-segment policy whose one segment was
        // dropped by the canonicalization above): no label needs to be imposed at all. Report
        // "no mapping" (see .ned doc's Limitations section) rather than violating Mpls's
        // non-empty-outLabel invariant (Mpls::tryLabelAndForwardDatagram ASSERTs
        // outLabel.size() > 0) with an empty PUSH stack.
        return false;
    }

    outLabel.clear();
    // Push in REVERSE segment-list order: LibTable::pushLabel()/Mpls::doStackOps() insert each
    // PUSH at the front of the packet's label stack, so the LAST op processed ends up on top --
    // i.e. segments[firstIndex] (the segment that must ride outermost, since it is the one
    // processed by the network first) must be the LAST op appended to outLabel.
    for (size_t i = segments.size(); i-- > firstIndex; ) {
        int label = (segments[i].kind == Segment::NODE) ? segments[i].label : sr->getAdjSidLabel(segments[i].interfaceId);
        if (label < 0)
            return false; // an adjacency segment beneath the first one is currently withdrawn (its link is down)

        LabelOp op;
        op.optcode = PUSH_OPER;
        op.label = label;
        outLabel.push_back(op);
    }

    outInterfaceId = firstInterfaceId;
    return true;
}

// static configuration

void SrPolicy::readPoliciesFromXML(const cXMLElement *policiesXml)
{
    ASSERT(policiesXml);
    if (strcmp(policiesXml->getTagName(), "policies") != 0)
        throw cRuntimeError("SrPolicy: policies' root element must be <policies>, but is <%s> at %s",
                policiesXml->getTagName(), policiesXml->getSourceLocation());
    checkTags(policiesXml, "policy");

    cXMLElementList list = policiesXml->getChildrenByTagName("policy");
    for (auto& elem : list) {
        const cXMLElement& entry = *elem;

        const char *destStr = entry.getAttribute("dest");
        if (!destStr)
            throw cRuntimeError("Invalid policy at %s: missing 'dest' attribute", entry.getSourceLocation());
        L3Address dest;
        if (!dest.tryParse(destStr))
            throw cRuntimeError("Invalid policy at %s: 'dest' is not a valid IPv4/IPv6 address: '%s'", entry.getSourceLocation(), destStr);

        // Dual-stack (Workstream F3): see ~StaticIngressClassifier::readTableFromXML() for the
        // same netmask=/prefixLength= backward-compat rationale.
        const char *netmaskStr = entry.getAttribute("netmask");
        const char *prefixLengthStr = entry.getAttribute("prefixLength");
        if (netmaskStr && prefixLengthStr)
            throw cRuntimeError("Invalid policy at %s: 'netmask' and 'prefixLength' are mutually exclusive, but both were given", entry.getSourceLocation());
        if (!netmaskStr && !prefixLengthStr)
            throw cRuntimeError("Invalid policy at %s: missing 'netmask' or 'prefixLength' attribute", entry.getSourceLocation());

        int prefixLength;
        if (netmaskStr) {
            if (!Ipv4Address::isWellFormed(netmaskStr))
                throw cRuntimeError("Invalid policy at %s: 'netmask' is not a valid IPv4 address: '%s'", entry.getSourceLocation(), netmaskStr);
            Ipv4Address netmask(netmaskStr);
            if (!netmask.isValidNetmask())
                throw cRuntimeError("Invalid policy at %s: 'netmask' is not a valid netmask (non-contiguous bits): '%s'", entry.getSourceLocation(), netmaskStr);
            prefixLength = netmask.getNetmaskLength();
        }
        else {
            prefixLength = atoi(prefixLengthStr);
            if (prefixLength < 0)
                throw cRuntimeError("Invalid policy at %s: 'prefixLength' must be >= 0, but is '%s'", entry.getSourceLocation(), prefixLengthStr);
        }

        PolicyEntry newEntry;
        newEntry.dest = dest;
        newEntry.prefixLength = prefixLength;

        const char *segmentsStr = entry.getAttribute("segments");
        if (!segmentsStr || !*segmentsStr)
            throw cRuntimeError("Invalid policy at %s: missing or empty 'segments' attribute", entry.getSourceLocation());

        cStringTokenizer tokenizer(segmentsStr);
        while (const char *token = tokenizer.nextToken()) {
            const char *colon = strchr(token, ':');
            if (!colon)
                throw cRuntimeError("Invalid segment '%s' in policy at %s: expected 'node:<sid>' or 'adj:<interface>'", token, entry.getSourceLocation());

            std::string kind(token, colon - token);
            std::string value(colon + 1);
            if (value.empty())
                throw cRuntimeError("Invalid segment '%s' in policy at %s: missing value after ':'", token, entry.getSourceLocation());

            Segment seg;
            if (kind == "node") {
                seg.kind = Segment::NODE;
                int sidIndex = atoi(value.c_str());

                // Resolve which router currently owns this sid, and cache the (static,
                // homogeneous-SRGB) label value -- both once, here, at config time; see
                // SegmentRouting::getSidByRouter()'s doc for why SrPolicy reads this rather than
                // duplicating sidTable parsing/config.
                bool found = false;
                for (auto& pair : sr->getSidByRouter()) {
                    if (pair.second == sidIndex) {
                        seg.nodeRouter = pair.first;
                        found = true;
                        break;
                    }
                }
                if (!found)
                    throw cRuntimeError("Invalid segment '%s' in policy at %s: no router is currently configured with node-SID index %d",
                            token, entry.getSourceLocation(), sidIndex);
                seg.label = sr->getSrgbBase() + sidIndex;
            }
            else if (kind == "adj") {
                seg.kind = Segment::ADJ;
                NetworkInterface *ie = ift->findInterfaceByName(value.c_str());
                if (!ie)
                    throw cRuntimeError("Invalid segment '%s' in policy at %s: interface '%s' not registered by any interface",
                            token, entry.getSourceLocation(), value.c_str());
                seg.interfaceId = ie->getInterfaceId();
            }
            else
                throw cRuntimeError("Invalid segment '%s' in policy at %s: unknown kind '%s' (expected 'node' or 'adj')",
                        token, entry.getSourceLocation(), kind.c_str());

            newEntry.segments.push_back(seg);
        }

        if (newEntry.segments.empty())
            throw cRuntimeError("Invalid policy at %s: 'segments' did not contain any segment", entry.getSourceLocation());

        policyTable.push_back(newEntry);
    }
}

std::ostream& operator<<(std::ostream& os, const SrPolicy::PolicyEntry& policy)
{
    os << "dest:" << policy.dest;
    os << "    prefixLength:" << policy.prefixLength;
    os << "    segments:" << policy.segments.size();
    return os;
}

} // namespace inet
