//
// Copyright (C) 2005-2010 Irene Ruengeler
// Copyright (C) 2009-2015 Thomas Dreibholz
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/applications/common/SocketTag_m.h"
#include "inet/common/IProtocolRegistrationListener.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/chunk/Chunk.h"
#include "inet/common/packet/Message.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/networklayer/contract/IL3AddressType.h"
#include "inet/transportlayer/common/L4Tools.h"
#include "inet/transportlayer/contract/sctp/SctpCommand_m.h"
#include "inet/transportlayer/contract/sctp/SctpSocket.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/transportlayer/sctp/Sctp.h"
#include "inet/transportlayer/sctp/SctpAssociation.h"
#include "inet/transportlayer/sctp/SctpHeaderSerializer.h"

#ifdef WITH_IPv4
#include "inet/networklayer/ipv4/Ipv4Header_m.h"
#endif // ifdef WITH_IPv4


namespace inet {

namespace sctp {

Define_Module(Sctp);

void Sctp::printInfoAssocMap()
{
    SctpAssociation *assoc;
    SockPair key;
    EV_DETAIL << "Number of Assocs: " << sizeAssocMap << "\n";
    if (sizeAssocMap > 0) {
        for (auto & elem : sctpAssocMap) {
            assoc = elem.second;
            key = elem.first;

            EV_DETAIL << "assocId: " << assoc->assocId << " src: " << key.localAddr << " dst: " << key.remoteAddr << " lPort: " << key.localPort << " rPort: " << key.remotePort << " fd: " << assoc->fd <<"\n";
        }

        EV_DETAIL << "\n";
    }
}

void Sctp::printVTagMap()
{
    int32 assocId;
    VTagPair key;
    EV_DETAIL << "Number of Assocs: " << sctpVTagMap.size() << "\n";
    if (sctpVTagMap.size() > 0) {
        for (auto & elem : sctpVTagMap) {
            assocId = elem.first;
            key = elem.second;

            EV_DETAIL << "assocId: " << assocId << " peerVTag: " << key.peerVTag
                      << " localVTag: " << key.localVTag
                      << " localPort: " << key.localPort << " rPort: " << key.remotePort << "\n";
        }

        EV_DETAIL << "\n";
    }
}

void Sctp::bindPortForUDP()
{
    EV_INFO << "Binding to UDP port " << SCTP_UDP_PORT << endl;

    udpSocket.setOutputGate(gate("ipOut"));
    udpSockId = getEnvir()->getUniqueNumber();
    EV_INFO << "UDP socket Id is " << udpSocket.getSocketId() << endl;
  //  udpSocket.bind(SCTP_UDP_PORT);
}

void Sctp::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        rt = getModuleFromPar<IRoutingTable>(par("routingTableModule"), this);
        auth = par("auth");
        pktdrop = par("packetDrop");
        sackNow = par("sackNow");
        numPktDropReports = 0;
        numPacketsReceived = 0;
        numPacketsDropped = 0;
        sizeAssocMap = 0;
        nextEphemeralPort = (uint16)(intrand(10000) + 30000);

        cModule *netw = getSimulation()->getSystemModule();
        if (netw->hasPar("testTimeout")) {
            testTimeout = netw->par("testTimeout");
        }
        const char *crcModeString = par("crcMode");
        crcMode = parseCrcMode(crcModeString, false);
        crcInsertion.setCrcMode(crcMode);
    }
    else if (stage == INITSTAGE_TRANSPORT_LAYER) {
        registerService(Protocol::sctp, gate("appIn"), gate("ipIn"));
        registerProtocol(Protocol::sctp, gate("ipOut"), gate("appOut"));
        if (crcMode == CRC_COMPUTED) {
#ifdef WITH_IPv4
            auto ipv4 = dynamic_cast<INetfilter *>(getModuleByPath("^.ipv4.ip"));
            if (ipv4 != nullptr) {
                ipv4->registerHook(0, &crcInsertion);
            }
#endif
#ifdef WITH_IPv6
            auto ipv6 = dynamic_cast<INetfilter *>(getModuleByPath("^.ipv6.ipv6"));
            if (ipv6 != nullptr)
                ipv6->registerHook(0, &crcInsertion);
#endif
        }
        if (par("udpEncapsEnabled")) {
            EV_INFO << "udpEncapsEnabled" << endl;
#ifdef WITH_IPv4
            auto ipv4 = dynamic_cast<INetfilter *>(getModuleByPath("^.ipv4.ip"));
            if (ipv4 != nullptr)
                ipv4->registerHook(0, &udpHook);
#endif
#ifdef WITH_IPv6
            auto ipv6 = dynamic_cast<INetfilter *>(getModuleByPath("^.ipv6.ipv6"));
            if (ipv6 != nullptr)
                ipv6->registerHook(0, &udpHook);
#endif
        }
    }
}

Sctp::~Sctp()
{
    EV_DEBUG << "delete SctpMain\n";
    if (!(sctpAppAssocMap.empty())) {
        EV_DEBUG << "clear appConnMap ptr=" << &sctpAppAssocMap << "\n";
        sctpAppAssocMap.clear();
    }
    if (!(assocStatMap.empty())) {
        EV_DEBUG << "clear assocStatMap ptr=" << &assocStatMap << "\n";
        assocStatMap.clear();
    }
    if (!(sctpVTagMap.empty())) {
        sctpVTagMap.clear();
    }
    EV_DEBUG << "after clearing maps\n";
}

