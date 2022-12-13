//
// Copyright 2008-2012 Irene Ruengeler
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/applications/sctpapp/SctpNatServer.h"

#include <stdio.h>
#include <stdlib.h>

#include "inet/applications/sctpapp/SctpNatPeer.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/packet/Message.h"
#include "inet/common/socket/SocketTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/sctp/SctpCommand_m.h"
#include "inet/transportlayer/contract/sctp/SctpSocket.h"
#include "inet/transportlayer/sctp/SctpAssociation.h"
#include "inet/transportlayer/sctp/SctpNatTable.h"

namespace inet {

using namespace sctp;

Define_Module(SctpNatServer);

void SctpNatServer::initialize(int stage)
{
    numSessions = packetsSent = packetsRcvd = bytesSent = notifications = 0;
    EV_DEBUG << "initialize SCTP NAT Server stage " << stage << endl;

    cSimpleModule::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        WATCH(numSessions);
        WATCH(packetsSent);
        WATCH(packetsRcvd);
        WATCH(bytesSent);
        WATCH(numRequestsToSend);
        inboundStreams = par("inboundStreams");
        outboundStreams = par("outboundStreams");
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER) {
        // parameters
        const char *addressesString = par("localAddress");
        AddressVector addresses = L3AddressResolver().resolve(cStringTokenizer(addressesString).asVector());
        int32_t port = par("localPort");

        ordered = par("ordered");
        lastStream = 0;

        socket = new SctpSocket();
        socket->setOutputGate(gate("socketOut"));
        socket->setInboundStreams(inboundStreams);
        socket->setOutboundStreams(outboundStreams);

        if (addresses.size() == 0)
            socket->bind(port);
        else
            socket->bindx(addresses, port);
        socket->listen(true, false, par("numPacketsToSendPerClient"));
        EV << "SctpNatServer::initialized listen port=" << port << "\n";

        shutdownReceived = false;
    }
}

