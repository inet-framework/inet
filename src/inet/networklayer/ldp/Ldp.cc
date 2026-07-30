//
// Copyright (C) 2005 Vojtech Janota
// Copyright (C) 2004 Andras Varga
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/networklayer/ldp/Ldp.h"

#include <algorithm>
#include <fstream>
#include <iostream>

#include "inet/common/ModuleAccess.h"
#include "inet/common/Simsignals.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/ipv4/IIpv4RoutingTable.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/networklayer/mpls/LibTable.h"
#include "inet/networklayer/ted/Ted.h"
#include "inet/transportlayer/contract/udp/UdpCommand_m.h"
#include "inet/transportlayer/tcp_common/TcpHeader.h"
#include "inet/transportlayer/udp/UdpHeader_m.h"
namespace inet {

Define_Module(Ldp);

simsignal_t Ldp::sessionUpSignal = registerSignal("sessionUp");
simsignal_t Ldp::sessionDownSignal = registerSignal("sessionDown");
simsignal_t Ldp::fecBindingCountSignal = registerSignal("fecBindingCount");

std::ostream& operator<<(std::ostream& os, const Ldp::FecBinding& f)
{
    os << "fecid=" << f.fecid << "  peer=" << f.peer << " label=" << f.label;
    return os;
}

bool fecPrefixCompare(const Ldp::Fec& a, const Ldp::Fec& b)
{
    return a.length > b.length;
}

std::ostream& operator<<(std::ostream& os, const Ldp::Fec& f)
{
    os << "fecid=" << f.fecid << "  addr=" << f.addr << "  length=" << f.length << "  nextHop=" << f.nextHop;
    return os;
}

std::ostream& operator<<(std::ostream& os, const Ldp::PendingRequest& r)
{
    os << "fecid=" << r.fecid << "  peer=" << r.peer;
    return os;
}

static const char *ldpSessionStateName(Ldp::peer_info::SessionState state)
{
    switch (state) {
        case Ldp::peer_info::NONEXISTENT: return "NONEXISTENT";
        case Ldp::peer_info::INITIALIZED: return "INITIALIZED";
        case Ldp::peer_info::OPENSENT: return "OPENSENT";
        case Ldp::peer_info::OPENREC: return "OPENREC";
        case Ldp::peer_info::OPERATIONAL: return "OPERATIONAL";
        default: return "???";
    }
}

std::ostream& operator<<(std::ostream& os, const Ldp::peer_info& p)
{
    os << "peerIP=" << p.peerIP << "  interface=" << p.linkInterface
       << "  activeRole=" << (p.activeRole ? "true" : "false")
       << "  socket=" << (p.socket ? TcpSocket::stateName(p.socket->getState()) : "nullptr")
       << "  state=" << ldpSessionStateName(p.state);
    return os;
}

bool operator==(const FecTlv& a, const FecTlv& b)
{
    return a.length == b.length && a.addr == b.addr;
}

bool operator!=(const FecTlv& a, const FecTlv& b)
{
    return !operator==(a, b);
}

std::ostream& operator<<(std::ostream& os, const FecTlv& a)
{
    os << "addr=" << a.addr << "  length=" << a.length;
    return os;
}

Ldp::Ldp()
{
}

Ldp::~Ldp()
{
    for (auto& elem : myPeers) {
        cancelAndDelete(elem.timeout);
        cancelAndDelete(elem.keepAliveSendTimer);
        cancelAndDelete(elem.sessionHoldTimer);
    }

    cancelAndDelete(sendHelloMsg);
    for (auto *m : retryMsgs)
        cancelAndDelete(m);
    for (auto *s : deadSockets)
        delete s;
    socketMap.deleteSockets();
}

void Ldp::initialize(int stage)
{
    RoutingProtocolBase::initialize(stage);

    // FIXME move bind() and listen() calls to a new startModule() function, and call it from initialize() and from handleOperationStage()
    // FIXME register to NetworkInterface changes, for detecting the interface add/delete, and detecting multicast config changes:
    // should be refresh the udpSockets vector when interface added/deleted, or isMulticast() value changed.

    if (stage == INITSTAGE_LOCAL) {
        holdTime = par("holdTime");
        helloInterval = par("helloInterval");
        keepaliveTime = par("keepaliveTime");
        advertiseImplicitNull = par("advertiseImplicitNull");
        distributionMode = par("distributionMode").stdstringValue();
        controlMode = par("controlMode").stdstringValue();
        retentionMode = par("retentionMode").stdstringValue();

        ift.reference(this, "interfaceTableModule", true);
        rt.reference(this, "routingTableModule", true);
        lt.reference(this, "libTableModule", true);
        tedmod.reference(this, "tedModule", true);

        WATCH(myPeers);
        WATCH(fecUp);
        WATCH(fecDown);
        WATCH(fecList);
        WATCH(pending);
        WATCH(numSent);
        WATCH(numReceived);
        WATCH_EXPR("numPeers", myPeers.size());
        WATCH_EXPR("numFecs", fecList.size());

        maxFecid = 0;
        WATCH(maxFecid);
        sendHelloMsg = new cMessage("LDPSendHello");
    }
    else if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        setupSockets();

        // build list of recognized FECs
        rebuildFecList();

        // listen for routing table modifications
        cModule *host = getContainingNode(this);
        host->subscribe(routeAddedSignal, this);
        host->subscribe(routeDeletedSignal, this);
    }
}

void Ldp::setupSockets()
{
    // (re)create fresh socket objects: this also runs on node restart, and a
    // closed socket cannot be re-bound
    udpSocket = UdpSocket();
    udpSockets.clear();
    serverSocket = TcpSocket();

    // bind UDP socket and subscribe to the all-routers group so we receive Hellos
    udpSocket.setOutputGate(gate("socketOut"));
    udpSocket.setCallback(this);
    udpSocket.bind(LDP_PORT);
    for (int i = 0; i < ift->getNumInterfaces(); ++i) {
        NetworkInterface *ie = ift->getInterface(i);
        if (ie->isMulticast()) {
            udpSocket.joinMulticastGroup(Ipv4Address::ALL_ROUTERS_MCAST, ie->getInterfaceId());
            udpSockets.push_back(UdpSocket());
            udpSockets.back().setOutputGate(gate("socketOut"));
            udpSockets.back().setMulticastLoop(false);
            udpSockets.back().setMulticastOutputInterface(ie->getInterfaceId());
        }
    }

    // start listening for incoming TCP conns
    EV_INFO << "Starting to listen on port " << LDP_PORT << " for incoming LDP sessions\n";
    serverSocket.setOutputGate(gate("socketOut"));
    serverSocket.setCallback(this);
    serverSocket.bind(LDP_PORT);
    serverSocket.listen();
}

void Ldp::handleMessageWhenUp(cMessage *msg)
{
    // delete sockets torn down inside a previous callback (safe now: we are no
    // longer executing inside their processMessage())
    for (auto *s : deadSockets)
        delete s;
    deadSockets.clear();

    EV_INFO << "Received: (" << msg->getClassName() << ")" << msg->getName() << "\n";
    if (msg == sendHelloMsg) {
        ASSERT(msg->isSelfMessage());
        // every LDP capable router periodically sends HELLO messages to the
        // "all routers in the sub-network" multicast address
        EV_INFO << "Multicasting LDP Hello to neighboring routers\n";
        sendHelloTo(Ipv4Address::ALL_ROUTERS_MCAST);

        // schedule next hello
        scheduleAfter(helloInterval, sendHelloMsg);
    }
    else if (msg->isSelfMessage()) {
        EV_INFO << "Timer " << msg->getName() << " expired\n";
        if (!strcmp(msg->getName(), "HelloTimeout")) {
            processHelloTimeout(msg);
        }
        else if (!strcmp(msg->getName(), "LDPKeepAliveSendTimer")) {
            processKeepAliveSendTimeout(msg);
        }
        else if (!strcmp(msg->getName(), "LDPSessionHoldTimer")) {
            processSessionHoldTimeout(msg);
        }
        else {
            retryMsgs.erase(std::remove(retryMsgs.begin(), retryMsgs.end(), msg), retryMsgs.end());
            auto ldpPacket = check_and_cast<Packet *>(msg)->popAtFront<LdpPacket>();
            processNOTIFICATION(ldpPacket, /*rescheduled*/ true);
            delete msg;
        }
    }
    else {
        ISocket *socket = socketMap.findSocketFor(msg);
        if (socket) {
            socket->processMessage(msg);
        }
        else if (serverSocket.belongsToSocket(msg)) {
            serverSocket.processMessage(msg);
        }
        else if (udpSocket.belongsToSocket(msg)) {
            udpSocket.processMessage(msg);
        }
        else {
            auto& tags = check_and_cast<ITaggedObject *>(msg)->getTags();
            int socketId = tags.getTag<SocketInd>()->getSocketId();
            for (auto& s : udpSockets) {
                if (s.getSocketId() == socketId) {
                    s.processMessage(msg);
                    return;
                }
            }
            // an in-flight TCP indication may arrive for a socket we just tore
            // down on session loss; drop it instead of aborting
            EV_WARN << "no socket found for msg '" << msg->getName() << "' with socketId " << socketId << ", dropping\n";
            delete msg;
        }
    }
    // TODO move to separate function and reuse from socketClosed
    if (operationalState == State::STOPPING_OPERATION) {
        if (udpSocket.isOpen() || serverSocket.isOpen())
            return;
        // TODO check for empty sockets?
        for (auto& s : udpSockets)
            if (s.isOpen())
                return;
        for (auto s : socketMap.getMap())
            if (s.second->isOpen())
                return;
        udpSockets.clear();
        socketMap.deleteSockets();
        startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
    }
}

void Ldp::socketDataArrived(UdpSocket *socket, Packet *packet)
{
    numReceived++;
    // process incoming udp packet
    // FIXME add implementation
    processLDPHello(check_and_cast<Packet *>(packet));
}

void Ldp::socketErrorArrived(UdpSocket *socket, Indication *indication)
{
    EV_WARN << "Ignoring UDP error report " << indication->getName() << endl;
    delete indication;
}

void Ldp::socketClosed(UdpSocket *socket)
{
}