void Sctp::handleMessage(cMessage *msg)
{
    L3Address destAddr;
    L3Address srcAddr;
    bool findListen = false;

    EV_INFO << "\n\nSctpMain handleMessage at " << getFullPath() << "\n";

    if (msg->isSelfMessage()) {
        EV_DEBUG << "selfMessage\n";

        SctpAssociation *assoc = (SctpAssociation *)msg->getContextPointer();
        if (assoc) {
            bool ret = assoc->processTimer(msg);

            if (!ret)
                removeAssociation(assoc);
        }
    }
    else if (msg->arrivedOn("ipIn")) {
        EV_INFO << "Message from IP\n";
        printInfoAssocMap();
        Packet *packet = check_and_cast<Packet *>(msg);
        auto protocol = packet->getTag<PacketProtocolTag>()->getProtocol();
        if (protocol == &Protocol::sctp) {
            // must be an SctpHeader
            SctpHeader *sctpmsg = (packet->peekAtFront<SctpHeader>().get()->dup());
            int chunkLength = B(sctpmsg->getChunkLength()).get();
            numPacketsReceived++;

            if (!pktdrop && (packet->hasBitError())) {
                EV_WARN << "Packet has bit-error. delete it\n";

                numPacketsDropped++;
                delete msg;
                return;
            }

            if (pktdrop && packet->hasBitError()) {
                EV_WARN << "Packet has bit-error. Call Pktdrop\n";
            }

            srcAddr = packet->getTag<L3AddressInd>()->getSrcAddress();
            destAddr = packet->getTag<L3AddressInd>()->getDestAddress();
            EV_INFO << "srcAddr=" << srcAddr << "   destAddr=" << destAddr << "\n";
            if (chunkLength > SCTP_COMMON_HEADER) {
                if (((sctpmsg->getSctpChunks(0)))->getSctpChunkType() == INIT || ((sctpmsg->getSctpChunks(0)))->getSctpChunkType() == INIT_ACK)
                    findListen = true;

                SctpAssociation *assoc = findAssocForMessage(srcAddr, destAddr, sctpmsg->getSrcPort(), sctpmsg->getDestPort(), findListen);
                if (!assoc && sctpAssocMap.size() > 0 && (((sctpmsg->getSctpChunks(0)))->getSctpChunkType() == INIT_ACK)) {
                    SctpInitAckChunk* initack = check_and_cast<SctpInitAckChunk *>((SctpChunk *)(sctpmsg->getSctpChunks(0)));
                    assoc = findAssocForInitAck(initack, srcAddr, destAddr, sctpmsg->getSrcPort(), sctpmsg->getDestPort(), findListen);
                }
                if (!assoc && sctpAssocMap.size() > 0 && (((sctpmsg->getSctpChunks(0)))->getSctpChunkType() == ERRORTYPE
                                                          || (sctpmsg->getSctpChunksArraySize() > 1 &&
                                                              (((sctpmsg->getSctpChunks(1)))->getSctpChunkType() == ASCONF || ((sctpmsg->getSctpChunks(1)))->getSctpChunkType() == ASCONF_ACK))))
                {
                    assoc = findAssocWithVTag(sctpmsg->getVTag(), sctpmsg->getSrcPort(), sctpmsg->getDestPort());
                }
                if (!assoc) {
                    EV_INFO << "no assoc found msg=" << sctpmsg->getName() << "\n";

                    if (!pktdrop && packet->hasBitError()) {
                        //delete sctpmsg;
                        return;
                    }

                    Ptr<SctpHeader> sctpmsgptr(sctpmsg);
                    if (((sctpmsg->getSctpChunks(0)))->getSctpChunkType() == SHUTDOWN_ACK)
                        sendShutdownCompleteFromMain(sctpmsgptr, destAddr, srcAddr);
                    else if (((sctpmsg->getSctpChunks(0)))->getSctpChunkType() != ABORT &&
                             ((sctpmsg->getSctpChunks(0)))->getSctpChunkType() != SHUTDOWN_COMPLETE) {
                        sendAbortFromMain(sctpmsgptr, destAddr, srcAddr);
                    }
                    delete packet;
                }
                else {
                    EV_INFO << "assoc " << assoc->assocId << " found\n";
                    bool ret = assoc->processSctpMessage(sctpmsg, srcAddr, destAddr);
                    if (!ret) {
                        EV_DEBUG << "SctpMain:: removeAssociation \n";
                        removeAssociation(assoc);
                        delete packet;
                    }
                    else {
                        delete packet;
                    }
                }
            }
        }
        else {
            delete packet;
        }
    }
    else {    // must be from app
        EV_DEBUG << "must be from app\n";
        auto& tags = getTags(msg);
        int32 assocId = tags.getTag<SocketReq>()->getSocketId();
        EV_INFO << "assocId = " << assocId << endl;
        if (msg->getKind() == SCTP_C_GETSOCKETOPTIONS) {
            auto controlInfo = tags.getTag<SctpSendReq>();
            Indication* cmsg = new Indication("SendSocketOptions", SCTP_I_SENDSOCKETOPTIONS);
            auto indication = cmsg->addTag<SctpCommandReq>();
            indication->setSocketId(controlInfo->getSocketId());
            socketOptions = collectSocketOptions();
            cmsg->setContextPointer((void*) socketOptions);
            cmsg->addTag<SocketInd>()->setSocketId(assocId);
            send(cmsg, "appOut");
            delete msg;
        } else {
        int32 appGateIndex;
            int32 fd;
                SctpCommandReq *controlInfo = tags.findTag<SctpOpenReq>();
                if (!controlInfo) {
                    controlInfo = tags.findTag<SctpSendReq>();
                    if (!controlInfo) {
                        controlInfo = tags.findTag<SctpCommandReq>();
                        if (!controlInfo) {
                            controlInfo = tags.findTag<SctpAvailableReq>();
                            if (!controlInfo) {
                                controlInfo = tags.findTag<SctpResetReq>();
                                if (!controlInfo) {
                                    controlInfo = tags.findTag<SctpInfoReq>();
                                    if (!controlInfo) {
                                        std::cout << "!!!!!!!!!Unknown Tag!!!!!!!!\n";
                                    }
                                }
                            }
                        }
                    }
                }
            if (controlInfo->getGate() != -1)
                appGateIndex = controlInfo->getGate();
            else
                appGateIndex = msg->getArrivalGate()->isVector() ? msg->getArrivalGate()->getIndex() : 0;
            if (controlInfo && assocId == -1) {
                fd = controlInfo->getFd();
                assocId = findAssocForFd(fd);
            }
            EV_INFO << "msg arrived from app for assoc " << assocId << "\n";
            SctpAssociation *assoc = findAssocForApp(appGateIndex, assocId);

            if (!assoc) {
                EV_INFO << "no assoc found. msg=" << msg->getName() << " number of assocs = " << assocList.size() << "\n";

                if (strcmp(msg->getName(), "PassiveOPEN") == 0 || strcmp(msg->getName(), "Associate") == 0) {
                    if (assocList.size() > 0) {
                        assoc = nullptr;
                        SctpOpenReq *open = tags.findTag<SctpOpenReq>();
                        EV_INFO << "Looking for assoc with remoteAddr=" << open->getRemoteAddr() << ", remotePort=" << open->getRemotePort() << ", localPort=" << open->getLocalPort() << "\n";
                        for (auto & elem : assocList) {
                            EV_DETAIL << "remoteAddr=" << (elem)->remoteAddr << ", remotePort=" << (elem)->remotePort << ", localPort=" << (elem)->localPort << "\n";
                            if ((elem)->remoteAddr == open->getRemoteAddr() && (elem)->localPort == open->getLocalPort() && (elem)->remotePort == open->getRemotePort()) {
                                assoc = (elem);
                                break;
                            }
                        }
                    }
                    if (assocList.size() == 0 || assoc == nullptr) {
                        assoc = new SctpAssociation(this, appGateIndex, assocId, rt, ift);

                        AppAssocKey key;
                        key.appGateIndex = appGateIndex;
                        key.assocId = assocId;
                        sctpAppAssocMap[key] = assoc;
                        EV_INFO << "SCTP association created for appGateIndex " << appGateIndex << " and assoc " << assocId << "\n";
                        bool ret = assoc->processAppCommand(msg, controlInfo);
                        if (!ret) {
                            removeAssociation(assoc);
                        }
                    }
                }
            } else {
                EV_INFO << "assoc found\n";
                bool ret = assoc->processAppCommand(msg, controlInfo);
                if (!ret) {
                    removeAssociation(assoc);
                }
            }
            delete msg;
        }
    }
}

