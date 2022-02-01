//
// Copyright (C) 2005 Vojtech Janota
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/rsvpte/RsvpClassifier.h"

#include <iostream>

#include "inet/common/ModuleAccess.h"
#include "inet/common/XMLUtils.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"
#include "inet/networklayer/mpls/LibTable.h"
#include "inet/networklayer/rsvpte/RsvpTe.h"

namespace inet {

Define_Module(RsvpClassifier);

using namespace xmlutils;

void RsvpClassifier::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        maxLabel = 0;
        WATCH_VECTOR(bindings);
    }
    // TODO INITSTAGE
    else if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        IIpv4RoutingTable *rt = getModuleFromPar<IIpv4RoutingTable>(par("routingTableModule"), this);
        routerId = rt->getRouterId();

        lt.reference(this, "libTableModule", true);
        rsvp.reference(this, "rsvpModule", true);

        readTableFromXML(par("config"));
    }
}

void RsvpClassifier::handleMessage(cMessage *)
{
    ASSERT(false);
}

// IIngressClassifier implementation (method invoked by MPLS)

bool RsvpClassifier::lookupLabel(Packet *packet, LabelOpVector& outLabel, std::string& outInterface, int& color)
{
    // never label OSPF(TED) and RSVP traffic
    const auto& ipv4Header = packet->peekAtFront<Ipv4Header>();

    switch (ipv4Header->getProtocolId()) {
        case IP_PROT_OSPF:
        case IP_PROT_RSVP:
            return false;

        default:
            break;
    }

    // forwarding decision for non-labeled datagrams

    for (auto& elem : bindings) {
        if (!elem.dest.isUnspecified() && !elem.dest.equals(ipv4Header->getDestAddress()))
            continue;

        if (!elem.src.isUnspecified() && !elem.src.equals(ipv4Header->getSrcAddress()))
            continue;

        EV_DETAIL << "packet belongs to fecid=" << elem.id << endl;

        if (elem.inLabel < 0)
            return false;

        return lt->resolveLabel("", elem.inLabel, outLabel, outInterface, color);
    }

    return false;
}

// IRsvpClassifier implementation (method invoked by RSVP)

void RsvpClassifier::bind(const SessionObj& session, const SenderTemplateObj& sender, int inLabel)
{
    for (auto& elem : bindings) {
        if (elem.session != session)
            continue;

        if (elem.sender != sender)
            continue;

        elem.inLabel = inLabel;
    }
}

// IScriptable implementation (method invoked by ScenarioManager)

void RsvpClassifier::processCommand(const cXMLElement& node)
{
    if (!strcmp(node.getTagName(), "bind-fec")) {
        readItemFromXML(&node);
    }
    else
        ASSERT(false);
}

// binding configuration

void RsvpClassifier::readTableFromXML(const cXMLElement *fectable)
{
    ASSERT(fectable);
    ASSERT(!strcmp(fectable->getTagName(), "fectable"));
    checkTags(fectable, "fecentry");
    cXMLElementList list = fectable->getChildrenByTagName("fecentry");
    for (auto& elem : list)
        readItemFromXML(elem);
}

void RsvpClassifier::readItemFromXML(const cXMLElement *fec)
{
    ASSERT(fec);
    ASSERT(!strcmp(fec->getTagName(), "fecentry") || !strcmp(fec->getTagName(), "bind-fec"));

    int fecid = getParameterIntValue(fec, "id");

    auto it = findFEC(fecid);

    if (getUniqueChildIfExists(fec, "label")) {
        // bind-fec to label
        checkTags(fec, "id label destination source");

        EV_INFO << "binding to a given label" << endl;

        FecEntry newFec;

        newFec.id = fecid;
        newFec.dest = getParameterIPAddressValue(fec, "destination");
        newFec.src = getParameterIPAddressValue(fec, "source", Ipv4Address());

        newFec.inLabel = getParameterIntValue(fec, "label");

        if (it == bindings.end()) {
            // create new binding
            bindings.push_back(newFec);
        }
        else {
            // update existing binding
            *it = newFec;
        }
    }
    else if (getUniqueChildIfExists(fec, "lspid")) {
        // bind-fec to LSP
        checkTags(fec, "id destination source tunnel_id extended_tunnel_id endpoint lspid");

        EV_INFO << "binding to a given path" << endl;

        FecEntry newFec;

        newFec.id = fecid;
        newFec.dest = getParameterIPAddressValue(fec, "destination");
        newFec.src = getParameterIPAddressValue(fec, "source", Ipv4Address());

        newFec.session.Tunnel_Id = getParameterIntValue(fec, "tunnel_id");
        newFec.session.Extended_Tunnel_Id = getParameterIPAddressValue(fec, "extened_tunnel_id", routerId).getInt();
        newFec.session.DestAddress = getParameterIPAddressValue(fec, "endpoint", newFec.dest); // ??? always use newFec.dest ???

        newFec.sender.Lsp_Id = getParameterIntValue(fec, "lspid");
        newFec.sender.SrcAddress = routerId;

        newFec.inLabel = rsvp->getInLabel(newFec.session, newFec.sender);

        if (it == bindings.end()) {
            // create new binding
            bindings.push_back(newFec);
        }
        else {
            // update existing binding
            *it = newFec;
        }
    }
    else {
        // un-bind
        checkTags(fec, "id");

        if (it != bindings.end()) {
            bindings.erase(it);
        }
    }
}

std::vector<RsvpClassifier::FecEntry>::iterator RsvpClassifier::findFEC(int fecid)
{
    auto it = bindings.begin();
    for (; it != bindings.end(); it++) {
        if (it->id == fecid)
            break;
    }
    return it;
}

std::ostream& operator<<(std::ostream& os, const RsvpClassifier::FecEntry& fec)
{
    os << "id:" << fec.id;
    os << "    dest:" << fec.dest;
    os << "    src:" << fec.src;
    os << "    session:" << fec.session;
    os << "    sender:" << fec.sender;
    os << "    inLabel:" << fec.inLabel;
    return os;
}

} // namespace inet