void SctpNatServer::sendInfo(NatInfo *info)
{
    uint8_t buffer[100], buffer2[100];
    int buflen = 16;
    struct nat_message *nat = (struct nat_message *)(buffer);
    nat->peer1 = info->peer1;
    nat->peer2 = info->peer2;
    nat->portPeer1 = info->peer1Port;
    nat->portPeer2 = info->peer2Port;
    nat->numAddrPeer1 = 2;
    nat->numAddrPeer2 = 2;
    nat->multi = info->multi;
    buflen = ADD_PADDING(buflen + 4 * (nat->numAddrPeer1 + nat->numAddrPeer2));
    nat->peer1Addresses[0] = info->peer1Address1.toIpv4().getInt();
    nat->peer1Addresses[1] = info->peer1Address2.toIpv4().getInt();
    nat->peer2Addresses[0] = info->peer2Address1.toIpv4().getInt();
    nat->peer2Addresses[1] = info->peer2Address2.toIpv4().getInt();

    EV << "Info for peer1: peer1-1=" << Ipv4Address(nat->peer1Addresses[0]).str() << " peer2-1=" << Ipv4Address(nat->peer2Addresses[0]).str() << "\n";
    if (info->multi)
        EV << " peer1-2=" << Ipv4Address(nat->peer1Addresses[1]).str() << " peer2-2=" << Ipv4Address(nat->peer2Addresses[1]).str() << endl;

    auto applicationData = makeShared<BytesChunk>(buffer, buflen);
    applicationData->addTag<CreationTimeTag>()->setCreationTime(simTime());
    auto applicationPacket = new Packet("ApplicationPacket", SCTP_C_SEND_ORDERED);
    applicationPacket->insertAtBack(applicationData);
    auto sctpSendReq = applicationPacket->addTag<SctpSendReq>();
    sctpSendReq->setLast(true);
    sctpSendReq->setPrMethod(0);
    sctpSendReq->setPrValue(0);
    sctpSendReq->setSid(0);
    applicationPacket->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::sctp);
    applicationPacket->addTag<SocketReq>()->setSocketId(info->peer1Assoc);
    send(applicationPacket, "socketOut");
    EV << "info sent to peer1\n";

    EV << "SctpNatServer::shutdown peer1\n";

    Request *msg = new Request("SHUTDOWN", SCTP_C_SHUTDOWN);
    auto& cmd = msg->addTag<SctpCommandReq>();
    cmd->setSocketId(info->peer1Assoc);
    msg->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::sctp);
    msg->addTag<SocketReq>()->setSocketId(info->peer1Assoc);
    send(msg, "socketOut");
    EV << "abortMsg sent to peer1\n";

    struct nat_message *nat2 = (struct nat_message *)(buffer2);
    buflen = 16;
    nat2->peer1 = info->peer2;
    nat2->peer2 = info->peer1;
    nat2->portPeer1 = info->peer2Port;
    nat2->portPeer2 = info->peer1Port;
    nat2->numAddrPeer1 = 2;
    nat2->numAddrPeer2 = 2;
    nat2->multi = info->multi;
    buflen = ADD_PADDING(buflen + 4 * (nat2->numAddrPeer1 + nat2->numAddrPeer2));
    nat2->peer1Addresses[0] = info->peer2Address1.toIpv4().getInt();
    nat2->peer1Addresses[1] = info->peer2Address2.toIpv4().getInt();
    nat2->peer2Addresses[0] = info->peer1Address1.toIpv4().getInt();
    nat2->peer2Addresses[1] = info->peer1Address2.toIpv4().getInt();

    EV << "Info for peer2: peer1-1=" << Ipv4Address(nat2->peer1Addresses[0]).str() << " peer2-1=" << Ipv4Address(nat2->peer2Addresses[0]).str() << "\n";

    auto applicationData2 = makeShared<BytesChunk>(buffer2, buflen);
    applicationData2->addTag<CreationTimeTag>()->setCreationTime(simTime());
    auto applicationPacket2 = new Packet("ApplicationPacket", SCTP_C_SEND_ORDERED);
    applicationPacket2->insertAtBack(applicationData2);
    auto sctpSendReq2 = applicationPacket2->addTag<SctpSendReq>();
    sctpSendReq2->setLast(true);
    sctpSendReq2->setPrMethod(0);
    sctpSendReq2->setPrValue(0);
    sctpSendReq2->setSid(0);
    applicationPacket2->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::sctp);
    applicationPacket2->addTag<SocketReq>()->setSocketId(info->peer2Assoc);
    send(applicationPacket2, "socketOut");
    EV << "info sent to peer2\n";

    EV << "SctpNatServer::shutdown peer2\n";

    Request *msg2 = new Request("SHUTDOWN", SCTP_C_SHUTDOWN);
    auto& cmd2 = msg2->addTag<SctpCommandReq>();
    cmd2->setSocketId(info->peer2Assoc);
    msg2->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::sctp);
    msg2->addTag<SocketReq>()->setSocketId(info->peer2Assoc);
    send(msg2, "socketOut");
    EV << "abortMsg sent to peer2\n";
}

void SctpNatServer::generateAndSend()
{
    auto cmsg = new Packet("ApplicationPacket", SCTP_C_SEND);
    auto msg = makeShared<BytesChunk>();
    int numBytes = par("requestLength");
    std::vector<uint8_t> vec;
    vec.resize(numBytes);
    for (int i = 0; i < numBytes; i++)
        vec[i] = (numBytes + i) & 0xFF;
    msg->setBytes(vec);
    cmsg->insertAtBack(msg);

    auto cmd = cmsg->addTag<SctpSendReq>();
    cmd->setSocketId(assocId);
    cmd->setSendUnordered(ordered ? COMPLETE_MESG_ORDERED : COMPLETE_MESG_UNORDERED);
    lastStream = (lastStream + 1) % outboundStreams;
    cmd->setSid(lastStream);
    cmd->setLast(true);
    packetsSent++;
    bytesSent += numBytes;
    send(cmsg, "socketOut");
}