SocketOptions* Sctp::collectSocketOptions()
{
    SocketOptions* sockOptions = new SocketOptions();
    sockOptions->maxInitRetrans = par("maxInitRetrans");
    sockOptions->maxInitRetransTimeout = SCTP_TIMEOUT_INIT_REXMIT_MAX;
    sockOptions->rtoInitial = par("rtoInitial");
    sockOptions->rtoMin = par("rtoMin");
    sockOptions->rtoMax = par("rtoMax");
    sockOptions->sackFrequency = par("sackFrequency");
    sockOptions->sackPeriod = par("sackPeriod");
    sockOptions->maxBurst = par("maxBurst");
    sockOptions->fragPoint = par("fragPoint");
    sockOptions->nagle = par("nagleEnabled").boolValue() ? 1 : 0;
    sockOptions->enableHeartbeats = par("enableHeartbeats");
    sockOptions->pathMaxRetrans = par("pathMaxRetrans");
    sockOptions->hbInterval = par("hbInterval");
    sockOptions->assocMaxRtx = par("assocMaxRetrans");
    return sockOptions;
}

void Sctp::sendAbortFromMain(Ptr<SctpHeader>& sctpmsg, L3Address fromAddr, L3Address toAddr)
{
    const auto& msg = makeShared<SctpHeader>();

    EV_DEBUG << "\n\nSctp::sendAbortFromMain()\n";

    msg->setSrcPort(sctpmsg->getDestPort());
    msg->setDestPort(sctpmsg->getSrcPort());
    msg->setChunkLength(B(SCTP_COMMON_HEADER));
    msg->setCrc(0);
    msg->setCrcMode(crcMode);
    msg->setChecksumOk(true);

    SctpAbortChunk *abortChunk = new SctpAbortChunk();
    abortChunk->setSctpChunkType(ABORT);
    if (sctpmsg->getSctpChunksArraySize() > 0 && ((sctpmsg->getSctpChunks(0)))->getSctpChunkType() == INIT) {
        const SctpInitChunk *initChunk = check_and_cast<const SctpInitChunk *>(sctpmsg->getSctpChunks(0));
        abortChunk->setT_Bit(0);
        msg->setVTag(initChunk->getInitTag());
    }
    else {
        abortChunk->setT_Bit(1);
        msg->setVTag(sctpmsg->getVTag());
    }
    abortChunk->setByteLength(SCTP_ABORT_CHUNK_LENGTH);
    msg->appendSctpChunks(abortChunk);
    Packet *pkt = new Packet("ABORT");

    auto addresses = pkt->addTag<L3AddressReq>();
    addresses->setSrcAddress(fromAddr);
    addresses->setDestAddress(toAddr);
    IL3AddressType *addressType = toAddr.getAddressType();
    pkt->addTag<DispatchProtocolReq>()->setProtocol(addressType->getNetworkProtocol());
    insertTransportProtocolHeader(pkt, Protocol::sctp, msg);
    send_to_ip(pkt);
}