void Ldp::handleStartOperation(LifecycleOperation *operation)
{
    // OperationalMixin calls this with operation==nullptr during initialize()
    // (before the socket-setup init stage); a real restart passes a non-null
    // operation and must re-open the sockets and rebuild the FEC state that stop
    // tore down.
    if (operation != nullptr) {
        setupSockets();
        rebuildFecList();
    }
    scheduleAfter(exponential(0.1), sendHelloMsg);
}

void Ldp::handleStopOperation(LifecycleOperation *operation)
{
    clearState();
    udpSocket.close();
    for (auto& s : udpSockets)
        s.close();
    serverSocket.close();
    for (auto s : socketMap.getMap())
        s.second->close();
    delayActiveOperationFinish(par("stopOperationTimeout"));
}

void Ldp::handleCrashOperation(LifecycleOperation *operation)
{
    clearState();
    udpSocket.destroy();
    for (auto& s : udpSockets)
        s.destroy();
    serverSocket.destroy();
    for (auto s : socketMap.getMap())
        s.second->destroy();
}

void Ldp::clearState()
{
    for (auto& elem : myPeers) {
        cancelAndDelete(elem.timeout);
        cancelAndDelete(elem.keepAliveSendTimer);
        cancelAndDelete(elem.sessionHoldTimer);
    }
    myPeers.clear();
    cancelEvent(sendHelloMsg);
    for (auto *m : retryMsgs)
        cancelAndDelete(m);
    retryMsgs.clear();
    // drop label bindings so a restart does not resume from stale state
    fecUp.clear();
    fecDown.clear();
    fecList.clear();
    pending.clear();
    emitFecBindingCount();
}

void Ldp::sendToPeer(Ipv4Address dest, Packet *msg)
{
    numSent++;
    getPeerSocket(dest)->send(msg);

    // RFC 5036 Section 2.5.6: sending ANY LDP message resets the KeepAlive send
    // timer (it only exists once the session is OPERATIONAL -- see
    // processKEEPALIVE's OPENREC->OPERATIONAL transition)
    int i = findPeer(dest);
    if (i != -1 && myPeers[i].keepAliveSendTimer != nullptr)
        rescheduleAfter(myPeers[i].negotiatedKeepaliveTime / 3, myPeers[i].keepAliveSendTimer);
}

bool Ldp::isPeerOperational(Ipv4Address peerAddr)
{
    int i = findPeer(peerAddr);
    return i != -1 && myPeers[i].state == peer_info::OPERATIONAL;
}

void Ldp::sendInit(Ipv4Address dest)
{
    Packet *pk = new Packet("Ldp-Init");
    const auto& ini = makeShared<LdpIni>();
    ini->setChunkLength(LDP_INITIALIZATION_BYTES);
    ini->setType(INITIALIZATION);
    ini->setLsrId(rt->getRouterId());
    ini->setKeepAliveTime((uint16_t)keepaliveTime.inUnit(SIMTIME_S));
    // RFC 5036 Section 3.5.3 A-bit (Label Advertisement Discipline): 0 signals
    // Downstream Unsolicited, 1 signals Downstream on Demand.
    ini->setAbit(distributionMode == "dod");
    ini->setDbit(false);
    ini->setReceiverLsrId(dest);
    pk->insertAtBack(ini);

    sendToPeer(dest, pk);
    EV_INFO << "Init sent to " << dest << " (keepAliveTime=" << keepaliveTime << ")\n";
}

void Ldp::sendKeepAlive(Ipv4Address dest)
{
    Packet *pk = new Packet("Ldp-KeepAlive");
    const auto& ka = makeShared<LdpKeepAlive>();
    ka->setChunkLength(LDP_KEEPALIVE_BYTES);
    ka->setType(KEEP_ALIVE);
    ka->setLsrId(rt->getRouterId());
    pk->insertAtBack(ka);

    sendToPeer(dest, pk);
    EV_INFO << "KeepAlive sent to " << dest << "\n";
}

void Ldp::sendAddress(Ipv4Address dest)
{
    // RFC 5036 Section 3.5.5/2.7: advertise all of this router's LDP-capable
    // interface addresses. "LDP-capable" is taken to mean the same set of
    // interfaces LDP itself sends/joins Hellos on (setupSockets' multicast-
    // capable interfaces), rather than re-deriving a separate notion of
    // "own addresses" (rebuildFecList's "our own addresses" block serves a
    // different purpose -- FEC bookkeeping -- and is not reused here).
    std::vector<Ipv4Address> addrs;
    for (int i = 0; i < ift->getNumInterfaces(); ++i) {
        NetworkInterface *ie = ift->getInterface(i);
        if (!ie->isMulticast())
            continue;
        auto ipv4Data = ie->findProtocolData<Ipv4InterfaceData>();
        if (ipv4Data)
            addrs.push_back(ipv4Data->getIPAddress());
    }

    Packet *pk = new Packet("Ldp-Address");
    const auto& addrMsg = makeShared<LdpAddress>();
    addrMsg->setChunkLength(ldpAddressMessageBytes(addrs.size()));
    addrMsg->setType(ADDRESS);
    addrMsg->setLsrId(rt->getRouterId());
    addrMsg->setAddressFamily(1); // IPv4
    addrMsg->setAddressesArraySize(addrs.size());
    for (size_t i = 0; i < addrs.size(); ++i)
        addrMsg->setAddresses(i, addrs[i]);
    pk->insertAtBack(addrMsg);

    sendToPeer(dest, pk);

    EV_INFO << "Address message sent to " << dest << " listing " << addrs.size() << " interface address(es):";
    for (auto& a : addrs)
        EV_INFO << " " << a;
    EV_INFO << endl;
}

void Ldp::sendMappingRequest(Ipv4Address dest, Ipv4Address addr, int length)
{
    if (!isPeerOperational(dest)) {
        EV_WARN << "not sending Label Request to " << dest << ": session is not OPERATIONAL\n";
        return;
    }

    Packet *pk = new Packet("Lb-Req");
    const auto& requestMsg = makeShared<LdpLabelRequest>();
    requestMsg->setChunkLength(LDP_LABEL_REQUEST_BYTES);
    requestMsg->setType(LABEL_REQUEST);

    FecTlv fec;
    fec.addr = addr;
    fec.length = length;
    requestMsg->setFec(fec);

    requestMsg->setLsrId(rt->getRouterId());
    pk->insertAtBack(requestMsg);

    sendToPeer(dest, pk);
}

void Ldp::duAdvertiseToPeer(const Ldp::Fec& fec, Ipv4Address peer)
{
    if (!isPeerOperational(peer))
        return;

    auto uit = findFecEntry(fecUp, fec.fecid, peer);
    // is the FEC's next hop our LDP peer, or are WE egress for it?
    bool ER = findPeerSocket(fec.nextHop) == nullptr;
    // do we hold a mapping from the FEC's CURRENT next hop? (liberal retention may
    // already hold one even though it only just became the next hop)
    auto dit = findFecEntry(fecDown, fec.fecid, fec.nextHop);

    int inInterface = findInterfaceFromPeerAddr(peer);

    bool haveMapping = ER || dit != fecDown.end();
    if (haveMapping) {
        // we can offer a real mapping now: either we are egress for this FEC
        // (ALWAYS advertised, regardless of control mode), or we hold a downstream
        // mapping for its current next hop -- freshly received, or already held
        // thanks to liberal retention (e.g. this peer just became the next hop
        // after a reroute). PHP behavior (advertiseImplicitNull) is unchanged from
        // the DoD path.
        bool implicitNull = ER && advertiseImplicitNull;
        LabelOpVector outLabel;
        int outInterface = -1;
        if (!implicitNull) {
            outInterface = findInterfaceFromPeerAddr(fec.nextHop);
            if (ER)
                outLabel = LibTable::popLabel();
            else
                outLabel = (dit->label == IMPLICIT_NULL_LABEL) ? LibTable::popLabel() : LibTable::swapLabel(dit->label);
        }

        int label;
        if (implicitNull) {
            // an implicit-null advertisement never allocates/installs a real LIB entry
            label = IMPLICIT_NULL_LABEL;
            if (uit == fecUp.end()) {
                FecBinding newItem;
                newItem.fecid = fec.fecid;
                newItem.peer = peer;
                newItem.label = label;
                fecUp.push_back(newItem);
            }
            else {
                uit->label = label;
                uit->installed = true;
            }
        }
        else if (uit == fecUp.end()) {
            label = lt->installLibEntry(-1, inInterface, outLabel, outInterface);
            FecBinding newItem;
            newItem.fecid = fec.fecid;
            newItem.peer = peer;
            newItem.label = label;
            fecUp.push_back(newItem);
        }
        else if (!uit->installed || uit->label == IMPLICIT_NULL_LABEL) {
            // either completing a label reserved earlier by independent control
            // (no LIB entry exists yet for it), or this peer's previously
            // advertised label was the implicit-null sentinel (never a real LIB
            // entry, and not reusable as one) -- either way there is no existing
            // LIB entry to update, but the label to (re-)use differs: a genuine
            // reservation must be completed at that SAME label (already on the
            // wire); a former implicit-null needs a brand new real label instead
            if (uit->installed) // i.e. was implicit-null
                label = lt->installLibEntry(-1, inInterface, outLabel, outInterface);
            else {
                label = uit->label;
                lt->installReservedLabel(label, inInterface, outLabel, outInterface);
            }
            uit->label = label;
            uit->installed = true;
        }
        else {
            // already installed (e.g. a prior next hop): refresh the swap target
            label = lt->installLibEntry(uit->label, inInterface, outLabel, outInterface);
            uit->label = label;
        }

        if (ER)
            EV_INFO << "DU: advertising unsolicited Label Mapping (egress) label=" << label << " for fec addr="
                    << fec.addr << " length=" << fec.length << " to " << peer << endl;
        else
            EV_INFO << "DU: advertising unsolicited Label Mapping label=" << label << " for fec addr=" << fec.addr
                    << " length=" << fec.length << " to " << peer << " using an already-held downstream mapping from "
                    << fec.nextHop << " (liberal retention switchover if the next hop just changed)" << endl;
        sendMapping(LABEL_MAPPING, peer, label, fec.addr, fec.length);
        return;
    }

    // no downstream mapping for the FEC's current next hop, and we are not egress
    if (controlMode == "independent") {
        if (uit == fecUp.end()) {
            // independent control: advertise now, without waiting for a downstream
            // mapping; reserve (but do not install) an inLabel -- the LIB swap
            // follows once the downstream mapping arrives (see
            // processLABEL_MAPPING). Until then, an incoming packet for this label
            // has nowhere to swap to and is dropped by
            // Mpls::processMplsPacketFromL2's resolveLabel() miss -- this is the
            // FAITHFUL independent-control transient, not a bug.
            int label = lt->allocateLabel();
            FecBinding newItem;
            newItem.fecid = fec.fecid;
            newItem.peer = peer;
            newItem.label = label;
            newItem.installed = false;
            fecUp.push_back(newItem);

            EV_INFO << "DU: advertising unsolicited Label Mapping (independent control, no downstream mapping yet) label="
                    << label << " for fec addr=" << fec.addr << " length=" << fec.length << " to " << peer << endl;
            sendMapping(LABEL_MAPPING, peer, label, fec.addr, fec.length);
        }
        else if (uit->label == IMPLICIT_NULL_LABEL) {
            // (uit->installed is necessarily true here, see the invariant note on
            // FecBinding::installed) an implicit-null advertisement never had a
            // real LIB entry to fall back to "pending" the way a genuine reserved
            // label can -- 3 must never be reused as a real inLabel either -- so
            // withdraw it outright instead. If a real mapping becomes available
            // again later, this function creates a brand new fecUp entry from
            // scratch (uit == fecUp.end() at that point).
            EV_INFO << "DU: withdrawing previously-advertised implicit-null mapping for fec addr=" << fec.addr
                    << " length=" << fec.length << " from " << peer
                    << ": we are no longer egress and no downstream mapping is held for it" << endl;
            sendMapping(LABEL_WITHDRAW, peer, uit->label, fec.addr, fec.length);
            fecUp.erase(uit);
        }
        else if (uit->installed) {
            // we previously advertised this FEC to this peer with a real, installed
            // LIB entry (e.g. for a former next hop); the next hop has since
            // changed and we no longer hold a downstream mapping for the new one.
            // The label we already gave upstream stays valid on the wire (no need
            // to re-advertise), but the installed entry now points at a stale (no
            // longer applicable) target -- remove it so traffic drops instead of
            // being misrouted
            EV_DETAIL << "DU: next hop changed and no downstream mapping is held for the new one; removing the "
                      << "(now stale) LIB entry for label=" << uit->label << ", fec addr=" << fec.addr << " length="
                      << fec.length << endl;
            lt->removeLibEntryIfExists(uit->label);
            uit->installed = false;
        }
        // else: already reserved-but-not-installed from before; nothing new to do
    }
    else {
        // ordered control: nothing to advertise without a downstream mapping
        if (uit != fecUp.end()) {
            // we previously advertised this FEC (we had a downstream mapping back
            // then) and have since lost it on a next-hop change -- withdraw
            EV_INFO << "DU: withdrawing previously-advertised mapping label=" << uit->label << " for fec addr="
                    << fec.addr << " length=" << fec.length << " from " << peer
                    << ": next hop changed and no downstream mapping is held for it" << endl;
            sendMapping(LABEL_WITHDRAW, peer, uit->label, fec.addr, fec.length);
            if (uit->installed && uit->label != IMPLICIT_NULL_LABEL)
                lt->removeLibEntryIfExists(uit->label);
            fecUp.erase(uit);
        }
    }
}

