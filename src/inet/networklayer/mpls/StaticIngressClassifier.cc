//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/mpls/StaticIngressClassifier.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/XMLUtils.h"
#include "inet/networklayer/common/L3Tools.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/networklayer/mpls/LibTable.h"

namespace inet {

Define_Module(StaticIngressClassifier);

using namespace xmlutils;

void StaticIngressClassifier::initialize(int stage)
{
    SimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        ift.reference(this, "interfaceTableModule", true);
        WATCH(fecTable);
    }
    else if (stage == INITSTAGE_NETWORK_LAYER) {
        // by this stage all interfaces are registered in ift (mirrors LibTable's own timing)
        readTableFromXML(par("config"));
    }
}

void StaticIngressClassifier::handleMessage(cMessage *msg)
{
    throw cRuntimeError("StaticIngressClassifier does not process messages, but received '%s'", msg->getName());
}

// IIngressClassifier implementation (method invoked by MPLS)

bool StaticIngressClassifier::lookupLabel(Packet *packet, LabelOpVector& outLabel, int& outInterfaceId)
{
    // Dual-stack (Workstream F3): the packet's native protocol (ipv4 or ipv6, both offered to
    // classifiers by Mpls -- see Mpls::processPacketFromL3/processPacketFromL2) is already
    // recorded on its PacketProtocolTag; peekNetworkProtocolHeader() dispatches to the right
    // concrete header type and returns it through the common NetworkHeaderBase interface, so
    // getDestinationAddress() yields a generic L3Address without this classifier needing its
    // own per-family dispatch.
    const Protocol *protocol = packet->getTag<PacketProtocolTag>()->getProtocol();
    L3Address destAddress = peekNetworkProtocolHeader(packet, *protocol)->getDestinationAddress();

    // longest-prefix match: the entry with the most specific (longest) prefix length wins.
    // fecTable may hold a MIX of IPv4 and IPv6 entries (this classifier sees both families --
    // Mpls offers every native packet to it, see the comment above); L3Address::matches()
    // dispatches on ITS OWN type and unconditionally converts the argument to that same
    // family, so it must never be called across families (it would throw) -- skip entries
    // whose family doesn't match the packet's before calling it.
    const FecEntry *best = nullptr;
    for (auto& entry : fecTable) {
        if (entry.dest.getType() != destAddress.getType())
            continue;
        if (!destAddress.matches(entry.dest, entry.prefixLength))
            continue;

        if (!best || entry.prefixLength > best->prefixLength)
            best = &entry;
    }

    if (!best)
        return false;

    outLabel = LibTable::pushLabel(best->label);
    outInterfaceId = best->outInterfaceId;
    return true;
}

// static configuration

void StaticIngressClassifier::readTableFromXML(const cXMLElement *fectable)
{
    ASSERT(fectable);
    ASSERT(!strcmp(fectable->getTagName(), "fectable"));
    checkTags(fectable, "fecentry");
    cXMLElementList list = fectable->getChildrenByTagName("fecentry");
    for (auto& elem : list) {
        const cXMLElement& entry = *elem;

        const char *destStr = entry.getAttribute("dest");
        if (!destStr)
            throw cRuntimeError("Invalid fecentry at %s: missing 'dest' attribute", entry.getSourceLocation());
        L3Address dest;
        if (!dest.tryParse(destStr))
            throw cRuntimeError("Invalid fecentry at %s: 'dest' is not a valid IPv4/IPv6 address: '%s'", entry.getSourceLocation(), destStr);

        // Dual-stack (Workstream F3): exactly one of the old 'netmask' (IPv4 dotted, kept for
        // backward compatibility -- e.g. examples/mpls/sr/policies.xml's sibling SrPolicy format)
        // or the new 'prefixLength' (a plain integer, usable for either family, mandatory for an
        // IPv6 dest since IPv6 has no netmask-string convention) attribute may be given.
        const char *netmaskStr = entry.getAttribute("netmask");
        const char *prefixLengthStr = entry.getAttribute("prefixLength");
        if (netmaskStr && prefixLengthStr)
            throw cRuntimeError("Invalid fecentry at %s: 'netmask' and 'prefixLength' are mutually exclusive, but both were given", entry.getSourceLocation());
        if (!netmaskStr && !prefixLengthStr)
            throw cRuntimeError("Invalid fecentry at %s: missing 'netmask' or 'prefixLength' attribute", entry.getSourceLocation());

        int prefixLength;
        if (netmaskStr) {
            if (!Ipv4Address::isWellFormed(netmaskStr))
                throw cRuntimeError("Invalid fecentry at %s: 'netmask' is not a valid IPv4 address: '%s'", entry.getSourceLocation(), netmaskStr);
            Ipv4Address netmask(netmaskStr);
            if (!netmask.isValidNetmask())
                throw cRuntimeError("Invalid fecentry at %s: 'netmask' is not a valid netmask (non-contiguous bits): '%s'", entry.getSourceLocation(), netmaskStr);
            prefixLength = netmask.getNetmaskLength();
        }
        else {
            prefixLength = atoi(prefixLengthStr);
            if (prefixLength < 0)
                throw cRuntimeError("Invalid fecentry at %s: 'prefixLength' must be >= 0, but is '%s'", entry.getSourceLocation(), prefixLengthStr);
        }

        FecEntry newEntry;
        newEntry.dest = dest;
        newEntry.prefixLength = prefixLength;

        const char *labelStr = entry.getAttribute("label");
        if (!labelStr)
            throw cRuntimeError("Invalid fecentry at %s: missing 'label' attribute", entry.getSourceLocation());
        newEntry.label = atoi(labelStr);
        if (newEntry.label < 0)
            throw cRuntimeError("Invalid fecentry at %s: 'label' must be >= 0, but is '%s'", entry.getSourceLocation(), labelStr);

        const char *interfaceName = entry.getAttribute("interface");
        if (!interfaceName)
            throw cRuntimeError("Invalid fecentry at %s: missing 'interface' attribute", entry.getSourceLocation());
        NetworkInterface *outIe = ift->findInterfaceByName(interfaceName);
        if (!outIe)
            throw cRuntimeError("Invalid fecentry at %s: interface '%s' not registered by any interface", entry.getSourceLocation(), interfaceName);
        newEntry.outInterfaceId = outIe->getInterfaceId();

        fecTable.push_back(newEntry);
    }
}

std::ostream& operator<<(std::ostream& os, const StaticIngressClassifier::FecEntry& fec)
{
    os << "dest:" << fec.dest;
    os << "    prefixLength:" << fec.prefixLength;
    os << "    label:" << fec.label;
    os << "    outInterfaceId:" << fec.outInterfaceId;
    return os;
}

} // namespace inet