void Sctp::sendShutdownCompleteFromMain(Ptr<SctpHeader>& sctpmsg, L3Address fromAddr, L3Address toAddr)
{
    const auto& msg = makeShared<SctpHeader>();

    EV_DEBUG << "\n\nSCTP:sendShutdownCompleteFromMain \n";

    msg->setSrcPort(sctpmsg->getDestPort());
    msg->setDestPort(sctpmsg->getSrcPort());
    msg->setChunkLength(b(SCTP_COMMON_HEADER));
    msg->setCrc(0);
    msg->setCrcMode(crcMode);
    msg->setChecksumOk(true);

    SctpShutdownCompleteChunk *scChunk = new SctpShutdownCompleteChunk();
    scChunk->setSctpChunkType(SHUTDOWN_COMPLETE);
    scChunk->setTBit(1);
    msg->setVTag(sctpmsg->getVTag());

    scChunk->setByteLength(SCTP_SHUTDOWN_ACK_LENGTH);
    msg->appendSctpChunks(scChunk);

    Packet *pkt = new Packet("SHUTDOWN_COMPLETE");
    auto addresses = pkt->addTag<L3AddressReq>();
    addresses->setSrcAddress(fromAddr);
    addresses->setDestAddress(toAddr);
    IL3AddressType *addressType = toAddr.getAddressType();
    pkt->addTag<DispatchProtocolReq>()->setProtocol(addressType->getNetworkProtocol());
    insertTransportProtocolHeader(pkt, Protocol::sctp, msg);
    send_to_ip(pkt);
}

void Sctp::send_to_ip(Packet *msg)
{
    EV_INFO << "send packet " << msg << " to IP\n";
    send(msg, "ipOut");
}

void Sctp::refreshDisplay() const
{
#if 0
    if (getEnvir()->disable_tracing) {
        // in express mode, we don't bother to update the display
        // (std::map's iteration is not very fast if map is large)
        getDisplayString().setTagArg("t", 0, "");
        return;
    }

    //char buf[40];
    //sprintf(buf,"%d conns", sctpAppConnMap.size());
    //displayString().setTagArg("t",0,buf);

    int32 numCLOSED = 0, numLISTEN = 0, numSYN_SENT = 0, numSYN_RCVD = 0,
          numESTABLISHED = 0, numCLOSE_WAIT = 0, numLAST_ACK = 0, numFIN_WAIT_1 = 0,
          numFIN_WAIT_2 = 0, numCLOSING = 0, numTIME_WAIT = 0;

    for (auto i = sctpAppConnMap.begin(); i != sctpAppConnMap.end(); ++i) {
        int32 state = (*i).second->getFsmState();
        switch (state) {
            // case SCTP_S_INIT:           numINIT++; break;
            case SCTP_S_CLOSED:
                numCLOSED++;
                break;

            case SCTP_S_COOKIE_WAIT:
                numLISTEN++;
                break;

            case SCTP_S_COOKIE_ECHOED:
                numSYN_SENT++;
                break;

            case SCTP_S_ESTABLISHED:
                numESTABLISHED++;
                break;

            case SCTP_S_SHUTDOWN_PENDING:
                numCLOSE_WAIT++;
                break;

            case SCTP_S_SHUTDOWN_SENT:
                numLAST_ACK++;
                break;

            case SCTP_S_SHUTDOWN_RECEIVED:
                numFIN_WAIT_1++;
                break;

            case SCTP_S_SHUTDOWN_ACK_SENT:
                numFIN_WAIT_2++;
                break;
        }
    }
    char buf2[300];
    buf2[0] = '\0';
    if (numCLOSED > 0)
        sprintf(buf2 + strlen(buf2), "closed:%d ", numCLOSED);
    if (numLISTEN > 0)
        sprintf(buf2 + strlen(buf2), "listen:%d ", numLISTEN);
    if (numSYN_SENT > 0)
        sprintf(buf2 + strlen(buf2), "syn_sent:%d ", numSYN_SENT);
    if (numSYN_RCVD > 0)
        sprintf(buf2 + strlen(buf2), "syn_rcvd:%d ", numSYN_RCVD);
    if (numESTABLISHED > 0)
        sprintf(buf2 + strlen(buf2), "estab:%d ", numESTABLISHED);
    if (numCLOSE_WAIT > 0)
        sprintf(buf2 + strlen(buf2), "close_wait:%d ", numCLOSE_WAIT);
    if (numLAST_ACK > 0)
        sprintf(buf2 + strlen(buf2), "last_ack:%d ", numLAST_ACK);
    if (numFIN_WAIT_1 > 0)
        sprintf(buf2 + strlen(buf2), "fin_wait_1:%d ", numFIN_WAIT_1);
    if (numFIN_WAIT_2 > 0)
        sprintf(buf2 + strlen(buf2), "fin_wait_2:%d ", numFIN_WAIT_2);
    if (numCLOSING > 0)
        sprintf(buf2 + strlen(buf2), "closing:%d ", numCLOSING);
    if (numTIME_WAIT > 0)
        sprintf(buf2 + strlen(buf2), "time_wait:%d ", numTIME_WAIT);
    getDisplayString().setTagArg("t", 0, buf2);
#endif // if 0
}