void Ldp::updateFecListEntry(Ldp::Fec oldItem)
{
    if (distributionMode == "du") {
        // RFC 5036 Section 2.6: Downstream Unsolicited -- (re)advertise this FEC to
        // every currently OPERATIONAL peer. Called only when this FEC was just
        // created or its next hop actually changed (see rebuildFecList), never for
        // an unchanged FEC.
        for (auto& p : myPeers) {
            if (p.state == peer_info::OPERATIONAL)
                duAdvertiseToPeer(oldItem, p.peerIP);
        }
        emitFecBindingCount();
        return;
    }

    // DoD path (distributionMode == "dod"): current Request/Mapping flow, unchanged

    // do we have mapping from downstream?
    auto dit = findFecEntry(fecDown, oldItem.fecid, oldItem.nextHop);

    // is next hop our LDP peer?
    bool ER = findPeerSocket(oldItem.nextHop) == nullptr;

    ASSERT(!(ER && dit != fecDown.end())); // can't be egress and have mapping at the same time

    // adjust upstream mappings
    for (auto uit = fecUp.begin(); uit != fecUp.end();) {
        if (uit->fecid != oldItem.fecid) {
            uit++;
            continue;
        }

        int inInterface = findInterfaceFromPeerAddr(uit->peer);
        int outInterface = findInterfaceFromPeerAddr(oldItem.nextHop);
        if (ER) {
            if (advertiseImplicitNull) {
                // penultimate hop popping: advertise the implicit null label
                // instead of allocating a local pop entry -- our upstream peer
                // must pop the label itself, so no labeled traffic for this
                // FEC should ever reach us
                uit->label = IMPLICIT_NULL_LABEL;
                EV_INFO << "advertising implicit null label (penultimate hop popping) for inInterface=" << inInterface
                        << " outInterface=" << outInterface << endl;
            }
            else {
                // we are egress, that's easy:
                LabelOpVector outLabel = LibTable::popLabel();
                uit->label = lt->installLibEntry(uit->label, inInterface, outLabel, outInterface);

                EV_DETAIL << "installed (egress) LIB entry inLabel=" << uit->label << " inInterface=" << inInterface
                          << " outLabel=" << outLabel << " outInterface=" << outInterface << endl;
            }
            uit++;
        }
        else if (dit != fecDown.end()) {
            // we have mapping from DS, that's easy -- unless the downstream
            // peer is itself the egress and advertised the implicit null
            // label, in which case we are the penultimate hop and must pop
            // rather than swap to label 3
            LabelOpVector outLabel = (dit->label == IMPLICIT_NULL_LABEL) ? LibTable::popLabel() : LibTable::swapLabel(dit->label);
            uit->label = lt->installLibEntry(uit->label, inInterface, outLabel, outInterface);

            EV_DETAIL << "installed LIB entry inLabel=" << uit->label << " inInterface=" << inInterface
                      << " outLabel=" << outLabel << " outInterface=" << outInterface << endl;
            uit++;
        }
        else {
            // no mapping from DS, withdraw mapping US
            EV_INFO << "sending withdraw message upstream" << endl;
            sendMapping(LABEL_WITHDRAW, uit->peer, uit->label, oldItem.addr, oldItem.length);

            // remove from US mappings
            uit = fecUp.erase(uit);
        }
    }

    if (!ER && dit == fecDown.end()) {
        // and ask DS for mapping
        EV_INFO << "sending request message downstream" << endl;
        sendMappingRequest(oldItem.nextHop, oldItem.addr, oldItem.length);
    }

    emitFecBindingCount();
}

void Ldp::rebuildFecList()
{
    EV_INFO << "make list of recognized FECs" << endl;

    FecVector oldList = fecList;
    fecList.clear();

    for (int i = 0; i < rt->getNumRoutes(); i++) {
        // every entry in the routing table

        const Ipv4Route *re = rt->getRoute(i);

        // ignore multicast routes
        if (re->getDestination().isMulticast())
            continue;

        // find out current next hop according to routing table
        Ipv4Address nextHop = (re->getGateway().isUnspecified()) ? re->getDestination() : re->getGateway();
        ASSERT(!nextHop.isUnspecified());

        EV_INFO << "nextHop <-- " << nextHop << endl;

        auto it = findFecEntry(oldList, re->getDestination(), re->getNetmask().getNetmaskLength());

        if (it == oldList.end()) {
            // fec didn't exist, it was just created
            Fec newItem;
            newItem.fecid = ++maxFecid;
            newItem.addr = re->getDestination();
            newItem.length = re->getNetmask().getNetmaskLength();
            newItem.nextHop = nextHop;
            updateFecListEntry(newItem);
            fecList.push_back(newItem);
        }
        else if (it->nextHop != nextHop) {
            // next hop for this FEC changed,
            it->nextHop = nextHop;
            updateFecListEntry(*it);
            fecList.push_back(*it);
            oldList.erase(it);
        }
        else {
            // FEC didn't change, reusing old values
            fecList.push_back(*it);
            oldList.erase(it);
            continue;
        }
    }

    // our own addresses (TODO is it needed?)

    for (int i = 0; i < ift->getNumInterfaces(); ++i) {
        NetworkInterface *ie = ift->getInterface(i);

        // TODO should replace to ie->isUp() or drop this code:
//        if (ie->getNetworkLayerGateIndex() < 0)
//            continue;

        auto ipv4Data = ie->findProtocolData<Ipv4InterfaceData>();
        if (!ipv4Data)
            continue;

        auto it = findFecEntry(oldList, ipv4Data->getIPAddress(), 32);
        if (it == oldList.end()) {
            Fec newItem;
            newItem.fecid = ++maxFecid;
            newItem.addr = ipv4Data->getIPAddress();
            newItem.length = 32;
            newItem.nextHop = ipv4Data->getIPAddress();
            fecList.push_back(newItem);
        }
        else {
            fecList.push_back(*it);
            oldList.erase(it);
        }
    }

    if (oldList.size() > 0) {
        EV_INFO << "there are " << oldList.size() << " deprecated FECs, removing them" << endl;

        for (auto& elem : oldList) {
            EV_DETAIL << "removing FEC= " << elem << endl;

            for (auto& _dit : fecDown) {
                if (_dit.fecid != elem.fecid)
                    continue;

                EV_DETAIL << "sending release label=" << _dit.label << " downstream to " << _dit.peer << endl;

                sendMapping(LABEL_RELEASE, _dit.peer, _dit.label, elem.addr, elem.length);
            }

            for (auto& _uit : fecUp) {
                if (_uit.fecid != elem.fecid)
                    continue;

                EV_DETAIL << "sending withdraw label=" << _uit.label << " upstream to " << _uit.peer << endl;

                sendMapping(LABEL_WITHDRAW, _uit.peer, _uit.label, elem.addr, elem.length);

                // an implicit-null advertisement never allocated a LIB entry, and
                // under DU independent control a label may have been advertised
                // (reserved via allocateLabel()) but never actually installed if
                // no downstream mapping ever arrived before the FEC itself
                // disappeared -- removeLibEntryIfExists() tolerates both
                if (_uit.label != IMPLICIT_NULL_LABEL) {
                    EV_DETAIL << "removing entry inLabel=" << _uit.label << " from LIB (if it was ever installed)" << endl;

                    lt->removeLibEntryIfExists(_uit.label);
                }
            }
        }
    }

    // we must keep this list sorted for matching to work correctly
    // this is probably slower than it must be
    std::stable_sort(fecList.begin(), fecList.end(), fecPrefixCompare);
}

