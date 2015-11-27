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

#include "inet/common/INETDefs.h"

#include "inet/networklayer/ted/LinkStateRouting.h"
#include "inet/networklayer/common/IPSocket.h"
#include "inet/networklayer/contract/ipv4/IPv4ControlInfo.h"
#include "inet/networklayer/ipv4/IPv4InterfaceData.h"
#include "inet/common/NotifierConsts.h"
#include "inet/networklayer/ipv4/IIPv4RoutingTable.h"
#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ted/TED.h"

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

    if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        tedmod = getModuleFromPar<TED>(par("tedModule"), this);

        IIPv4RoutingTable *rt = getModuleFromPar<IIPv4RoutingTable>(par("routingTableModule"), this);
        routerId = rt->getRouterId();

        // listen for TED modifications
        cModule *host = getContainingNode(this);
        host->subscribe(NF_TED_CHANGED, this);

        // peers are given as interface names in the "peers" module parameter;
        // store corresponding interface addresses in peerIfAddrs[]
        cStringTokenizer tokenizer(par("peers"));
        IInterfaceTable *ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        const char *token;
        while ((token = tokenizer.nextToken()) != nullptr) {
            ASSERT(ift->getInterfaceByName(token));
            peerIfAddrs.push_back(ift->getInterfaceByName(token)->ipv4Data()->getIPAddress());
        }

        // schedule start of flooding link state info
        announceMsg = new cMessage("announce");
        scheduleAt(simTime() + exponential(0.01), announceMsg);

        IPSocket socket(gate("ipOut"));
        socket.registerProtocol(IP_PROT_OSPF);
    }
}

void LinkStateRouting::handleMessage(cMessage *msg)
{
    if (msg == announceMsg) {
        delete announceMsg;
        announceMsg = nullptr;
        sendToPeers(tedmod->ted, true, IPv4Address());
    }
    else if (!strcmp(msg->getArrivalGate()->getName(), "ipIn")) {
        EV_INFO << "Processing message from IPv4: " << msg << endl;
        IPv4ControlInfo *controlInfo = check_and_cast<IPv4ControlInfo *>(msg->getControlInfo());
        IPv4Address sender = controlInfo->getSrcAddr();
        processLINK_STATE_MESSAGE(check_and_cast<LinkStateMsg *>(msg), sender);
    }
    else
        ASSERT(false);
}

void LinkStateRouting::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj DETAILS_ARG)
{
    Enter_Method_Silent();
    printNotificationBanner(signalID, obj);

    ASSERT(signalID == NF_TED_CHANGED);

    EV_INFO << "TED changed\n";

    const TEDChangeInfo *d = check_and_cast<const TEDChangeInfo *>(obj);

    unsigned int k = d->getTedLinkIndicesArraySize();

    ASSERT(k > 0);

    // build linkinfo list
    std::vector<TELinkStateInfo> links;
    for (unsigned int i = 0; i < k; i++) {
        unsigned int index = d->getTedLinkIndices(i);

        tedmod->updateTimestamp(&tedmod->ted[index]);
        links.push_back(tedmod->ted[index]);
    }

    sendToPeers(links, false, IPv4Address());
}

void LinkStateRouting::processLINK_STATE_MESSAGE(LinkStateMsg *msg, IPv4Address sender)
{
    EV_INFO << "received LINK_STATE message from " << sender << endl;

    TELinkStateInfoVector forward;

    unsigned int n = msg->getLinkInfoArraySize();

    bool change = false;    // in topology

    // loop through every link in the message
    for (unsigned int i = 0; i < n; i++) {
        const TELinkStateInfo& link = msg->getLinkInfo(i);

        TELinkStateInfo *match;

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

    delete msg;
}

void LinkStateRouting::sendToPeers(const std::vector<TELinkStateInfo>& list, bool req, IPv4Address exceptPeer)
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

void LinkStateRouting::sendToPeer(IPv4Address peer, const std::vector<TELinkStateInfo>& list, bool req)
{
    EV_INFO << "sending LINK_STATE message to " << peer << endl;

    LinkStateMsg *out = new LinkStateMsg("link state");

    out->setLinkInfoArraySize(list.size());
    for (unsigned int j = 0; j < list.size(); j++)
        out->setLinkInfo(j, list[j]);

    out->setRequest(req);

    sendToIP(out, peer);
}

void LinkStateRouting::sendToIP(LinkStateMsg *msg, IPv4Address destAddr)
{
    // attach control info to packet
    IPv4ControlInfo *controlInfo = new IPv4ControlInfo();
    controlInfo->setDestAddr(destAddr);
    controlInfo->setSrcAddr(routerId);
    controlInfo->setProtocol(IP_PROT_OSPF);
    msg->setControlInfo(controlInfo);

    int length = msg->getLinkInfoArraySize() * 72;
    msg->setByteLength(length);

    msg->addPar("color") = TED_TRAFFIC;

    send(msg, "ipOut");
}

} // namespace inet

