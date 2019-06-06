//
// (C) 2005 Vojtech Janota, Andras Varga
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

#include <algorithm>

#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/Simsignals.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/networklayer/ted/LinkStateRouting.h"
#include "inet/networklayer/ted/Ted.h"

namespace inet {

Define_Module(LinkStateRouting);

LinkStateRouting::LinkStateRouting()
{
}

LinkStateRouting::~LinkStateRouting()
{
    cancelAndDelete(announceMsg);
}

void LinkStateRouting::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    // TODO: INITSTAGE
    if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        tedmod = getModuleFromPar<Ted>(par("tedModule"), this);

        IIpv4RoutingTable *rt = getModuleFromPar<IIpv4RoutingTable>(par("routingTableModule"), this);
        routerId = rt->getRouterId();

        // listen for TED modifications
        cModule *host = getContainingNode(this);
        host->subscribe(tedChangedSignal, this);

        // peers are given as interface names in the "peers" module parameter;
        // store corresponding interface addresses in peerIfAddrs[]
        cStringTokenizer tokenizer(par("peers"));
        IInterfaceTable *ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        const char *token;
        while ((token = tokenizer.nextToken()) != nullptr) {
            peerIfAddrs.push_back(CHK(ift->findInterfaceByName(token))->getProtocolData<Ipv4InterfaceData>()->getIPAddress());
        }

        // schedule start of flooding link state info
        announceMsg = new cMessage("announce");
        scheduleAt(simTime() + exponential(0.01), announceMsg);
        registerService(Protocol::linkStateRouting, nullptr, gate("ipIn"));
        registerProtocol(Protocol::linkStateRouting, gate("ipOut"), nullptr);
    }
}

void LinkStateRouting::handleMessage(cMessage *msg)
{
    if (msg == announceMsg) {
        delete announceMsg;
        announceMsg = nullptr;
        sendToPeers(tedmod->ted, true, Ipv4Address());
    }
    else if (!strcmp(msg->getArrivalGate()->getName(), "ipIn")) {
        EV_INFO << "Processing message from Ipv4: " << msg << endl;
        Ipv4Address sender = check_and_cast<Packet *>(msg)->getTag<L3AddressInd>()->getSrcAddress().toIpv4();
        processLINK_STATE_MESSAGE(check_and_cast<Packet *>(msg), sender);
    }
    else
        ASSERT(false);
}

void LinkStateRouting::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method_Silent();
    printSignalBanner(signalID, obj, details);

    ASSERT(signalID == tedChangedSignal);

    EV_INFO << "TED changed\n";

    const TedChangeInfo *d = check_and_cast<const TedChangeInfo *>(obj);

    unsigned int k = d->getTedLinkIndicesArraySize();

    ASSERT(k > 0);

    // build linkinfo list
    std::vector<TeLinkStateInfo> links;
    for (unsigned int i = 0; i < k; i++) {
        unsigned int index = d->getTedLinkIndices(i);

        tedmod->updateTimestamp(&tedmod->ted[index]);
        links.push_back(tedmod->ted[index]);
    }

    sendToPeers(links, false, Ipv4Address());
}

void LinkStateRouting::processLINK_STATE_MESSAGE(Packet *pk, Ipv4Address sender)
{
    EV_INFO << "received LINK_STATE message from " << sender << endl;

    const auto& msg = pk->peekAtFront<LinkStateMsg>();
    TeLinkStateInfoVector forward;

    unsigned int n = msg->getLinkInfoArraySize();

    bool change = false;    // in topology

    // loop through every link in the message
    for (unsigned int i = 0; i < n; i++) {
        const TeLinkStateInfo& link = msg->getLinkInfo(i);

        TeLinkStateInfo *match;

        // process link if we haven't seen this already and timestamp is newer
        if (tedmod->checkLinkValidity(link, match)) {
            ASSERT(link.sourceId == link.advrouter.getInt());

            EV_INFO << "new information found" << endl;

            if (!match) {
                // and we have no info on this link so far, store it as it is
                tedmod->ted.push_back(link);
                change = true;
            }
            else {
                // copy over the information from it
                if (match->state != link.state) {
                    match->state = link.state;
                    change = true;
                }
                match->messageId = link.messageId;
                match->sourceId = link.sourceId;
                match->timestamp = link.timestamp;
                for (int i = 0; i < 8; i++)
                    match->UnResvBandwidth[i] = link.UnResvBandwidth[i];
                match->MaxBandwidth = link.MaxBandwidth;
                match->metric = link.metric;
            }

            forward.push_back(link);
        }
    }

    if (change)
        tedmod->rebuildRoutingTable();

    if (msg->getRequest()) {
        sendToPeer(sender, tedmod->ted, false);
    }

    if (forward.size() > 0) {
        sendToPeers(forward, false, sender);
    }

    delete pk;
}

void LinkStateRouting::sendToPeers(const std::vector<TeLinkStateInfo>& list, bool req, Ipv4Address exceptPeer)
{
    EV_INFO << "sending LINK_STATE message to peers" << endl;

    // send "list" to every peer (linkid in our ted[] entries???) in a LinkStateMsg
    for (auto & elem : tedmod->ted) {
        if (elem.advrouter != routerId)
            continue;

        if (elem.linkid == exceptPeer)
            continue;

        if (!elem.state)
            continue;

        if (find(peerIfAddrs.begin(), peerIfAddrs.end(), elem.local) == peerIfAddrs.end())
            continue;

        // send a copy
        sendToPeer(elem.linkid, list, req);
    }
}

void LinkStateRouting::sendToPeer(Ipv4Address peer, const std::vector<TeLinkStateInfo>& list, bool req)
{
    EV_INFO << "sending LINK_STATE message to " << peer << endl;

    Packet *pk = new Packet("link state");
    const auto& out = makeShared<LinkStateMsg>();

    out->setLinkInfoArraySize(list.size());
    for (unsigned int j = 0; j < list.size(); j++)
        out->setLinkInfo(j, list[j]);

    out->setRequest(req);
    //B length = B(72) * out->getLinkInfoArraySize();
    //B length = B(113 * out->getLinkInfoArraySize()) + B(6) + B(25);
    B length = B(113 * out->getLinkInfoArraySize()) + B(6);
    out->setChunkLength(length);
    pk->insertAtBack(out);

    sendToIP(pk, peer);
}

void LinkStateRouting::sendToIP(Packet *msg, Ipv4Address destAddr)
{
    msg->addPar("color") = TED_TRAFFIC;

    msg->addTagIfAbsent<PacketProtocolTag>()->setProtocol(&Protocol::linkStateRouting);
    msg->addTagIfAbsent<DispatchProtocolInd>()->setProtocol(&Protocol::linkStateRouting);
    msg->addTagIfAbsent<DispatchProtocolReq>()->setProtocol(&Protocol::ipv4);
    msg->addTagIfAbsent<L3AddressReq>()->setDestAddress(destAddr);
    msg->addTagIfAbsent<L3AddressReq>()->setSrcAddress(routerId);
    send(msg, "ipOut");
}

} // namespace inet