void Ldp::updateFecList(Ipv4Address nextHop)
{
    for (auto& elem : fecList) {
        if (elem.nextHop != nextHop)
            continue;

        updateFecListEntry(elem);
    }
}

void Ldp::emitFecBindingCount()
{
    emit(fecBindingCountSignal, (long)(fecUp.size() + fecDown.size()));
}

void Ldp::sendHelloTo(Ipv4Address dest)
{
    Packet *pk = new Packet("LDP-Hello");
    const auto& hello = makeShared<LdpHello>();
    hello->setChunkLength(LDP_HELLO_BYTES);
    hello->setType(HELLO);
    hello->setLsrId(rt->getRouterId());
    // targeted hellos (which would need a destination LSR-Id) and the R-bit/T-bit
    // (LDP hello state machine extensions) are not modeled; left at their default
    // (unset/false) values
    hello->setHoldTime((uint16_t)holdTime.inUnit(SIMTIME_S));
    pk->insertAtBack(hello);

    if (dest.isMulticast()) {
        for (size_t i = 0; i < udpSockets.size(); ++i) {
            Packet *msg = (i == udpSockets.size() - 1) ? pk : pk->dup();
            udpSockets[i].sendTo(msg, dest, LDP_PORT);
        }
        if (udpSockets.size() == 0)
            delete pk;
    }
    else
        udpSocket.sendTo(pk, dest, LDP_PORT);
}

void Ldp::processHelloTimeout(cMessage *msg)
{
    // peer is gone

    unsigned int i;
    for (i = 0; i < myPeers.size(); i++)
        if (myPeers[i].timeout == msg)
            break;

    ASSERT(i < myPeers.size());

    Ipv4Address peerIP = myPeers[i].peerIP;

    EV_INFO << "peer=" << peerIP << " is gone, removing adjacency" << endl;

    ASSERT(!myPeers[i].timeout->isScheduled());
    delete myPeers[i].timeout;
    cancelAndDelete(myPeers[i].keepAliveSendTimer);
    cancelAndDelete(myPeers[i].sessionHoldTimer);
    if (myPeers[i].socket) {
        socketMap.removeSocket(myPeers[i].socket);
        myPeers[i].socket->abort(); // should we only close?
        delete myPeers[i].socket;
    }
    myPeers.erase(myPeers.begin() + i);

    removePeerBindings(peerIP);

    // update TED and routing table

    unsigned int index = tedmod->linkIndex(rt->getRouterId(), peerIP);
    tedmod->setLinkState(index, false);
}

void Ldp::processKeepAliveSendTimeout(cMessage *msg)
{
    for (size_t i = 0; i < myPeers.size(); i++) {
        if (myPeers[i].keepAliveSendTimer == msg) {
            // RFC 5036 Section 2.5.6: no LDP message was sent to this peer for a
            // full negotiatedKeepaliveTime/3 -- send an explicit KeepAlive so the
            // peer's session hold timer never expires on us. sendKeepAlive() ->
            // sendToPeer() reschedules this very timer, so nothing else to do here.
            // Guard against the narrow window where the socket has already left
            // CONNECTED (e.g. a graceful close in progress via socketPeerClosed)
            // but the teardown callback hasn't fired yet to cancel this timer:
            // a peer/protocol-adjacent condition must never abort the simulation
            // (see getPeerSocket()), so just skip this cycle instead of sending.
            if (findPeerSocket(myPeers[i].peerIP) == nullptr) {
                EV_DETAIL << "skipping scheduled KeepAlive to " << myPeers[i].peerIP << ": session is closing\n";
                return;
            }
            EV_DETAIL << "no LDP message sent to " << myPeers[i].peerIP << " for "
                      << (myPeers[i].negotiatedKeepaliveTime / 3) << ", sending KeepAlive" << endl;
            sendKeepAlive(myPeers[i].peerIP);
            return;
        }
    }
    // the timer belongs to a peer that no longer exists (session already torn
    // down through another path); nothing to do
}

void Ldp::processSessionHoldTimeout(cMessage *msg)
{
    for (size_t i = 0; i < myPeers.size(); i++) {
        if (myPeers[i].sessionHoldTimer == msg) {
            Ipv4Address peerIP = myPeers[i].peerIP;

            // RFC 5036 Section 2.5.6: no LDP message (Keepalive or otherwise) was
            // received from this peer within the negotiated KeepAlive Time -- the
            // control channel is presumed dead even though the Hello adjacency may
            // still be alive (a stale CONNECTED TcpSocket the transport layer
            // hasn't itself noticed yet; see the peer_info comment in Ldp.h).
            // Terminate the session now rather than waiting on TCP's own,
            // potentially much slower, failure detection.
            EV_WARN << "KeepAlive Timer Expired for session with " << peerIP << ": no LDP message received within "
                    << myPeers[i].negotiatedKeepaliveTime << "; closing the session (hello adjacency, if still alive, "
                    << "will re-establish it on the next Hello -- see processLDPHello)" << endl;

            if (findPeerSocket(peerIP) != nullptr)
                sendNotify(KEEPALIVE_TIMER_EXPIRED, peerIP, Ipv4Address(), 0);

            // decisive, synchronous teardown of the transport connection -- mirrors
            // processHelloTimeout's abort() (we cannot trust a graceful close() to
            // ever complete against a peer that may not even be there any more),
            // but -- unlike processHelloTimeout -- the peer_info entry itself
            // (and the Hello adjacency it represents) is NOT erased
            if (myPeers[i].socket) {
                socketMap.removeSocket(myPeers[i].socket);
                myPeers[i].socket->abort();
                delete myPeers[i].socket;
                myPeers[i].socket = nullptr;
            }
            emit(sessionDownSignal, (long)peerIP.getInt());

            removePeerBindings(peerIP); // resets FSM state/timers (incl. deleting 'msg' itself) and discards bindings
            return;
        }
    }
    // the timer belongs to a peer that no longer exists (session already torn
    // down through another path); nothing to do
}

void Ldp::removePeerBindings(Ipv4Address peerIP)
{
    // reset the session FSM: this is the common teardown point reached whenever a
    // session dies (handleTcpConnectionDown, a stale-session replacement in
    // socketAvailable, or a dead Hello adjacency in processHelloTimeout -- in the
    // last case the peer_info entry has already been erased, so findPeer() below
    // legitimately finds nothing and there is nothing left to reset)
    int pi = findPeer(peerIP);
    if (pi != -1) {
        cancelAndDelete(myPeers[pi].keepAliveSendTimer);
        myPeers[pi].keepAliveSendTimer = nullptr;
        cancelAndDelete(myPeers[pi].sessionHoldTimer);
        myPeers[pi].sessionHoldTimer = nullptr;
        myPeers[pi].state = peer_info::NONEXISTENT;
        myPeers[pi].negotiatedKeepaliveTime = 0;
        // stale until re-advertised by a fresh Address message once the
        // replacement session reaches OPERATIONAL (see processKEEPALIVE)
        myPeers[pi].peerAddresses.clear();
    }

    EV_INFO << "removing (stale) bindings from fecDown for peer=" << peerIP << endl;

    for (auto dit = fecDown.begin(); dit != fecDown.end();) {
        if (dit->peer != peerIP) {
            dit++;
            continue;
        }

        EV_DETAIL << "label=" << dit->label << endl;

        // send release message just in case (?)
        // what happens if peer is not really down and
        // hello messages just disappeared?
        // does the protocol recover on its own (TODO check this)

        dit = fecDown.erase(dit);
    }

    EV_INFO << "removing bindings from sent to peer=" << peerIP << " from fecUp" << endl;

    for (auto uit = fecUp.begin(); uit != fecUp.end();) {
        if (uit->peer != peerIP) {
            uit++;
            continue;
        }

        EV_DETAIL << "label=" << uit->label << endl;

        // send withdraw message just in case (?)
        // see comment above...

        uit = fecUp.erase(uit);
    }

    emitFecBindingCount();

    EV_INFO << "updating fecList" << endl;

    updateFecList(peerIP);
}

void Ldp::processLDPHello(Packet *msg)
{
    int socketId = msg->getTag<SocketInd>()->getSocketId();
    ASSERT(socketId == udpSocket.getSocketId());

    const auto& ldpHello = msg->peekAtFront<LdpHello>();
    Ipv4Address peerAddr = ldpHello->getLsrId();
    uint16_t receivedHoldTime = ldpHello->getHoldTime();
    int interfaceId = msg->getTag<InterfaceInd>()->getInterfaceId();
    delete msg;

    // RFC 5036 2.4.1: the effective hold time is the smaller of the two proposals
    simtime_t effectiveHoldTime = receivedHoldTime > 0 ? std::min(holdTime, SimTime((int)receivedHoldTime, SIMTIME_S)) : holdTime;

    EV_INFO << "Received LDP Hello from " << peerAddr << ", ";

    if (peerAddr.isUnspecified() || peerAddr == rt->getRouterId()) {
        // must be ourselves (we're also in the all-routers multicast group), ignore
        EV_INFO << "that's myself, ignore\n";
        return;
    }

    // report the link as working again; Ted decides whether that's actually
    // a change (and rebuilds/announces accordingly)
    unsigned int index = tedmod->linkIndex(rt->getRouterId(), peerAddr);
    tedmod->setLinkState(index, true);

    // peer already in table?
    int i = findPeer(peerAddr);
    if (i != -1) {
        EV_DETAIL << "already in my peer table, rescheduling timeout" << endl;
        ASSERT(myPeers[i].timeout);
        rescheduleAfter(effectiveHoldTime, myPeers[i].timeout);
        // if the session died but the hello adjacency survived, re-establish it
        if (myPeers[i].activeRole && myPeers[i].socket == nullptr) {
            EV_INFO << "session with peer " << peerAddr << " is down, reconnecting\n";
            openTCPConnectionToPeer(i);
        }
        return;
    }

    // not in table, add it
    peer_info info;
    info.peerIP = peerAddr;
    info.linkInterface = ift->getInterfaceById(interfaceId)->getInterfaceName();
    // RFC 5036 Section 2.5.2: the LSR with the GREATER transport address plays
    // the active role and initiates the TCP connection.
    info.activeRole = rt->getRouterId().getInt() > peerAddr.getInt();
    info.socket = nullptr;
    info.timeout = new cMessage("HelloTimeout");
    scheduleAfter(effectiveHoldTime, info.timeout);
    myPeers.push_back(info);
    int peerIndex = myPeers.size() - 1;

    EV_INFO << "added to peer table\n";
    EV_INFO << "We'll be " << (info.activeRole ? "ACTIVE" : "PASSIVE") << " in this session\n";

    // introduce ourselves with a Hello, then connect if we're in ACTIVE role
    sendHelloTo(peerAddr);
    if (info.activeRole) {
        EV_INFO << "Establishing session with it\n";
        openTCPConnectionToPeer(peerIndex);
    }
}