void SctpNatServer::handleMessage(cMessage *msg)
{
    int32_t id;

    if (msg->isSelfMessage()) {
        handleTimer(msg);
    }
    else {
        EV << "SctpNatServer::handleMessage kind=" << SctpAssociation::indicationName(msg->getKind()) << " (" << msg->getKind() << ")\n";
        switch (msg->getKind()) {
            case SCTP_I_PEER_CLOSED:
            case SCTP_I_ABORT: {
                Message *message = check_and_cast<Message *>(msg);
                assocId = message->getTag<SocketInd>()->getSocketId();
                auto& indtags = message->getTags();
                const auto& ind = indtags.findTag<SctpCommandReq>();

                Request *cmsg = new Request("SCTP_C_ABORT", SCTP_C_ABORT);
                auto& cmd = cmsg->addTag<SctpSendReq>();

                id = ind->getSocketId();
                cmd->setSocketId(id);
                cmd->setSid(ind->getSid());
                cmd->setNumMsgs(ind->getNumMsgs());
                delete msg;
                send(cmsg, "socketOut");
                break;
            }

            case SCTP_I_ESTABLISHED: {
                Message *message = check_and_cast<Message *>(msg);
                auto& tags = message->getTags();
                const auto& connectInfo = tags.findTag<SctpConnectReq>();
                numSessions++;
                assocId = connectInfo->getSocketId();
                id = assocId;
                inboundStreams = connectInfo->getInboundStreams();
                outboundStreams = connectInfo->getOutboundStreams();

                delete msg;

                break;
            }

            case SCTP_I_AVAILABLE: {
                EV_DETAIL << "SCTP_I_AVAILABLE arrived at server\n";
                Message *message = check_and_cast<Message *>(msg);
                int newSockId = message->getTag<SctpAvailableReq>()->getNewSocketId();
                EV_DETAIL << "new socket id = " << newSockId << endl;
                Request *cmsg = new Request("SCTP_C_ACCEPT_SOCKET_ID", SCTP_C_ACCEPT_SOCKET_ID);
                cmsg->addTag<SctpAvailableReq>()->setSocketId(newSockId);
                cmsg->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::sctp);
                cmsg->addTag<SocketReq>()->setSocketId(newSockId);
                EV_INFO << "Sending accept socket id request ..." << endl;

                send(cmsg, "socketOut");
                delete msg;
                break;
            }

            case SCTP_I_DATA_NOTIFICATION: {
                EV_DETAIL << "SCTP_I_DATA_NOTIFICATION arrived at server\n";
                notifications++;
                Message *message = check_and_cast<Message *>(msg);
                auto& intags = message->getTags();
                const auto& ind = intags.findTag<SctpCommandReq>();
                Request *cmsg = new Request("ReceiveRequest", SCTP_C_RECEIVE);
                auto cmd = cmsg->addTag<SctpSendReq>();
                id = ind->getSocketId();
                cmd->setSocketId(id);
                cmd->setSid(ind->getSid());
                cmd->setNumMsgs(ind->getNumMsgs());
                delete msg;
                cmsg->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::sctp);
                cmsg->addTag<SocketReq>()->setSocketId(id);
                send(cmsg, "socketOut");
                break;
            }

            case SCTP_I_DATA: {
                EV << "\nData arrived at server: assoc=" << assocId << "\n";
                printNatVector();
                Packet *message = check_and_cast<Packet *>(msg);
                auto& tags = message->getTags();
                const auto& ind = tags.findTag<SctpRcvReq>();
                id = ind->getSocketId();
                const auto& smsg = message->peekDataAsBytes();
                int bufferlen = B(smsg->getChunkLength()).get();
                uint8_t buffer[bufferlen];
                std::vector<uint8_t> vec = smsg->getBytes();
                for (int i = 0; i < bufferlen; i++) {
                    buffer[i] = vec[i];
                }
                struct nat_message *nat = (struct nat_message *)buffer;
                bool found = false;
                if (natVector.size() > 0) {
                    for (auto& elem : natVector) {
                        if ((elem)->peer1 == nat->peer1 || (elem)->peer1Assoc == assocId) {
                            EV << "found entry: info: Peer1 = " << nat->peer1 << "  peer1Address1=" << nat->peer1Addresses[0] << " peer2=" << nat->peer2 << " peer2Address1=" << nat->peer2Addresses[0] << "\n";
                            if (nat->multi && nat->numAddrPeer1 > 1 && nat->numAddrPeer2 > 1) {
                                EV << " peer1Address2=" << Ipv4Address(nat->peer1Addresses[1]).str() << " peer2Address2=" << Ipv4Address(nat->peer2Addresses[1]).str() << endl;
                            }
                            if ((elem)->peer1 == 0 && !(elem)->peer1Address2.isUnspecified()) {
                                (elem)->peer1 = nat->peer1;
                                (elem)->peer1Address1 = ind->getRemoteAddr();
                                (elem)->peer1Port = nat->portPeer1;
                                if ((elem)->peer2 == 0) {
                                    (elem)->peer2 = nat->peer2;
                                }
                            }
                            if (nat->multi && (elem)->peer1Address2.isUnspecified()) {
                                (elem)->peer1Address2 = ind->getRemoteAddr();
                            }
                            if (!(elem)->peer2Address1.isUnspecified() && !(elem)->peer1Address1.isUnspecified()) {
                                if (!(elem)->multi || ((elem)->multi && !(elem)->peer2Address2.isUnspecified() && !(elem)->peer1Address2.isUnspecified())) {
                                    EV << "entry now: Peer1=" << (elem)->peer1 << " Peer2=" << (elem)->peer2 << " peer1Address1=" << (elem)->peer1Address1 << " peer1Address2=" << (elem)->peer1Address2 << " peer2Address1=" << (elem)->peer2Address1 << " peer2Address2=" << (elem)->peer2Address2 << " peer2Port=" << (elem)->peer2Port << "\n";
                                    sendInfo((elem));
                                }
                            }
                            found = true;
                            break;
                        }
                        if ((elem)->peer2 == nat->peer1 || (elem)->peer2Assoc == assocId) {
                            EV << "opposite way:  info: Peer1 = " << nat->peer1 << "  peer1Address1=" << Ipv4Address(nat->peer1Addresses[0]).str() << " peer2=" << nat->peer2 << " peer2Address1=" << Ipv4Address(nat->peer2Addresses[0]).str() << "\n";
                            if (nat->multi && nat->numAddrPeer1 > 1 && nat->numAddrPeer2 > 1) {
                                EV << " peer1Address2=" << Ipv4Address(nat->peer1Addresses[1]).str() << " peer2Address2=" << Ipv4Address(nat->peer2Addresses[1]).str() << endl;
                            }
                            if ((elem)->peer2 == 0) {
                                (elem)->peer2 = nat->peer1;
                            }
                            if ((elem)->peer1 == 0) {
                                (elem)->peer1 = nat->peer2;
                            }
                            if ((elem)->peer2Address1.isUnspecified()) {
                                (elem)->peer2Address1 = ind->getRemoteAddr();
                                (elem)->peer2Assoc = assocId;
                                (elem)->peer2Port = nat->portPeer1;
                                (elem)->peer2Gate = ind->getGate();
                                EV << "set peer2Address1=" << ind->getRemoteAddr() << " peer2Assoc=" << assocId << " peer2Port=" << nat->portPeer1 << "\n";
                            }
                            else if ((elem)->multi && !(elem)->peer2Address2.isUnspecified())
                                (elem)->peer2Address2 = ind->getRemoteAddr();

                            if (!(elem)->multi || ((elem)->multi && !(elem)->peer2Address2.isUnspecified() && !(elem)->peer1Address2.isUnspecified()
                                                   && !(elem)->peer2Address1.isUnspecified() && !(elem)->peer1Address1.isUnspecified()))
                            {
                                EV << "entry now: Peer1=" << (elem)->peer1 << " Peer2=" << (elem)->peer2 << " peer1Address1=" << (elem)->peer1Address1 << " peer1Address2=" << (elem)->peer1Address2 << " peer2Address1=" << (elem)->peer2Address1 << " peer2Address2=" << (elem)->peer2Address2 << " peer1Port=" << (elem)->peer1Port << " peer2Port=" << (elem)->peer2Port << "\n";
                                sendInfo((elem));
                            }
                            found = true;
                            break;
                        }
                    }
                }
                if (natVector.size() == 0 || !found) {
                    EV << "make new Info for ";
                    NatInfo *info = new NatInfo();
                    info->peer1 = nat->peer1;
                    EV << info->peer1 << " and assoc " << assocId << "\n";
                    info->multi = nat->multi;
                    info->peer1Address1 = ind->getRemoteAddr();
                    if (info->multi) {
                        info->peer1Address2 = L3Address();
                        info->peer2Address2 = L3Address();
                    }
                    info->peer1Port = nat->portPeer1;
                    info->peer1Assoc = assocId;
                    info->peer1Gate = ind->getGate();
                    info->peer2 = nat->peer2;
                    info->peer2Address1 = L3Address();
                    info->peer2Port = 0;
                    info->peer2Assoc = 0;
                    info->peer2Gate = -1;
                    natVector.push_back(info);
                    EV << "Info: peer1=" << info->peer1 << " peer1Address1=" << info->peer1Address1 << " peer1Address2=" << info->peer1Address2 << " peer1Assoc=" << info->peer1Assoc << "\n peer2=" << info->peer2 << " peer2Address1=" << info->peer2Address1 << " peer2Address2=" << info->peer2Address2 << " peer2Assoc=" << info->peer2Assoc << "\n";
                }
                EV << "\n";
                printNatVector();

                delete msg;
                break;
            }

            case SCTP_I_SHUTDOWN_RECEIVED: {
                Message *message = check_and_cast<Message *>(msg);
                id = message->getTag<SocketInd>()->getSocketId();
                EV << "server: SCTP_I_SHUTDOWN_RECEIVED for assoc " << id << "\n";
                Request *cmsg = new Request("SCTP_C_NO_OUTSTANDING", SCTP_C_NO_OUTSTANDING);
                auto& qinfo = cmsg->addTag<SctpCommandReq>();
                qinfo->setSocketId(id);
                send(cmsg, "socketOut");

//                delete command;
                shutdownReceived = true;
                delete msg;
                break;
            }

            case SCTP_I_SEND_STREAMS_RESETTED:
            case SCTP_I_RCV_STREAMS_RESETTED: {
                EV << "Streams have been resetted\n";
                delete msg;
                break;
            }

            case SCTP_I_CLOSED:
                delete msg;
                break;

            case SCTP_I_ADDRESS_ADDED: {
                Message *message = check_and_cast<Message *>(msg);
                auto& intags = message->getTags();
                const auto& ind = intags.findTag<SctpCommandReq>();
                bool found = false;
                printNatVector();
                EV << " address added: LOCAL=" << ind->getLocalAddr() << ", remote=" << ind->getRemoteAddr() << " assoc=" << assocId << "\n";
                if (natVector.size() > 0) {
                    for (auto& elem : natVector) {
                        if ((elem)->peer1Assoc == assocId) {
                            EV << "found entry for assoc1 = " << assocId << "  Peer1 = " << (elem)->peer1 << "  peer1Address1=" << (elem)->peer1Address1 << " peer1Address2=" << (elem)->peer1Address2 << " peer2=" << (elem)->peer2 << " peer2Address1=" << (elem)->peer2Address1 << " peer2Address2=" << (elem)->peer2Address2 << "\n";
                            if ((elem)->multi && (elem)->peer1Address2.isUnspecified()) {
                                (elem)->peer1Address2 = ind->getRemoteAddr();
                                EV << "added peer1Address2=" << ind->getRemoteAddr() << "\n";
                            }
                            if (!(elem)->peer2Address1.isUnspecified()) {
                                if (!(elem)->multi || ((elem)->multi && !(elem)->peer2Address2.isUnspecified() && !(elem)->peer1Address2.isUnspecified())) {
                                    EV << "entry now: Peer1=" << (elem)->peer1 << " Peer2=" << (elem)->peer2 << " peer1Address1=" << (elem)->peer1Address1 << " peer1Address2=" << (elem)->peer1Address2 << " peer2Address1=" << (elem)->peer2Address1 << " peer2Address2=" << (elem)->peer2Address2 << " peer2Port=" << (elem)->peer2Port << "\n";
                                    sendInfo((elem));
                                }
                            }
                            found = true;
                            break;
                        }
                        else if ((elem)->peer2Assoc == assocId) {
                            EV << "opposite way: found entry for assoc2 = " << assocId << "  peer1Address1=" << (elem)->peer1Address1 << " peer1Address2=" << (elem)->peer1Address2 << " peer2Address1=" << (elem)->peer2Address1 << " peer2Address2=" << (elem)->peer2Address2 << "\n";
                            if ((elem)->multi)
                                (elem)->peer2Address2 = ind->getRemoteAddr();

                            if (!(elem)->multi || ((elem)->multi && !(elem)->peer2Address2.isUnspecified() && !(elem)->peer1Address2.isUnspecified())) {
                                EV << "entry now: Peer1=" << (elem)->peer1 << " Peer2=" << (elem)->peer2 << " peer1Address1=" << (elem)->peer1Address1 << " peer1Address2=" << (elem)->peer1Address2 << " peer2Address1=" << (elem)->peer2Address1 << " peer2Address2=" << (elem)->peer2Address2 << " peer1Port=" << (elem)->peer1Port << "peer2Port=" << (elem)->peer2Port << "\n";
                                sendInfo((elem));
                            }
                            found = true;
                            break;
                        }
                        else if ((elem)->peer2Assoc == 0 && ((elem)->multi)) {
                            (elem)->peer2Address2 = ind->getRemoteAddr();
                            (elem)->peer2Assoc = assocId;
                            (elem)->peer2Port = ind->getRemotePort();
                            (elem)->peer2Gate = ind->getGate();
                            EV << "entry now: Peer1=" << (elem)->peer1 << " Peer2=" << (elem)->peer2 << " peer1Address1=" << (elem)->peer1Address1 << " peer1Address2=" << (elem)->peer1Address2 << " peer2Address1=" << (elem)->peer2Address1 << " peer2Address2=" << (elem)->peer2Address2 << " peer1Port=" << (elem)->peer1Port << "peer2Port=" << (elem)->peer2Port << "\n";

                            found = true;
                        }
                    }
                }
                else if (natVector.size() == 0 && !found) {
                    EV << "make new Info for ";
                    NatInfo *info = new NatInfo();
                    info->peer1 = 0;
                    info->peer1Assoc = assocId;
                    EV << info->peer1 << " and assoc " << assocId << "\n";
                    info->multi = 1;
                    info->peer1Address1 = L3Address();
                    info->peer1Address2 = ind->getRemoteAddr();
                    info->peer1Port = ind->getRemotePort();
                    info->peer1Gate = ind->getGate();
                    info->peer2 = 0;
                    info->peer2Address1 = L3Address();
                    info->peer2Address2 = L3Address();
                    info->peer2Port = 0;
                    info->peer2Assoc = 0;
                    info->peer2Gate = -1;
                    natVector.push_back(info);
                    EV << "Info: peer1=" << info->peer1 << " peer1Address1=" << info->peer1Address1 << " peer1Address2=" << info->peer1Address2 << " peer1Assoc=" << info->peer1Assoc << "\n peer2=" << info->peer1 << " peer2Address1=" << info->peer2Address1 << " peer2Address2=" << info->peer2Address2 << " peer2Assoc=" << info->peer2Assoc << "\n";
                }
                delete msg;
                printNatVector();
                break;
            }

            default:
                EV << "Message type " << SctpAssociation::indicationName(msg->getKind()) << " not implemented\n";
                delete msg;
        }
    }
}

