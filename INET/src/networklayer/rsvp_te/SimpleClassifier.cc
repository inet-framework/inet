//
// (C) 2005 Vojtech Janota
//
// This library is free software, you can redistribute it
// and/or modify
// it under  the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation;
// either version 2 of the License, or any later version.
// The library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//

#include <iostream>
#include "SimpleClassifier.h"
#include "XMLUtils.h"
#include "RoutingTableAccess.h"
#include "LIBTableAccess.h"
#include "RSVPAccess.h"
#include "LIBTable.h"

Define_Module(SimpleClassifier);

void SimpleClassifier::initialize(int stage)
{
    // we have to wait until routerId gets assigned in stage 3
    if (stage!=4)
        return;

    maxLabel = 0;

    RoutingTableAccess routingTableAccess;
    IRoutingTable *rt = routingTableAccess.get();
    routerId = rt->getRouterId();

    LIBTableAccess libTableAccess;
    lt = libTableAccess.get();

    RSVPAccess rsvpAccess;
    rsvp = rsvpAccess.get();

    readTableFromXML(par("conf").xmlValue());

    WATCH_VECTOR(bindings);
}

void SimpleClassifier::handleMessage(cMessage *)
{
    ASSERT(false);
}

// IClassifier implementation (method invoked by MPLS)

bool SimpleClassifier::lookupLabel(IPDatagram *ipdatagram, LabelOpVector& outLabel, std::string& outInterface, int& color)
{
    // never label OSPF(TED) and RSVP traffic

    switch(ipdatagram->getTransportProtocol())
    {
        case IP_PROT_OSPF:
        case IP_PROT_RSVP:
            return false;

        default:
            ;
    }

    // forwarding decision for non-labeled datagrams

    std::vector<FECEntry>::iterator it;
    for (it = bindings.begin(); it != bindings.end(); it++)
    {
        if (!it->dest.isUnspecified() && !it->dest.equals(ipdatagram->getDestAddress()))
            continue;

        if (!it->src.isUnspecified() && !it->src.equals(ipdatagram->getSrcAddress()))
            continue;

        EV << "packet belongs to fecid=" << it->id << endl;

        if (it->inLabel < 0)
            return false;

        return lt->resolveLabel("", it->inLabel, outLabel, outInterface, color);
    }

    return false;
}

// IRSVPClassifier implementation (method invoked by RSVP)

void SimpleClassifier::bind(const SessionObj_t& session, const SenderTemplateObj_t& sender, int inLabel)
{
    std::vector<FECEntry>::iterator it;
    for (it = bindings.begin(); it != bindings.end(); it++)
    {
        if (it->session != session)
            continue;

        if (it->sender != sender)
            continue;

        it->inLabel = inLabel;
    }
}

// IScriptable implementation (method invoked by ScenarioManager)

void SimpleClassifier::processCommand(const cXMLElement& node)
{
    if (!strcmp(node.getTagName(), "bind-fec"))
    {
        readItemFromXML(&node);
    }
    else
        ASSERT(false);
}


// binding configuration

void SimpleClassifier::readTableFromXML(const cXMLElement *fectable)
{
    ASSERT(fectable);
    ASSERT(!strcmp(fectable->getTagName(), "fectable"));
    checkTags(fectable, "fecentry");
    cXMLElementList list = fectable->getChildrenByTagName("fecentry");
    for (cXMLElementList::iterator it=list.begin(); it != list.end(); it++)
        readItemFromXML(*it);
}

void SimpleClassifier::readItemFromXML(const cXMLElement *fec)
{
    ASSERT(fec);
    ASSERT(!strcmp(fec->getTagName(), "fecentry") || !strcmp(fec->getTagName(), "bind-fec"));

    int fecid = getParameterIntValue(fec, "id");

    std::vector<FECEntry>::iterator it = findFEC(fecid);

    if (getUniqueChildIfExists(fec, "label"))
    {
        // bind-fec to label
        checkTags(fec, "id label destination source");

        EV << "binding to a given label" << endl;

        FECEntry newFec;

        newFec.id = fecid;
        newFec.dest = getParameterIPAddressValue(fec, "destination");
        newFec.src = getParameterIPAddressValue(fec, "source", IPAddress());

        newFec.inLabel = getParameterIntValue(fec, "label");

        if (it == bindings.end())
        {
            // create new binding
            bindings.push_back(newFec);
        }
        else
        {
            // update existing binding
            *it = newFec;
        }
    }
    else if (getUniqueChildIfExists(fec, "lspid"))
    {
        // bind-fec to LSP
        checkTags(fec, "id destination source tunnel_id extended_tunnel_id endpoint lspid");

        EV << "binding to a given path" << endl;

        FECEntry newFec;

        newFec.id = fecid;
        newFec.dest = getParameterIPAddressValue(fec, "destination");
        newFec.src = getParameterIPAddressValue(fec, "source", IPAddress());

        newFec.session.Tunnel_Id = getParameterIntValue(fec, "tunnel_id");
        newFec.session.Extended_Tunnel_Id = getParameterIPAddressValue(fec, "extened_tunnel_id", routerId).getInt();
        newFec.session.DestAddress = getParameterIPAddressValue(fec, "endpoint", newFec.dest); // ??? always use newFec.dest ???

        newFec.sender.Lsp_Id = getParameterIntValue(fec, "lspid");
        newFec.sender.SrcAddress = routerId;

        newFec.inLabel = rsvp->getInLabel(newFec.session, newFec.sender);

        if (it == bindings.end())
        {
            // create new binding
            bindings.push_back(newFec);
        }
        else
        {
            // update existing binding
            *it = newFec;
        }
    }
    else
    {
        // un-bind
        checkTags(fec, "id");

        if (it != bindings.end())
        {
            bindings.erase(it);
        }
    }
}

std::vector<SimpleClassifier::FECEntry>::iterator SimpleClassifier::findFEC(int fecid)
{
    std::vector<FECEntry>::iterator it;
    for (it = bindings.begin(); it != bindings.end(); it++)
    {
        if (it->id != fecid)
            continue;

        break;
    }
    return it;
}

std::ostream& operator<<(std::ostream& os, const SimpleClassifier::FECEntry& fec)
{
    os << "id:" << fec.id;
    os << "    dest:" << fec.dest;
    os << "    src:" << fec.src;
    os << "    session:" << fec.session;
    os << "    sender:" << fec.sender;
    os << "    inLabel:" << fec.inLabel;
    return os;
}