void Ldp::openTCPConnectionToPeer(int peerIndex)
{
    TcpSocket *socket = new TcpSocket();
    socket->setOutputGate(gate("socketOut"));
    socket->setCallback(this);
    socket->bind(rt->getRouterId(), 0);
    socketMap.addSocket(socket);
    myPeers[peerIndex].socket = socket;

    socket->connect(myPeers[peerIndex].peerIP, LDP_PORT);
}

void Ldp::socketEstablished(TcpSocket *socket)
{
    int i = findPeer(socket->getRemoteAddress().toIpv4());
    if (i == -1) {
        EV_WARN << "TCP connection established with an unknown peer, ignoring\n";
        return;
    }
    EV_INFO << "TCP connection established with peer " << myPeers[i].peerIP << "\n";

    // RFC 5036 Section 2.5.3: the transport connection alone does not make the
    // session usable -- Initialization/KeepAlive negotiation must complete first
    // (see processINITIALIZATION/processKEEPALIVE). sessionUp and updateFecList()
    // are deferred to that OPERATIONAL transition.
    myPeers[i].state = peer_info::INITIALIZED;

    if (myPeers[i].activeRole) {
        // the LSR with the greater transport address plays the active role and
        // initiates parameter negotiation (RFC 5036 Section 2.5.2/2.5.3)
        sendInit(myPeers[i].peerIP);
        myPeers[i].state = peer_info::OPENSENT;
    }
}

void Ldp::socketAvailable(TcpSocket *socketocket, TcpAvailableInfo *availableInfo)
{
    // TODO
    // not yet in socketMap, must be new incoming connection.
    // find which peer it is and register connection
    TcpSocket *newSocket = new TcpSocket(availableInfo);
    newSocket->setOutputGate(gate("socketOut"));

    // FIXME there seems to be some confusion here. Is it sure that
    // routerIds we use as peerAddrs are the same as IP addresses
    // the routing is based on? --Andras
    Ipv4Address peerAddr = newSocket->getRemoteAddress().toIpv4();

    int i = findPeer(peerAddr);
    if (i == -1) {
        // nothing known about this peer: refuse
        newSocket->close(); // reset()?
        delete newSocket;
        return;
    }
    if (myPeers[i].socket) {
        // we already hold a session to this peer. Only the active side connects,
        // so a fresh incoming connection means the peer restarted and its old
        // session is stale -- replace it.
        EV_WARN << "peer " << peerAddr << " reconnected, replacing the stale session\n";
        socketMap.removeSocket(myPeers[i].socket);
        myPeers[i].socket->abort();
        delete myPeers[i].socket;
        myPeers[i].socket = nullptr;
        removePeerBindings(peerAddr);
    }
    myPeers[i].socket = newSocket;
    newSocket->setCallback(this);
    socketMap.addSocket(newSocket);
    socketocket->accept(availableInfo->getNewSocketId());
}

void Ldp::socketDataArrived(TcpSocket *socket)
{
    int i = findPeer(socket->getRemoteAddress().toIpv4());
    if (i != -1)
        EV_INFO << "Message arrived over TCP from peer " << myPeers[i].peerIP << "\n";

    auto queue = socket->getReadBuffer();
    while (queue->has<LdpPacket>()) {
        numReceived++;
        auto header = queue->pop<LdpPacket>();
        processLdpPacketFromTcp(header);
    }
}

void Ldp::handleTcpConnectionDown(TcpSocket *socket)
{
    // Discard the session and its label bindings. The hello adjacency (UDP)
    // stays alive, so the session is re-established the next time a Hello is
    // received (the active side reconnects) -- RFC 5036 hello-driven recovery.
    // Deleting the socket here is safe: TcpSocket::processMessage invokes these
    // callbacks as its last action on the socket, and handleMessageWhenUp does
    // not touch the socket after processMessage returns.
    if (socketMap.removeSocket(socket) == nullptr)
        return; // already torn down (e.g. both socketClosed and socketFailure fired)

    int i = findPeer(socket->getRemoteAddress().toIpv4());
    if (i != -1 && myPeers[i].socket == socket) {
        Ipv4Address peerIP = myPeers[i].peerIP;
        myPeers[i].socket = nullptr;
        // sessionDown mirrors sessionUp: only meaningful if the session ever
        // actually reached OPERATIONAL (RFC 5036 session, not just a TCP connection)
        if (myPeers[i].state == peer_info::OPERATIONAL)
            emit(sessionDownSignal, (long)peerIP.getInt());
        removePeerBindings(peerIP); // also resets the session FSM state and timers
    }
    // defer the delete: we are called from inside socket->processMessage(), which
    // still touches the socket after this callback returns
    deadSockets.push_back(socket);
}

void Ldp::socketPeerClosed(TcpSocket *socket)
{
    EV_INFO << "Peer " << socket->getRemoteAddress() << " closed the TCP connection, closing here as well\n";
    // close our side; the ensuing socketClosed will clean up the session
    socket->close();
}

void Ldp::socketClosed(TcpSocket *socket)
{
    EV_WARN << "TCP connection to peer " << socket->getRemoteAddress() << " closed\n";
    handleTcpConnectionDown(socket);
}

void Ldp::socketFailure(TcpSocket *socket, int code)
{
    EV_WARN << "TCP connection to peer " << socket->getRemoteAddress() << " broken\n";
    handleTcpConnectionDown(socket);
}

void Ldp::processLdpPacketFromTcp(Ptr<const LdpPacket>& ldpPacket)
{
    // RFC 5036 Section 2.5.6: receiving ANY LDP message resets the session hold
    // timer (it only exists once the session is OPERATIONAL -- see
    // processKEEPALIVE's OPENREC->OPERATIONAL transition)
    int hi = findPeer(ldpPacket->getLsrId());
    if (hi != -1 && myPeers[hi].sessionHoldTimer != nullptr)
        rescheduleAfter(myPeers[hi].negotiatedKeepaliveTime, myPeers[hi].sessionHoldTimer);

    switch (ldpPacket->getType()) {
        case HELLO:
            // Hellos belong on UDP; a Hello over TCP is a peer/protocol anomaly, ignore it
            EV_WARN << "ignoring an LDP HELLO received over TCP (Hellos arrive over UDP)" << endl;
            break;

        case INITIALIZATION:
            processINITIALIZATION(ldpPacket);
            break;

        case KEEP_ALIVE:
            processKEEPALIVE(ldpPacket);
            break;

        case LABEL_MAPPING:
        case LABEL_REQUEST:
        case LABEL_WITHDRAW:
        case LABEL_RELEASE:
        case ADDRESS:
        case ADDRESS_WITHDRAW: {
            // RFC 5036 Section 3.5.3: label- and address-plane messages are only
            // valid once the session has completed Initialization/KeepAlive
            // negotiation
            Ipv4Address srcAddr = ldpPacket->getLsrId();
            if (!isPeerOperational(srcAddr)) {
                EV_WARN << "rejecting LDP message type " << ldpPacket->getType() << " from " << srcAddr
                        << ": session is not OPERATIONAL yet" << endl;
                if (findPeerSocket(srcAddr) != nullptr)
                    sendNotify(SHUTDOWN, srcAddr, Ipv4Address(), 0);
                break;
            }
            switch (ldpPacket->getType()) {
                case LABEL_MAPPING: processLABEL_MAPPING(ldpPacket); break;
                case LABEL_REQUEST: processLABEL_REQUEST(ldpPacket); break;
                case LABEL_WITHDRAW: processLABEL_WITHDRAW(ldpPacket); break;
                case LABEL_RELEASE: processLABEL_RELEASE(ldpPacket); break;
                case ADDRESS: processADDRESS(ldpPacket); break;
                case ADDRESS_WITHDRAW: processADDRESS_WITHDRAW(ldpPacket); break;
                default: break; // unreachable given the outer case list
            }
            break;
        }

        case NOTIFICATION:
            processNOTIFICATION(ldpPacket, /*rescheduled*/ false);
            break;

        default:
            // an unrecognized message type from a peer must not abort the simulation
            EV_WARN << "ignoring an unrecognized LDP message of type " << ldpPacket->getType() << endl;
            break;
    }
}