void SctpNatServer::handleTimer(cMessage *msg)
{
    int32_t id;

    switch (msg->getKind()) {
        case SCTP_C_SEND:
            if (numRequestsToSend > 0) {
                generateAndSend();
                numRequestsToSend--;
            }
            break;

        case SCTP_I_ABORT: {
            Request *cmsg = new Request("SCTP_C_CLOSE", SCTP_C_CLOSE);
            auto& cmd = cmsg->addTag<SctpCommandReq>();
            id = atoi(msg->getName());
            cmd->setSocketId(id);
            send(cmsg, "socketOut");
            break;
        }

        case SCTP_C_RECEIVE:
            send(msg, "socketOut");
            break;

        default:
            break;
    }
}

void SctpNatServer::printNatVector(void)
{
    for (auto& elem : natVector) {
        EV << "Peer1: " << (elem)->peer1 << " Assoc: " << (elem)->peer1Assoc << " Address1: " << (elem)->peer1Address1 << " Address2: " << (elem)->peer1Address2 << "Port: " << (elem)->peer1Port << endl;
        EV << "Peer2: " << (elem)->peer2 << " Assoc: " << (elem)->peer2Assoc << " Address1: " << (elem)->peer2Address1 << " Address2: " << (elem)->peer2Address2 << "Port: " << (elem)->peer2Port << endl;
    }
}

void SctpNatServer::finish()
{
    EV << getFullPath() << ": opened " << numSessions << " sessions\n";
    EV << getFullPath() << ": sent " << bytesSent << " bytes in " << packetsSent << " packets\n";

    EV << getFullPath() << "Over all " << packetsRcvd << " packets received\n ";
    EV << getFullPath() << "Over all " << notifications << " notifications received\n ";
}

} // namespace inet