SctpAssociation *Sctp::findAssocWithVTag(uint32 peerVTag, uint32 remotePort, uint32 localPort)
{
    printVTagMap();
    EV_DEBUG << "findAssocWithVTag: peerVTag=" << peerVTag << " srcPort=" << remotePort << "    destPort=" << localPort << "\n";
    printInfoAssocMap();

    // try with fully qualified SockPair
    for (auto & elem : sctpVTagMap) {
        if ((elem.second.peerVTag == peerVTag && elem.second.localPort == localPort
             && elem.second.remotePort == remotePort)
            || (elem.second.localVTag == peerVTag && elem.second.localPort == localPort
                && elem.second.remotePort == remotePort))
            return getAssoc(elem.first);
    }
    return nullptr;
}

SctpAssociation *Sctp::findAssocForInitAck(SctpInitAckChunk *initAckChunk, L3Address srcAddr, L3Address destAddr, uint32 srcPort, uint32 destPort, bool findListen)
{
    SctpAssociation *assoc = nullptr;
    int numberAddresses = initAckChunk->getAddressesArraySize();
    for (int32 j = 0; j < numberAddresses; j++) {
        if (initAckChunk->getAddresses(j).getType() == L3Address::IPv6)
            continue;
        assoc = findAssocForMessage(initAckChunk->getAddresses(j), destAddr, srcPort, destPort, findListen);
        if (assoc) {
            break;
        }
    }
    return assoc;
}


SctpAssociation *Sctp::findAssocForMessage(L3Address srcAddr, L3Address destAddr, uint32 srcPort, uint32 destPort, bool findListen)
{
    SockPair key;

    key.localAddr = destAddr;
    key.remoteAddr = srcAddr;
    key.localPort = destPort;
    key.remotePort = srcPort;
    SockPair save = key;
    EV_DEBUG << "findAssocForMessage: srcAddr=" << destAddr << " destAddr=" << srcAddr << " srcPort=" << destPort << "  destPort=" << srcPort << "\n";
    printInfoAssocMap();

    // try with fully qualified SockPair
    auto i = sctpAssocMap.find(key);
    if (i != sctpAssocMap.end())
        return i->second;

    // try with localAddr missing (only localPort specified in passive/active open)
    key.localAddr = L3Address();

    i = sctpAssocMap.find(key);
    if (i != sctpAssocMap.end()) {
        // try with localAddr missing (only localPort specified in passive/active open)
        return i->second;
    }

    if (findListen == true) {
        // try fully qualified local socket + blank remote socket (for incoming SYN)
        key = save;
        key.remoteAddr = L3Address();
        key.remotePort = 0;
        i = sctpAssocMap.find(key);
        if (i != sctpAssocMap.end()) {
            // try fully qualified local socket + blank remote socket
            return i->second;
        }

        // try with blank remote socket, and localAddr missing (for incoming SYN)
        key.localAddr = L3Address();
        i = sctpAssocMap.find(key);
        if (i != sctpAssocMap.end()) {
            // try with blank remote socket, and localAddr missing
            return i->second;
        }
    }
    // given up

    EV_INFO << "giving up on trying to find assoc for localAddr=" << srcAddr << " remoteAddr=" << destAddr << " localPort=" << srcPort << " remotePort=" << destPort << "\n";
    return nullptr;
}

SctpAssociation *Sctp::findAssocForApp(int32 appGateIndex, int32 assocId)
{
    AppAssocKey key;
    key.appGateIndex = appGateIndex;
    key.assocId = assocId;
    EV_INFO << "findAssoc for appGateIndex " << appGateIndex << " and assoc " << assocId << "\n";
    auto i = sctpAppAssocMap.find(key);
    return (i == sctpAppAssocMap.end()) ? nullptr : i->second;
}

int32 Sctp::findAssocForFd(int32 fd)
{
    SctpAssociation *assoc = NULL;
    for (auto & elem : sctpAppAssocMap) {
        assoc = elem.second;
        if (assoc->fd == fd)
            return assoc->assocId;
    }
    return -1;
}

uint16 Sctp::getEphemeralPort()
{
    if (nextEphemeralPort == 5000)
        throw cRuntimeError("Ephemeral port range 1024..4999 exhausted (email SCTP model "
                            "author that he should implement reuse of ephemeral ports!!!)");
    return nextEphemeralPort++;
}

void Sctp::updateSockPair(SctpAssociation *assoc, L3Address localAddr, L3Address remoteAddr, int32 localPort, int32 remotePort)
{
    SockPair key;
    EV_INFO << "updateSockPair:   localAddr: " << localAddr << "   remoteAddr=" << remoteAddr << "    localPort=" << localPort << " remotePort=" << remotePort << "\n";

    key.localAddr = (assoc->localAddr = localAddr);
    key.remoteAddr = (assoc->remoteAddr = remoteAddr);
    key.localPort = assoc->localPort = localPort;
    key.remotePort = assoc->remotePort = remotePort;

    // Do not update a sock pair that is already stored
    for (auto & elem : sctpAssocMap) {
        if (elem.second == assoc &&
            elem.first.localAddr == key.localAddr &&
            elem.first.remoteAddr == key.remoteAddr &&
            elem.first.localPort == key.localPort
            && elem.first.remotePort == key.remotePort)
            return;
    }

    for (auto i = sctpAssocMap.begin(); i != sctpAssocMap.end(); i++) {
        if (i->second == assoc) {
            sctpAssocMap.erase(i);
            break;
        }
    }

    EV_INFO << "updateSockPair assoc=" << assoc->assocId << "    localAddr=" << key.localAddr << "            remoteAddr=" << key.remoteAddr << "     localPort=" << key.localPort << "  remotePort=" << remotePort << "\n";

    sctpAssocMap[key] = assoc;
    sizeAssocMap = sctpAssocMap.size();
    EV_DEBUG << "assoc inserted in sctpAssocMap\n";
    printInfoAssocMap();
}