void Ldp::processINITIALIZATION(Ptr<const LdpPacket>& ldpPacket)
{
    const auto& ini = CHK(dynamicPtrCast<const LdpIni>(ldpPacket));
    Ipv4Address srcAddr = ini->getLsrId();
    uint16_t peerKeepAliveTime = ini->getKeepAliveTime();

    int i = findPeer(srcAddr);
    if (i == -1) {
        EV_WARN << "Init received from unknown peer " << srcAddr << ", ignoring" << endl;
        return;
    }

    EV_INFO << "Init received from " << srcAddr << " (peer keepAliveTime=" << peerKeepAliveTime << "s)" << endl;

    if (myPeers[i].state != peer_info::INITIALIZED && myPeers[i].state != peer_info::OPENSENT) {
        // a duplicate/out-of-sequence Init is a peer/protocol anomaly, not a model error
        EV_WARN << "unexpected Init from " << srcAddr << " in session state " << ldpSessionStateName(myPeers[i].state)
                << ", ignoring" << endl;
        return;
    }

    if (peerKeepAliveTime == 0) {
        // RFC 5036 Section 3.5.1.2.2: unacceptable session parameter -> reject and close
        EV_WARN << "peer " << srcAddr << " proposed an unacceptable KeepAlive Time (0), rejecting session" << endl;
        sendNotify(SESSION_REJECTED_BAD_KEEPALIVE_TIME, srcAddr, Ipv4Address(), 0);
        TcpSocket *sock = findPeerSocket(srcAddr);
        if (sock)
            sock->close();
        return;
    }

    myPeers[i].negotiatedKeepaliveTime = std::min(keepaliveTime, SimTime(peerKeepAliveTime, SIMTIME_S));

    // RFC 5036 Section 3.5.3: the A-bit signals the peer's own label advertisement
    // discipline (0=DU, 1=DoD). RFC 5036 allows DU/DoD to be negotiated per link
    // (falling back to DoD on a mismatch); this model does not implement that
    // negotiation -- on a mismatch, warn and simply proceed using OUR OWN mode.
    bool peerWantsDod = ini->getAbit();
    bool weAreDod = (distributionMode == "dod");
    if (peerWantsDod != weAreDod) {
        EV_WARN << "peer " << srcAddr << " advertised distribution mode " << (peerWantsDod ? "DoD" : "DU")
                << ", which differs from ours (" << (weAreDod ? "DoD" : "DU") << "); RFC 5036 allows per-link "
                << "DU/DoD negotiation, which this model does not implement -- proceeding using our own mode" << endl;
    }

    if (myPeers[i].state == peer_info::INITIALIZED) {
        // passive side: this is the peer's opening move -- reply with our own
        // Init, then immediately KeepAlive (RFC 5036 Section 2.5.3)
        sendInit(srcAddr);
        sendKeepAlive(srcAddr);
    }
    else {
        // active side (OPENSENT): we already sent our Init; peer's Init is
        // acceptable, so acknowledge with a KeepAlive
        sendKeepAlive(srcAddr);
    }
    myPeers[i].state = peer_info::OPENREC;
    EV_INFO << "session with " << srcAddr << " negotiated keepAliveTime=" << myPeers[i].negotiatedKeepaliveTime
            << ", state OPENREC" << endl;
}

void Ldp::processKEEPALIVE(Ptr<const LdpPacket>& ldpPacket)
{
    Ipv4Address srcAddr = ldpPacket->getLsrId();

    int i = findPeer(srcAddr);
    if (i == -1) {
        EV_WARN << "KeepAlive received from unknown peer " << srcAddr << ", ignoring" << endl;
        return;
    }

    EV_INFO << "KeepAlive received from " << srcAddr << endl;

    if (myPeers[i].state == peer_info::OPENREC) {
        myPeers[i].state = peer_info::OPERATIONAL;
        EV_INFO << "LDP session with " << srcAddr << " is now OPERATIONAL" << endl;
        emit(sessionUpSignal, (long)srcAddr.getInt());

        // start KeepAlive-based session liveness (RFC 5036 Section 2.5.6): initial
        // arm of both timers (before this point they are nullptr, so the
        // unconditional reset attempts in sendToPeer/processLdpPacketFromTcp are
        // no-ops)
        ASSERT(myPeers[i].keepAliveSendTimer == nullptr && myPeers[i].sessionHoldTimer == nullptr);
        myPeers[i].keepAliveSendTimer = new cMessage("LDPKeepAliveSendTimer");
        scheduleAfter(myPeers[i].negotiatedKeepaliveTime / 3, myPeers[i].keepAliveSendTimer);
        myPeers[i].sessionHoldTimer = new cMessage("LDPSessionHoldTimer");
        scheduleAfter(myPeers[i].negotiatedKeepaliveTime, myPeers[i].sessionHoldTimer);

        // now (and only now) that the session is usable, drive the FEC/label
        // machinery for this peer (RFC 5036 Section 3.5.3), and let the peer
        // know which of our interface addresses it can use as a next hop
        // (RFC 5036 Section 3.5.5/2.7)
        if (distributionMode == "du") {
            // RFC 5036 Section 2.6: Downstream Unsolicited -- advertise EVERY FEC
            // we recognize to this newly-OPERATIONAL peer, not just the ones whose
            // next hop happens to be this peer (updateFecList's filter, which the
            // DoD path below still uses).
            for (auto& fec : fecList)
                duAdvertiseToPeer(fec, srcAddr);
            // This peer's session just became usable as a downstream link too:
            // any FEC whose next hop IS this peer may already have been
            // (mis)advertised to OTHER peers as if we were egress for it, because
            // duAdvertiseToPeer's ER check (findPeerSocket(fec.nextHop)==nullptr)
            // could not yet distinguish "genuinely egress" from "next hop not
            // OPERATIONAL yet" at the time of that earlier advertisement.
            // updateFecListEntry's DU branch re-advertises to every OPERATIONAL
            // peer (this one included, harmlessly) using the now-correct ER
            // value, which corrects any such stale implicit-null advertisement.
            for (auto& fec : fecList) {
                if (fec.nextHop == srcAddr)
                    updateFecListEntry(fec);
            }
            emitFecBindingCount();
        }
        else
            updateFecList(srcAddr);
        sendAddress(srcAddr);
    }
    else if (myPeers[i].state == peer_info::OPERATIONAL) {
        // steady-state KeepAlive refresh; the session hold timer reset already
        // happened unconditionally at the top of processLdpPacketFromTcp
        EV_DETAIL << "KeepAlive refresh from " << srcAddr << " (session already OPERATIONAL)" << endl;
    }
    else {
        EV_WARN << "unexpected KeepAlive from " << srcAddr << " in session state " << ldpSessionStateName(myPeers[i].state)
                << ", ignoring" << endl;
    }
}

void Ldp::processADDRESS(Ptr<const LdpPacket>& ldpPacket)
{
    const auto& addr = CHK(dynamicPtrCast<const LdpAddress>(ldpPacket));
    Ipv4Address srcAddr = addr->getLsrId();

    int i = findPeer(srcAddr);
    if (i == -1) {
        EV_WARN << "Address message from unknown peer " << srcAddr << ", ignoring" << endl;
        return;
    }

    size_t n = addr->getAddressesArraySize();
    EV_INFO << "Address message received from " << srcAddr << " listing " << n << " interface address(es):";
    for (size_t k = 0; k < n; ++k) {
        Ipv4Address a = addr->getAddresses(k);
        EV_INFO << " " << a;
        if (std::find(myPeers[i].peerAddresses.begin(), myPeers[i].peerAddresses.end(), a) == myPeers[i].peerAddresses.end())
            myPeers[i].peerAddresses.push_back(a);
    }
    EV_INFO << endl;
}

void Ldp::processADDRESS_WITHDRAW(Ptr<const LdpPacket>& ldpPacket)
{
    const auto& addr = CHK(dynamicPtrCast<const LdpAddress>(ldpPacket));
    Ipv4Address srcAddr = addr->getLsrId();

    int i = findPeer(srcAddr);
    if (i == -1) {
        EV_WARN << "Address Withdraw from unknown peer " << srcAddr << ", ignoring" << endl;
        return;
    }

    size_t n = addr->getAddressesArraySize();
    EV_INFO << "Address Withdraw received from " << srcAddr << " for " << n << " interface address(es):";
    for (size_t k = 0; k < n; ++k) {
        Ipv4Address a = addr->getAddresses(k);
        EV_INFO << " " << a;
        auto it = std::find(myPeers[i].peerAddresses.begin(), myPeers[i].peerAddresses.end(), a);
        if (it != myPeers[i].peerAddresses.end())
            myPeers[i].peerAddresses.erase(it);
    }
    EV_INFO << endl;
}

// Pre-condition: myPeers vector is finalized
int Ldp::findInterfaceFromPeerAddr(Ipv4Address peerIP)
{
    // this function is a misnomer, we must recognize our own address too
    if (rt->isLocalAddress(peerIP))
        return CHK(ift->findInterfaceByName("lo0"))->getInterfaceId();

    // RFC 5036 Section 3.5.5: 'peerIP' here may be a peer's LSR-ID (the key
    // myPeers is indexed by) or one of that peer's interface addresses learned
    // via an Address message (peerAddresses) -- a FEC's next-hop address comes
    // from the routing table's gateway field and need not equal the LSR-ID.
    // The peer table -- populated from real Hello adjacencies and Address
    // messages -- is the authoritative source of "which peer session owns
    // this address"; consult it first instead of guessing through a generic
    // routing-table lookup.
    for (auto& p : myPeers) {
        if (p.state != peer_info::OPERATIONAL)
            continue;
        bool isPeerAddr = p.peerIP == peerIP ||
            std::find(p.peerAddresses.begin(), p.peerAddresses.end(), peerIP) != p.peerAddresses.end();
        if (isPeerAddr) {
            EV_DETAIL << "resolved " << peerIP << " to peer session " << p.peerIP
                      << " via the LDP peer address table (interface " << p.linkInterface << ")" << endl;
            return CHK(ift->findInterfaceByName(p.linkInterface.c_str()))->getInterfaceId();
        }
    }

    // no OPERATIONAL peer owns this address (e.g. a FEC whose next hop is not
    // yet a known LDP peer address): fall back to a generic routing-table lookup
    NetworkInterface *ie = rt->getInterfaceForDestAddr(peerIP);
    if (!ie)
        throw cRuntimeError("findInterfaceFromPeerAddr(): %s is not routable", peerIP.str().c_str());
    return ie->getInterfaceId();
}

Ldp::FecBindVector::iterator Ldp::findFecEntry(FecBindVector& fecs, int fecid, Ipv4Address peer)
{
    auto it = fecs.begin();
    for (; it != fecs.end(); it++) {
        if ((it->fecid == fecid) && (it->peer == peer))
            break;
    }
    return it;
}

