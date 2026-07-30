//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/mpls/StaticIngressClassifier.h"

#include "inet/common/XMLUtils.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
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
    const auto& ipv4Header = packet->peekAtFront<Ipv4Header>();
    Ipv4Address destAddress = ipv4Header->getDestAddress();

    // longest-prefix match: the entry with the most specific (longest) netmask wins
    const FecEntry *best = nullptr;
    for (auto& entry : fecTable) {
        if (!Ipv4Address::maskedAddrAreEqual(destAddress, entry.dest, entry.netmask))
            continue;

        if (!best || entry.netmask.getNetmaskLength() > best->netmask.getNetmaskLength())
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
        if (!Ipv4Address::isWellFormed(destStr))
            throw cRuntimeError("Invalid fecentry at %s: 'dest' is not a valid IPv4 address: '%s'", entry.getSourceLocation(), destStr);

        const char *netmaskStr = entry.getAttribute("netmask");
        if (!netmaskStr)
            throw cRuntimeError("Invalid fecentry at %s: missing 'netmask' attribute", entry.getSourceLocation());
        if (!Ipv4Address::isWellFormed(netmaskStr))
            throw cRuntimeError("Invalid fecentry at %s: 'netmask' is not a valid IPv4 address: '%s'", entry.getSourceLocation(), netmaskStr);

        FecEntry newEntry;
        newEntry.dest = Ipv4Address(destStr);
        newEntry.netmask = Ipv4Address(netmaskStr);
        if (!newEntry.netmask.isValidNetmask())
            throw cRuntimeError("Invalid fecentry at %s: 'netmask' is not a valid netmask (non-contiguous bits): '%s'", entry.getSourceLocation(), netmaskStr);

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
    os << "    netmask:" << fec.netmask;
    os << "    label:" << fec.label;
    os << "    outInterfaceId:" << fec.outInterfaceId;
    return os;
}

} // namespace inet