void Sctp::addLocalAddress(SctpAssociation *assoc, L3Address address)
{
    SockPair key;

    key.localAddr = assoc->localAddr;
    key.remoteAddr = assoc->remoteAddr;
    key.localPort = assoc->localPort;
    key.remotePort = assoc->remotePort;

    auto i = sctpAssocMap.find(key);
    if (i != sctpAssocMap.end()) {
        ASSERT(i->second == assoc);
        if (key.localAddr.isUnspecified()) {
            sctpAssocMap.erase(i);
            sizeAssocMap--;
        }
    }
    else
        EV_INFO << "no actual sockPair found\n";
    key.localAddr = address;
    sctpAssocMap[key] = assoc;
    sizeAssocMap = sctpAssocMap.size();
    EV_INFO << "addLocalAddress " << address << " number of connections now=" << sizeAssocMap << "\n";

    printInfoAssocMap();
}

void Sctp::addLocalAddressToAllRemoteAddresses(SctpAssociation *assoc, L3Address address, std::vector<L3Address> remAddresses)
{
    SockPair key;

    for (auto & remAddresse : remAddresses) {
        //EV_DEBUG<<"remote address="<<(*i)<<"\n";
        key.localAddr = assoc->localAddr;
        key.remoteAddr = (remAddresse);
        key.localPort = assoc->localPort;
        key.remotePort = assoc->remotePort;

        auto j = sctpAssocMap.find(key);
        if (j != sctpAssocMap.end()) {
            ASSERT(j->second == assoc);
            if (key.localAddr.isUnspecified()) {
                sctpAssocMap.erase(j);
                sizeAssocMap--;
            }
        }
        else
            EV_INFO << "no actual sockPair found\n";
        key.localAddr = address;
        sctpAssocMap[key] = assoc;

        sizeAssocMap++;
        EV_DEBUG << "number of connections=" << sctpAssocMap.size() << "\n";

        printInfoAssocMap();
    }
}

void Sctp::removeLocalAddressFromAllRemoteAddresses(SctpAssociation *assoc, L3Address address, std::vector<L3Address> remAddresses)
{
    SockPair key;

    for (auto & remAddresse : remAddresses) {
        key.localAddr = address;
        key.remoteAddr = (remAddresse);
        key.localPort = assoc->localPort;
        key.remotePort = assoc->remotePort;

        auto j = sctpAssocMap.find(key);
        if (j != sctpAssocMap.end()) {
            ASSERT(j->second == assoc);
            sctpAssocMap.erase(j);
            sizeAssocMap--;
        }
        else
            EV_INFO << "no actual sockPair found\n";

        printInfoAssocMap();
    }
}

void Sctp::removeRemoteAddressFromAllAssociations(SctpAssociation *assoc, L3Address address, std::vector<L3Address> locAddresses)
{
    SockPair key;

    for (auto & locAddresse : locAddresses) {
        key.localAddr = (locAddresse);
        key.remoteAddr = address;
        key.localPort = assoc->localPort;
        key.remotePort = assoc->remotePort;

        auto j = sctpAssocMap.find(key);
        if (j != sctpAssocMap.end()) {
            ASSERT(j->second == assoc);
            sctpAssocMap.erase(j);
            sizeAssocMap--;
        }
        else
            EV_INFO << "no actual sockPair found\n";

        printInfoAssocMap();
    }
}

bool Sctp::addRemoteAddress(SctpAssociation *assoc, L3Address localAddress, L3Address remoteAddress)
{
    EV_INFO << "Add remote Address: " << remoteAddress << " to local Address " << localAddress << "\n";

    SockPair key;
    key.localAddr = localAddress;
    key.remoteAddr = remoteAddress;
    key.localPort = assoc->localPort;
    key.remotePort = assoc->remotePort;

    auto i = sctpAssocMap.find(key);
    if (i != sctpAssocMap.end()) {
        ASSERT(i->second == assoc);
        return false;
    }
    else {
        sctpAssocMap[key] = assoc;
        sizeAssocMap++;
    }

    printInfoAssocMap();
    return true;
}

void Sctp::addForkedAssociation(SctpAssociation *assoc, SctpAssociation *newAssoc, L3Address localAddr, L3Address remoteAddr, int32 localPort, int32 remotePort)
{
    SockPair keyAssoc;
    bool found = false;

    EV_INFO << "addForkedConnection assocId=" << assoc->assocId << "    newId=" << newAssoc->assocId << "\n";

    for (auto & elem : sctpAssocMap) {
        if (assoc->assocId == elem.second->assocId) {
            keyAssoc = elem.first;
            found = true;
            break;
        }
    }

    ASSERT(found == true);

    // update assoc's socket pair, and register newAssoc (which'll keep LISTENing)
    updateSockPair(assoc, localAddr, remoteAddr, localPort, remotePort);
    updateSockPair(newAssoc, keyAssoc.localAddr, keyAssoc.remoteAddr, keyAssoc.localPort, keyAssoc.remotePort);

    // assoc will get a new assocId...
    AppAssocKey key;
    key.appGateIndex = assoc->appGateIndex;
    key.assocId = assoc->assocId;
    sctpAppAssocMap.erase(key);
    assoc->listeningAssocId = assoc->assocId;
    int id = SctpSocket::getNewAssocId();
    EV_INFO << "id = " << id << endl;
    key.assocId = assoc->assocId = id;
    EV_INFO << "listeningAssocId set to " << assoc->listeningAssocId << " new assocId = " << assoc->assocId << endl;
    sctpAppAssocMap[key] = assoc;

    // ...and newAssoc will live on with the old assocId
    key.appGateIndex = newAssoc->appGateIndex;
    key.assocId = newAssoc->assocId;
    sctpAppAssocMap[key] = newAssoc;
    sizeAssocMap = sctpAssocMap.size();
    printInfoAssocMap();
}