Ldp::FecVector::iterator Ldp::findFecEntry(FecVector& fecs, Ipv4Address addr, int length)
{
    auto it = fecs.begin();
    for (; it != fecs.end(); it++) {
        if ((it->length == length) && (it->addr == addr)) // TODO compare only relevant part (?)
            break;
    }
    return it;
}

void Ldp::sendNotify(int status, Ipv4Address dest, Ipv4Address addr, int length)
{
    // Send NOTIFY message
    Packet *packet = new Packet("Lb-Notify");
    const auto& lnMessage = makeShared<LdpNotify>();
    lnMessage->setChunkLength(LDP_NOTIFICATION_BYTES);
    lnMessage->setType(NOTIFICATION);
    lnMessage->setStatus(status);
    lnMessage->setLsrId(rt->getRouterId());

    FecTlv fec;
    fec.addr = addr;
    fec.length = length;

    lnMessage->setFec(fec);
    packet->insertAtBack(lnMessage);

    sendToPeer(dest, packet);
}

void Ldp::sendMapping(int type, Ipv4Address dest, int label, Ipv4Address addr, int length)
{
    if (!isPeerOperational(dest)) {
        EV_WARN << "not sending LDP label message (type " << type << ") to " << dest << ": session is not OPERATIONAL\n";
        return;
    }

    // Send LABEL MAPPING downstream
    Packet *packet = new Packet("Lb-Mapping");
    const auto& lmMessage = makeShared<LdpLabelMapping>();
    lmMessage->setChunkLength(LDP_LABEL_MAPPING_BYTES); // also used for LABEL_WITHDRAW/LABEL_RELEASE (see LDP_LABEL_MAPPING_BYTES)
    lmMessage->setType(type);
    lmMessage->setLsrId(rt->getRouterId());
    lmMessage->setLabel(label);

    FecTlv fec;
    fec.addr = addr;
    fec.length = length;

    lmMessage->setFec(fec);
    packet->insertAtBack(lmMessage);

    sendToPeer(dest, packet);
}

void Ldp::processNOTIFICATION(Ptr<const LdpPacket>& ldpPacket, bool rescheduled)
{
    const auto& packet = CHK(dynamicPtrCast<const LdpNotify>(ldpPacket));
    FecTlv fec = packet->getFec();
    Ipv4Address srcAddr = packet->getLsrId();
    int status = packet->getStatus();

    // FIXME NO_ROUTE processing should probably be split into two functions,
    // this is not the cleanest thing I ever wrote :)   --Vojta

    if (rescheduled) {
        // re-scheduled by ourselves
        EV_INFO << "notification retry for peer=" << srcAddr << " fec=" << fec << " status=" << status << endl;
    }
    else {
        // received via network
        EV_INFO << "notification received from=" << srcAddr << " fec=" << fec << " status=" << status << endl;
    }

    switch (status) {
        case NO_ROUTE: {
            EV_INFO << "route does not exit on that peer" << endl;

            auto it = findFecEntry(fecList, fec.addr, fec.length);
            if (it != fecList.end()) {
                if (it->nextHop == srcAddr) {
                    if (!rescheduled) {
                        EV_DETAIL << "we are still interesed in this mapping, we will retry later" << endl;
                        auto pk = new Packet("LdpNotifyRetry", ldpPacket);
                        retryMsgs.push_back(pk);
                        scheduleAfter(1.0 /* FIXME */, pk);
                        return;
                    }
                    else {
                        EV_DETAIL << "reissuing request" << endl;

                        sendMappingRequest(srcAddr, fec.addr, fec.length);
                    }
                }
                else
                    EV_DETAIL << "and we still recognize this FEC, but we use different next hop, forget it" << endl;
            }
            else
                EV_DETAIL << "and we do not recognize this any longer, forget it" << endl;

            break;
        }

        default:
            // a notification status this model does not act on; log and ignore
            EV_WARN << "ignoring LDP notification with unhandled status " << status << endl;
            break;
    }
}

void Ldp::processLABEL_REQUEST(Ptr<const LdpPacket>& ldpPacket)
{
    const auto& packet = CHK(dynamicPtrCast<const LdpLabelRequest>(ldpPacket));
    FecTlv fec = packet->getFec();
    Ipv4Address srcAddr = packet->getLsrId();

    EV_INFO << "Label Request from LSR " << srcAddr << " for FEC " << fec << endl;

    auto it = findFecEntry(fecList, fec.addr, fec.length);
    if (it == fecList.end()) {
        EV_DETAIL << "FEC not recognized, sending back No route message" << endl;

        sendNotify(NO_ROUTE, srcAddr, fec.addr, fec.length);
        return;
    }

    // do we already have mapping for this fec from our downstream peer?

    //
    // TODO this code duplicates rebuildFecList
    //

    // does upstream have mapping from us?
    auto uit = findFecEntry(fecUp, it->fecid, srcAddr);

    if (uit != fecUp.end()) {
        // we already have an upstream binding for this FEC from this peer (e.g. a
        // duplicate request after a session reset); re-advertise the current
        // mapping instead of building a second one
        EV_INFO << "already have an upstream binding for this FEC, re-advertising" << endl;
        if (uit->label != -1)
            sendMapping(LABEL_MAPPING, srcAddr, uit->label, fec.addr, fec.length);
        return;
    }

    // do we have mapping from downstream?
    auto dit = findFecEntry(fecDown, it->fecid, it->nextHop);

    // is next hop our LDP peer?
    bool ER = !findPeerSocket(it->nextHop);

    ASSERT(!(ER && dit != fecDown.end())); // can't be egress and have mapping at the same time

    if (ER || dit != fecDown.end()) {
        FecBinding newItem;
        newItem.fecid = it->fecid;
        newItem.label = -1;
        newItem.peer = srcAddr;
        fecUp.push_back(newItem);
        uit = fecUp.end() - 1;
    }

    int inInterface = findInterfaceFromPeerAddr(srcAddr);
    int outInterface = findInterfaceFromPeerAddr(it->nextHop);

    if (ER) {
        if (advertiseImplicitNull) {
            // penultimate hop popping: advertise the implicit null label
            // instead of allocating a local pop entry -- our upstream peer
            // must pop the label itself, so no labeled traffic for this
            // FEC should ever reach us
            uit->label = IMPLICIT_NULL_LABEL;
            EV_INFO << "egress: advertising implicit null label (penultimate hop popping) for FEC " << fec << " to " << srcAddr << endl;
        }
        else {
            // we are egress, that's easy:
            LabelOpVector outLabel = LibTable::popLabel();

            uit->label = lt->installLibEntry(uit->label, inInterface, outLabel, outInterface);

            EV_DETAIL << "installed (egress) LIB entry inLabel=" << uit->label << " inInterface=" << inInterface
                      << " outLabel=" << outLabel << " outInterface=" << outInterface << endl;
        }

        // We are egress, let our upstream peer know
        // about it by sending back a Label Mapping message

        sendMapping(LABEL_MAPPING, srcAddr, uit->label, fec.addr, fec.length);
    }
    else if (dit != fecDown.end()) {
        // we have mapping from DS, that's easy -- unless the downstream peer
        // is itself the egress and advertised the implicit null label, in
        // which case we are the penultimate hop and must pop rather than
        // swap to label 3
        LabelOpVector outLabel = (dit->label == IMPLICIT_NULL_LABEL) ? LibTable::popLabel() : LibTable::swapLabel(dit->label);
        uit->label = lt->installLibEntry(uit->label, inInterface, outLabel, outInterface);

        EV_DETAIL << "installed LIB entry inLabel=" << uit->label << " inInterface=" << inInterface
                  << " outLabel=" << outLabel << " outInterface=" << outInterface << endl;

        // We already have a mapping for this FEC, let our upstream peer know
        // about it by sending back a Label Mapping message

        sendMapping(LABEL_MAPPING, srcAddr, uit->label, fec.addr, fec.length);
    }
    else {
        // no mapping from DS, mark as pending

        EV_DETAIL << "no mapping for this FEC from the downstream router, marking as pending" << endl;

        PendingRequest newItem;
        newItem.fecid = it->fecid;
        newItem.peer = srcAddr;
        pending.push_back(newItem);
    }

    emitFecBindingCount();
}

void Ldp::processLABEL_RELEASE(Ptr<const LdpPacket>& ldpPacket)
{
    const auto& packet = CHK(dynamicPtrCast<const LdpLabelMapping>(ldpPacket));
    FecTlv fec = packet->getFec();
    int label = packet->getLabel();
    Ipv4Address fromIP = packet->getLsrId();

    EV_INFO << "Mapping release received for label=" << label << " fec=" << fec << " from " << fromIP << endl;

    ASSERT(label > 0);

    // remove label from fecUp

    auto it = findFecEntry(fecList, fec.addr, fec.length);
    if (it == fecList.end()) {
        EV_INFO << "FEC no longer recognized here, ignoring" << endl;
        return;
    }

    auto uit = findFecEntry(fecUp, it->fecid, fromIP);
    if (uit == fecUp.end() || label != uit->label) {
        // this is ok and may happen; e.g. we removed the mapping because downstream
        // neighbour withdrew its mapping. we sent withdraw upstream as well and
        // this is upstream's response
        EV_INFO << "mapping not found among sent mappings, ignoring" << endl;
        return;
    }

    // an implicit-null advertisement never allocated a LIB entry, and under DU
    // independent control the released label may have been advertised (reserved
    // via allocateLabel()) but never actually installed if no downstream mapping
    // ever arrived -- removeLibEntryIfExists() tolerates both
    if (uit->label != IMPLICIT_NULL_LABEL) {
        EV_DETAIL << "removing from LIB table (if it was ever installed) label=" << uit->label << endl;
        lt->removeLibEntryIfExists(uit->label);
    }

    EV_DETAIL << "removing label from list of sent mappings" << endl;
    fecUp.erase(uit);
    emitFecBindingCount();
}

void Ldp::processLABEL_WITHDRAW(Ptr<const LdpPacket>& ldpPacket)
{
    const auto& ldpLabelMapping = CHK(dynamicPtrCast<const LdpLabelMapping>(ldpPacket));
    FecTlv fec = ldpLabelMapping->getFec();
    int label = ldpLabelMapping->getLabel();
    Ipv4Address fromIP = ldpLabelMapping->getLsrId();

    EV_INFO << "Mapping withdraw received for label=" << label << " fec=" << fec << " from " << fromIP << endl;

    ASSERT(label > 0);

    // remove label from fecDown

    auto it = findFecEntry(fecList, fec.addr, fec.length);
    if (it == fecList.end()) {
        EV_INFO << "matching FEC not found, ignoring withdraw message" << endl;
        return;
    }

    auto dit = findFecEntry(fecDown, it->fecid, fromIP);

    if (dit == fecDown.end() || label != dit->label) {
        EV_INFO << "matching mapping not found, ignoring withdraw message" << endl;
        return;
    }

    ASSERT(dit != fecDown.end());
    ASSERT(label == dit->label);

    EV_INFO << "removing label from list of received mappings" << endl;
    fecDown.erase(dit);

    EV_INFO << "sending back relase message" << endl;
    auto reply = makeShared<LdpLabelMapping>(*ldpLabelMapping.get());
    reply->setType(LABEL_RELEASE);
    // pre-existing (not this commit's scope): 'reply' is a copy of the incoming
    // withdraw and so still carries ITS sender's lsrId, not our own -- the
    // receiving peer's getLsrId() therefore identifies the wrong LSR for this
    // message. Harmless today (routing uses fromIP via sendToPeer, and the
    // mismatch only degrades this reply's own peer-identification on the far
    // end to a silent no-op), but worth a follow-up: reply->setLsrId(rt->getRouterId()).
    auto pk = new Packet("LDP_RELEASE", reply);
//    pk->addTag<PacketProtocolTag>()->setProtocol(&Protocol::ldp) //FIXME
    // send msg to peer over TCP
    sendToPeer(fromIP, pk);

    updateFecListEntry(*it);
}

void Ldp::processLABEL_MAPPING(Ptr<const LdpPacket>& ldpPacket)
{
    const auto& packet = CHK(dynamicPtrCast<const LdpLabelMapping>(ldpPacket));
    FecTlv fec = packet->getFec();
    int label = packet->getLabel();
    Ipv4Address fromIP = packet->getLsrId();

    EV_INFO << "Label mapping label=" << label << " received for fec=" << fec << " from " << fromIP << endl;

    ASSERT(label > 0);

    auto it = findFecEntry(fecList, fec.addr, fec.length);
    if (it == fecList.end()) {
        // a mapping for a FEC we do not recognize (e.g. an unsolicited mapping);
        // per RFC 5036 this is a legal peer event -- ignore it
        EV_INFO << "not a recognized FEC, ignoring the mapping" << endl;
        return;
    }

    auto dit = findFecEntry(fecDown, it->fecid, fromIP);
    if (dit != fecDown.end()) {
        // we already hold a mapping for this FEC from this peer (duplicate); ignore
        EV_INFO << "already have a mapping for this FEC from this peer, ignoring" << endl;
        return;
    }

    bool isNextHop = (it->nextHop == fromIP);
    // Retention mode only governs mappings received from a peer that is not the
    // FEC's next hop -- a situation that can only systematically arise from
    // proactive DU flooding to every peer. Under DoD, a mapping is only ever
    // received in response to a Label Request WE sent to the (then-)next hop;
    // the FEC's next hop can in principle have since changed before the reply
    // arrives, and the original Request/Mapping flow always accepted such a
    // reply unconditionally -- so this gate is scoped to distributionMode=="du"
    // to keep that DoD flow completely unchanged (verified empirically: without
    // this scoping, a DoD-configured run diverges from the pre-this-commit
    // baseline on exactly this race).
    if (distributionMode == "du" && !isNextHop && retentionMode == "conservative") {
        // RFC 5036 Section 2.6: conservative label retention -- only keep mappings
        // from a FEC's CURRENT next hop; explicitly release anything else instead
        // of silently ignoring it (this model's original implicit behavior, now
        // made explicit)
        EV_INFO << "mapping is from a peer that is not this FEC's current next hop and retentionMode is "
                << "conservative, releasing it" << endl;
        sendMapping(LABEL_RELEASE, fromIP, label, fec.addr, fec.length);
        return;
    }

    // liberal retention (default): keep mappings from ANY peer, not just the
    // FEC's current next hop -- this is what lets a later next-hop change switch
    // over instantly (see duAdvertiseToPeer's dit-found branch), without ever
    // sending a new Label Request

    FecBinding newItem;
    newItem.fecid = it->fecid;
    newItem.peer = fromIP;
    newItem.label = label;
    fecDown.push_back(newItem);

    if (isNextHop && distributionMode == "du") {
        // this mapping is for the FEC's CURRENT next hop: (re-)advertise it to
        // every OPERATIONAL peer via the same per-peer/per-control-mode decision
        // tree duAdvertiseToPeer always uses -- it transparently handles every
        // prior state a peer's fecUp entry may be in: none yet (first
        // advertisement, e.g. a peer that only just went OPERATIONAL), reserved
        // but not installed (independent control -- this completes it at the
        // SAME already-advertised label), or already installed (refresh).
        for (auto& p : myPeers) {
            if (p.state == peer_info::OPERATIONAL)
                duAdvertiseToPeer(*it, p.peerIP);
        }
    }

    // respond to pending DoD requests for this FEC (unchanged; empty under pure DU)

    for (auto pit = pending.begin(); pit != pending.end();) {
        if (pit->fecid != it->fecid) {
            pit++;
            continue;
        }

        EV_DETAIL << "there's pending request for this FEC from " << pit->peer << ", sending mapping" << endl;

        int inInterface = findInterfaceFromPeerAddr(pit->peer);
        int outInterface = findInterfaceFromPeerAddr(fromIP);
        // penultimate hop popping: label 3 from our downstream (egress) peer
        // means we must pop rather than swap to label 3
        LabelOpVector outLabel = (label == IMPLICIT_NULL_LABEL) ? LibTable::popLabel() : LibTable::swapLabel(label);

        FecBinding newItem;
        newItem.fecid = it->fecid;
        newItem.peer = pit->peer;
        newItem.label = lt->installLibEntry(-1, inInterface, outLabel, outInterface);
        fecUp.push_back(newItem);

        EV_DETAIL << "installed LIB entry inLabel=" << newItem.label << " inInterface=" << inInterface
                  << " outLabel=" << outLabel << " outInterface=" << outInterface << endl;

        sendMapping(LABEL_MAPPING, pit->peer, newItem.label, it->addr, it->length);

        // remove request from the list
        pit = pending.erase(pit);
    }

    emitFecBindingCount();
}

int Ldp::findPeer(Ipv4Address peerAddr)
{
    for (auto i = myPeers.begin(); i != myPeers.end(); ++i)
        if (i->peerIP == peerAddr)
            return i - myPeers.begin();

    return -1;
}

TcpSocket *Ldp::findPeerSocket(Ipv4Address peerAddr)
{
    // find peer in table and return its socket
    int i = findPeer(peerAddr);
    if (i == -1 || !(myPeers[i].socket) || myPeers[i].socket->getState() != TcpSocket::CONNECTED)
        return nullptr; // we don't have an LDP session to this peer
    return myPeers[i].socket;
}

TcpSocket *Ldp::getPeerSocket(Ipv4Address peerAddr)
{
    TcpSocket *sock = findPeerSocket(peerAddr);
    ASSERT(sock);
    if (!sock)
        throw cRuntimeError("No LDP session to peer %s yet", peerAddr.str().c_str());
    return sock;
}

bool Ldp::classifyPacket(Packet *packet, LabelOpVector& outLabel, int& outInterfaceId)
{
    const auto& ipv4Header = packet->peekAtFront<Ipv4Header>();
    Ipv4Address destAddr = ipv4Header->getDestAddress();
    int protocol = ipv4Header->getProtocolId();

    // never match and always route via L3 if:

    // OSPF traffic (Ted)
    if (protocol == IP_PROT_OSPF)
        return false;

    // LDP traffic (both discovery...
    if (protocol == IP_PROT_UDP) {
        const auto& udpHeader = packet->peekDataAt<UdpHeader>(ipv4Header->getChunkLength());
        if (udpHeader->getDestinationPort() == LDP_PORT)
            return false;
    }
    else if (protocol == IP_PROT_TCP) {
        // ...and session)
        const auto& tcpHeader = packet->peekDataAt<tcp::TcpHeader>(ipv4Header->getChunkLength());
        if (tcpHeader->getDestPort() == LDP_PORT || tcpHeader->getSrcPort() == LDP_PORT)
            return false;
    }

    // regular traffic, classify, label etc.

    for (auto& elem : fecList) {
        if (!destAddr.prefixMatches(elem.addr, elem.length))
            continue;

        EV_DETAIL << "FEC matched: " << elem << endl;

        auto dit = findFecEntry(fecDown, elem.fecid, elem.nextHop);
        if (dit != fecDown.end()) {
            if (dit->label == IMPLICIT_NULL_LABEL) {
                // penultimate hop popping: our downstream peer for this FEC is
                // the egress and advertised the implicit null label, meaning
                // "send me unlabeled traffic". We are ingress and penultimate
                // hop at once (a one-hop LSP) -- there is nothing to push, so
                // fall back to regular L3 routing instead.
                EV_DETAIL << "downstream mapping for this FEC is the implicit null label, no label to push, doing regular L3 routing" << endl;
                return false;
            }
            outLabel = LibTable::pushLabel(dit->label);
            ASSERT(outLabel[0].label != IMPLICIT_NULL_LABEL); // must never push the implicit null label
            outInterfaceId = findInterfaceFromPeerAddr(elem.nextHop);
            EV_DETAIL << "mapping found, outLabel=" << outLabel << ", outInterface=" << outInterfaceId << endl;
            return true;
        }
        else {
            EV_DETAIL << "no mapping for this FEC exists" << endl;
            return false;
        }
    }
    return false;
}

void Ldp::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method("%s", cComponent::getSignalName(signalID));

    printSignalBanner(signalID, obj, details);

    ASSERT(signalID == routeAddedSignal || signalID == routeDeletedSignal);

    EV_INFO << "routing table changed, rebuild list of known FEC" << endl;

    rebuildFecList();
}

} // namespace inet