void Sctp::removeAssociation(SctpAssociation *assoc)
{
    bool ok = false;
    bool find = false;
    const int32 id = assoc->assocId;

    EV_INFO << "Deleting SCTP connection " << assoc << " id= " << id << endl;

    printInfoAssocMap();
    if (sizeAssocMap > 0) {
        auto assocStatMapIterator = assocStatMap.find(assoc->assocId);
        if (assocStatMapIterator != assocStatMap.end()) {
            assocStatMapIterator->second.stop = simTime();
            assocStatMapIterator->second.lifeTime = assocStatMapIterator->second.stop - assocStatMapIterator->second.start;
            assocStatMapIterator->second.throughput = assocStatMapIterator->second.ackedBytes * 8 / assocStatMapIterator->second.lifeTime.dbl();
        }
        while (!ok) {
            if (sizeAssocMap == 0) {
                ok = true;
            }
            else {
                for (auto sctpAssocMapIterator = sctpAssocMap.begin();
                     sctpAssocMapIterator != sctpAssocMap.end(); sctpAssocMapIterator++)
                {
                    if (sctpAssocMapIterator->second != nullptr) {
                        SctpAssociation *myAssoc = sctpAssocMapIterator->second;
                        if (myAssoc->assocId == assoc->assocId) {
                            if (myAssoc->T1_InitTimer) {
                                myAssoc->stopTimer(myAssoc->T1_InitTimer);
                            }
                            if (myAssoc->T2_ShutdownTimer) {
                                myAssoc->stopTimer(myAssoc->T2_ShutdownTimer);
                            }
                            if (myAssoc->T5_ShutdownGuardTimer) {
                                myAssoc->stopTimer(myAssoc->T5_ShutdownGuardTimer);
                            }
                            if (myAssoc->SackTimer) {
                                myAssoc->stopTimer(myAssoc->SackTimer);
                            }
                            if (myAssoc->StartAddIP) {
                                myAssoc->stopTimer(myAssoc->StartAddIP);
                            }
                            sctpAssocMap.erase(sctpAssocMapIterator);
                            sizeAssocMap--;
                            find = true;
                            break;
                        }
                    }
                }
            }

            if (!find) {
                ok = true;
            }
            else {
                find = false;
            }
        }
    }
    // Write statistics
    char str[128];
    for (auto pathMapIterator = assoc->sctpPathMap.begin();
         pathMapIterator != assoc->sctpPathMap.end(); pathMapIterator++)
    {
        const SctpPathVariables *path = pathMapIterator->second;
        snprintf(str, sizeof(str), "Number of Fast Retransmissions %d:%s",
                assoc->assocId, path->remoteAddress.str().c_str());
        recordScalar(str, path->numberOfFastRetransmissions);
        snprintf(str, sizeof(str), "Number of Timer-Based Retransmissions %d:%s",
                assoc->assocId, path->remoteAddress.str().c_str());
        recordScalar(str, path->numberOfTimerBasedRetransmissions);
        snprintf(str, sizeof(str), "Number of Heartbeats Sent %d:%s",
                assoc->assocId, path->remoteAddress.str().c_str());
        recordScalar(str, path->numberOfHeartbeatsSent);
        snprintf(str, sizeof(str), "Number of Heartbeats Received %d:%s",
                assoc->assocId, path->remoteAddress.str().c_str());
        recordScalar(str, path->numberOfHeartbeatsRcvd);
        snprintf(str, sizeof(str), "Number of Heartbeat ACKs Sent %d:%s",
                assoc->assocId, path->remoteAddress.str().c_str());
        recordScalar(str, path->numberOfHeartbeatAcksSent);
        snprintf(str, sizeof(str), "Number of Heartbeat ACKs Received %d:%s",
                assoc->assocId, path->remoteAddress.str().c_str());
        recordScalar(str, path->numberOfHeartbeatAcksRcvd);
        snprintf(str, sizeof(str), "Number of Duplicates %d:%s",
                assoc->assocId, path->remoteAddress.str().c_str());
        recordScalar(str, path->numberOfDuplicates);
        snprintf(str, sizeof(str), "Number of Bytes received from %d:%s",
                assoc->assocId, path->remoteAddress.str().c_str());
        recordScalar(str, path->numberOfBytesReceived);
    }
    for (uint16 i = 0; i < assoc->inboundStreams; i++) {
        snprintf(str, sizeof(str), "Bytes received on stream %d of assoc %d",
                i, assoc->assocId);
        recordScalar(str, assoc->getState()->streamThroughput[i]);
    }
    recordScalar("Blocking TSNs Moved", assoc->state->blockingTsnsMoved);

    assoc->removePath();
    assoc->deleteStreams();

    // Chunks may be in the transmission and retransmission queues simultaneously.
    // Remove entry from transmission queue if it is already in the retransmission queue.
    for (auto i = assoc->getRetransmissionQueue()->payloadQueue.begin();
         i != assoc->getRetransmissionQueue()->payloadQueue.end(); i++)
    {
        auto j = assoc->getTransmissionQueue()->payloadQueue.find(i->second->tsn);
        if (j != assoc->getTransmissionQueue()->payloadQueue.end()) {
            assoc->getTransmissionQueue()->payloadQueue.erase(j);
        }
    }
    // Now, both queues can be safely deleted.
    delete assoc->getRetransmissionQueue();
    delete assoc->getTransmissionQueue();

    AppAssocKey key;
    key.appGateIndex = assoc->appGateIndex;
    key.assocId = assoc->assocId;
    sctpAppAssocMap.erase(key);
    assocList.remove(assoc);
    delete assoc;
}

SctpAssociation *Sctp::getAssoc(int32 assocId)
{
    for (auto & elem : sctpAppAssocMap) {
        if (elem.first.assocId == assocId)
            return elem.second;
    }
    return nullptr;
}

void Sctp::finish()
{
    auto assocMapIterator = sctpAssocMap.begin();
    while (assocMapIterator != sctpAssocMap.end()) {
        removeAssociation(assocMapIterator->second);
        assocMapIterator = sctpAssocMap.begin();
    }
    EV_INFO << getFullPath() << ": finishing SCTP with "
            << sctpAssocMap.size() << " connections open." << endl;

    for (AssocStatMap::const_iterator iterator = assocStatMap.begin();
         iterator != assocStatMap.end(); iterator++)
    {
        const Sctp::AssocStat& assoc = iterator->second;

        EV_DETAIL << "Association " << assoc.assocId << ": started at " << assoc.start
                  << " and finished at " << assoc.stop << " --> lifetime: " << assoc.lifeTime << endl;
        EV_DETAIL << "Association " << assoc.assocId << ": sent bytes=" << assoc.sentBytes
                  << ", acked bytes=" << assoc.ackedBytes << ", throughput=" << assoc.throughput << " bit/s" << endl;
        EV_DETAIL << "Association " << assoc.assocId << ": transmitted Bytes="
                  << assoc.transmittedBytes << ", retransmitted Bytes=" << assoc.transmittedBytes - assoc.ackedBytes << endl;
        EV_DETAIL << "Association " << assoc.assocId << ": number of Fast RTX="
                  << assoc.numFastRtx << ", number of Timer-Based RTX=" << assoc.numT3Rtx
                  << ", path failures=" << assoc.numPathFailures << ", ForwardTsns=" << assoc.numForwardTsn << endl;
        EV_DETAIL << "AllMessages=" << numPacketsReceived << " BadMessages=" << numPacketsDropped << endl;

        recordScalar("Association Lifetime", assoc.lifeTime);
        recordScalar("Acked Bytes", assoc.ackedBytes);
        recordScalar("Throughput [bit/s]", assoc.throughput);
        recordScalar("Transmitted Bytes", assoc.transmittedBytes);
        recordScalar("Fast RTX", assoc.numFastRtx);
        recordScalar("Timer-Based RTX", assoc.numT3Rtx);
        recordScalar("Duplicate Acks", assoc.numDups);
        recordScalar("Packets Received", numPacketsReceived);
        recordScalar("Packets Dropped", numPacketsDropped);
        recordScalar("Sum of R Gap Ranges", assoc.sumRGapRanges);
        recordScalar("Sum of NR Gap Ranges", assoc.sumNRGapRanges);
        recordScalar("Overfull SACKs", assoc.numOverfullSACKs);
        recordScalar("Drops Because New TSN Greater Than Highest TSN", assoc.numDropsBecauseNewTsnGreaterThanHighestTsn);
        recordScalar("Drops Because No Room In Buffer", assoc.numDropsBecauseNoRoomInBuffer);
        recordScalar("Chunks Reneged", assoc.numChunksReneged);
        recordScalar("sackPeriod", (simtime_t)socketOptions->sackPeriod);
        recordScalar("Number of AUTH chunks sent", assoc.numAuthChunksSent);
        recordScalar("Number of AUTH chunks accepted", assoc.numAuthChunksAccepted);
        recordScalar("Number of AUTH chunks rejected", assoc.numAuthChunksRejected);
        recordScalar("Number of StreamReset requests sent", assoc.numResetRequestsSent);
        recordScalar("Number of StreamReset requests performed", assoc.numResetRequestsPerformed);
        if (par("fairStart").doubleValue() > 0.0) {
            recordScalar("fair acked bytes", assoc.fairAckedBytes);
            recordScalar("fair start time", assoc.fairStart);
            recordScalar("fair stop time", assoc.fairStop);
            recordScalar("fair lifetime", assoc.fairLifeTime);
            recordScalar("fair throughput", assoc.fairThroughput);
        }
        recordScalar("Number of PacketDrop Reports", numPktDropReports);

        if (assoc.numEndToEndMessages > 0 && (assoc.cumEndToEndDelay / assoc.numEndToEndMessages) > 0) {
            uint32 msgnum = assoc.numEndToEndMessages - assoc.startEndToEndDelay;
            if (assoc.stopEndToEndDelay > 0)
                msgnum -= (assoc.numEndToEndMessages - assoc.stopEndToEndDelay);
            recordScalar("Average End to End Delay", assoc.cumEndToEndDelay / msgnum);
        }

        recordScalar("RTXMethod", par("RTXMethod").intValue());
    }
}

} // namespace sctp

} // namespace inet

