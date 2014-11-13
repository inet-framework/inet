//
// Copyright 2008-2012 Irene Ruengeler
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

#include <stdlib.h>
#include <stdio.h>

#include "inet/applications/sctpapp/SCTPNatServer.h"

#include "inet/transportlayer/sctp/SCTPAssociation.h"
#include "inet/transportlayer/contract/sctp/SCTPCommand_m.h"
#include "inet/transportlayer/sctp/SCTPMessage_m.h"
#include "inet/transportlayer/contract/sctp/SCTPSocket.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/sctp/SCTPNatTable.h"

namespace inet {

using namespace sctp;

Define_Module(SCTPNatServer);

NatVector SCTPNatServer::natVector;

void SCTPNatServer::initialize()
{
    numSessions = packetsSent = packetsRcvd = bytesSent = notifications = 0;
    WATCH(numSessions);
    WATCH(packetsSent);
    WATCH(packetsRcvd);
    WATCH(bytesSent);
    WATCH(numRequestsToSend);

    // parameters
    const char *addressesString = par("localAddress");
    AddressVector addresses = L3AddressResolver().resolve(cStringTokenizer(addressesString).asVector());
    int32 port = par("localPort");
    inboundStreams = par("inboundStreams");
    outboundStreams = par("outboundStreams");
    ordered = (bool)par("ordered");
    lastStream = 0;

    socket = new SCTPSocket();
    socket->setOutputGate(gate("sctpOut"));
    socket->setInboundStreams(inboundStreams);
    socket->setOutboundStreams(outboundStreams);

    if (addresses.size() == 0)
        socket->bind(port);
    else
        socket->bindx(addresses, port);
    socket->listen(true, false, par("numPacketsToSendPerClient"));
    EV << "SCTPNatServer::initialized listen port=" << port << "\n";

    shutdownReceived = false;
}

void SCTPNatServer::sendInfo(NatInfo *info)
{
    NatMessage *msg = new NatMessage("Rendezvous");
    msg->setKind(SCTP_C_NAT_INFO);
    msg->setMulti(info->multi);
    msg->setPeer1(info->peer1);
    msg->setPeer1AddressesArraySize(2);
    msg->setPeer1Addresses(0, info->peer1Address1);
    msg->setPeer1Addresses(1, info->peer1Address2);
    msg->setPortPeer1(info->peer1Port);
    msg->setPeer2(info->peer2);
    msg->setPeer2AddressesArraySize(2);
    msg->setPeer2Addresses(0, info->peer2Address1);
    msg->setPeer2Addresses(1, info->peer2Address2);
    msg->setPortPeer2(info->peer2Port);
    EV << "Info for peer1: peer1-1=" << msg->getPeer1Addresses(0) << " peer2-1=" << msg->getPeer2Addresses(0) << "\n";
    if (info->multi)
        EV << " peer1-2=" << msg->getPeer1Addresses(1) << " peer2-2=" << msg->getPeer2Addresses(1) << endl;
    cPacket *cmsg = new cPacket(msg->getName());
    SCTPSimpleMessage *smsg = new SCTPSimpleMessage("nat_data");
    smsg->setEncaps(true);
    smsg->encapsulate(msg);
    smsg->setCreationTime(simulation.getSimTime());
    smsg->setByteLength(16);
    smsg->setDataLen(16);
    cmsg->encapsulate(PK(smsg));
    SCTPSendCommand *cmd = new SCTPSendCommand();
    cmd->setAssocId(info->peer1Assoc);
    cmd->setGate(info->peer1Gate);
    cmd->setSendUnordered(COMPLETE_MESG_UNORDERED);
    cmd->setSid(0);
    cmd->setLast(true);
    cmsg->setKind(SCTP_C_SEND);
    cmsg->setControlInfo(cmd);
    send(cmsg, "sctpOut");
    EV << "info sent to peer1\n";

    cPacket *abortMsg = new cPacket("abortPeer1", SCTP_C_SHUTDOWN);
    abortMsg->setControlInfo(cmd->dup());
    send(abortMsg, "sctpOut");
    EV << "abortMsg sent to peer1\n";

    msg = new NatMessage("Rendezvous");
    msg->setKind(SCTP_C_NAT_INFO);
    msg->setMulti(info->multi);
    msg->setPeer1(info->peer2);
    msg->setPeer1AddressesArraySize(2);
    msg->setPeer1Addresses(0, info->peer2Address1);
    msg->setPeer1Addresses(1, info->peer2Address2);
    msg->setPortPeer1(info->peer2Port);
    msg->setPeer2(info->peer1);
    msg->setPeer2AddressesArraySize(2);
    msg->setPeer2Addresses(0, info->peer1Address1);
    msg->setPeer2Addresses(1, info->peer1Address2);
    msg->setPortPeer2(info->peer1Port);
    EV << "Info for peer2: peer1-1=" << msg->getPeer1Addresses(0) << " peer2-1=" << msg->getPeer2Addresses(0) << "\n";
    if (info->multi)
        EV << " peer1-2=" << msg->getPeer1Addresses(1) << " peer2-2=" << msg->getPeer2Addresses(1) << endl;
    cmsg = new cPacket(msg->getName());
    smsg = new SCTPSimpleMessage("nat_data");
    smsg->setEncaps(true);
    smsg->encapsulate(msg);
    smsg->setCreationTime(simulation.getSimTime());
    smsg->setByteLength(16);
    smsg->setDataLen(16);
    cmsg->encapsulate(PK(smsg));
    cmd = new SCTPSendCommand();
    cmd->setAssocId(info->peer2Assoc);
    cmd->setGate(info->peer2Gate);
    cmd->setSendUnordered(COMPLETE_MESG_UNORDERED);
    cmd->setSid(0);
    cmd->setLast(true);
    cmsg->setKind(SCTP_C_SEND);
    cmsg->setControlInfo(cmd);
    EV << "info sent to peer2\n";
    send(cmsg, "sctpOut");
    abortMsg = new cPacket("abortPeer2", SCTP_C_SHUTDOWN);
    abortMsg->setControlInfo(cmd->dup());
    send(abortMsg, "sctpOut");
    EV << "abortMsg sent to peer2\n";
}

void SCTPNatServer::generateAndSend()
{
    cPacket *cmsg = new cPacket("CMSG");
    SCTPSimpleMessage *msg = new SCTPSimpleMessage("Server");
    int numBytes = (int)par("requestLength");
    msg->setDataArraySize(numBytes);
    for (int i = 0; i < numBytes; i++)
        msg->setData(i, 's');

    msg->setDataLen(numBytes);
    msg->setEncaps(false);
    msg->setBitLength(numBytes * 8);
    cmsg->encapsulate(msg);
    SCTPSendCommand *cmd = new SCTPSendCommand("Send1");
    cmd->setAssocId(assocId);
    cmd->setSendUnordered(ordered ? COMPLETE_MESG_ORDERED : COMPLETE_MESG_UNORDERED);
    lastStream = (lastStream + 1) % outboundStreams;
    cmd->setSid(lastStream);
    cmd->setLast(true);
    cmsg->setKind(SCTP_C_SEND);
    cmsg->setControlInfo(cmd);
    packetsSent++;
    bytesSent += msg->getBitLength() / 8;
    send(cmsg, "sctpOut");
}

void SCTPNatServer::handleMessage(cMessage *msg)
{
    int32 id;

    if (msg->isSelfMessage()) {
        handleTimer(msg);
    }
    else {
        EV << "SCTPNatServer::handleMessage kind=" << SCTPAssociation::indicationName(msg->getKind()) << " (" << msg->getKind() << ")\n";
        switch (msg->getKind()) {
            case SCTP_I_PEER_CLOSED:
            case SCTP_I_ABORT: {
                SCTPCommand *ind = check_and_cast<SCTPCommand *>(msg->getControlInfo()->dup());
                cPacket *cmsg = new cPacket("Notification");
                SCTPSendCommand *cmd = new SCTPSendCommand();
                id = ind->getAssocId();
                cmd->setAssocId(id);
                cmd->setSid(ind->getSid());
                cmd->setNumMsgs(ind->getNumMsgs());
                cmsg->setControlInfo(cmd);
                delete ind;
                delete msg;
                cmsg->setKind(SCTP_C_ABORT);
                send(cmsg, "sctpOut");
                break;
            }

            case SCTP_I_ESTABLISHED: {
                SCTPConnectInfo *connectInfo = check_and_cast<SCTPConnectInfo *>(msg->removeControlInfo());
                numSessions++;
                assocId = connectInfo->getAssocId();
                id = assocId;
                inboundStreams = connectInfo->getInboundStreams();
                outboundStreams = connectInfo->getOutboundStreams();

                delete connectInfo;
                delete msg;

                break;
            }

            case SCTP_I_DATA_NOTIFICATION: {
                notifications++;
                SCTPCommand *ind = check_and_cast<SCTPCommand *>(msg->removeControlInfo());
                cPacket *cmsg = new cPacket("Notification");
                SCTPSendCommand *cmd = new SCTPSendCommand();
                id = ind->getAssocId();
                cmd->setAssocId(id);
                cmd->setSid(ind->getSid());
                cmd->setNumMsgs(ind->getNumMsgs());
                cmsg->setKind(SCTP_C_RECEIVE);
                cmsg->setControlInfo(cmd);
                delete ind;
                delete msg;
                send(cmsg, "sctpOut");
                break;
            }

            case SCTP_I_DATA: {
                EV << "\nData arrived at server: assoc=" << assocId << "\n";
                printNatVector();
                SCTPCommand *ind = check_and_cast<SCTPCommand *>(msg->removeControlInfo());
                id = ind->getAssocId();
                SCTPSimpleMessage *smsg = check_and_cast<SCTPSimpleMessage *>(msg);
                NatMessage *nat = check_and_cast<NatMessage *>(smsg->decapsulate());
                bool found = false;
                if (natVector.size() > 0) {
                    for (NatVector::iterator it = natVector.begin(); it != natVector.end(); it++) {
                        if ((*it)->peer1 == nat->getPeer1() || (*it)->peer1Assoc == assocId) {
                            EV << "found entry: info: Peer1 = " << nat->getPeer1() << "  peer1Address1=" << nat->getPeer1Addresses(0) << " peer2=" << nat->getPeer2() << " peer2Address1=" << nat->getPeer2Addresses(0) << "\n";
                            if (nat->getMulti() && nat->getPeer1AddressesArraySize() > 1 && nat->getPeer2AddressesArraySize() > 1) {
                                EV << " peer1Address2=" << nat->getPeer1Addresses(1) << " peer2Address2=" << nat->getPeer2Addresses(1) << endl;
                            }
                            if ((*it)->peer1 == 0 && !(*it)->peer1Address2.isUnspecified()) {
                                (*it)->peer1 = nat->getPeer1();
                                (*it)->peer1Address1 = ind->getRemoteAddr();
                                (*it)->peer1Port = nat->getPortPeer1();
                                if ((*it)->peer2 == 0) {
                                    (*it)->peer2 = nat->getPeer2();
                                }
                            }
                            if (nat->getMulti() && (*it)->peer1Address2.isUnspecified()) {
                                (*it)->peer1Address2 = ind->getRemoteAddr();
                            }
                            if (!(*it)->peer2Address1.isUnspecified() && !(*it)->peer1Address1.isUnspecified()) {
                                if (!(*it)->multi || ((*it)->multi && !(*it)->peer2Address2.isUnspecified() && !(*it)->peer1Address2.isUnspecified())) {
                                    EV << "entry now: Peer1=" << (*it)->peer1 << " Peer2=" << (*it)->peer2 << " peer1Address1=" << (*it)->peer1Address1 << " peer1Address2=" << (*it)->peer1Address2 << " peer2Address1=" << (*it)->peer2Address1 << " peer2Address2=" << (*it)->peer2Address2 << " peer2Port=" << (*it)->peer2Port << "\n";
                                    sendInfo((*it));
                                }
                            }
                            found = true;
                            break;
                        }
                        if ((*it)->peer2 == nat->getPeer1() || (*it)->peer2Assoc == assocId) {
                            EV << "opposite way:  info: Peer1 = " << nat->getPeer1() << "  peer1Address1=" << nat->getPeer1Addresses(0) << " peer2=" << nat->getPeer2() << " peer2Address1=" << nat->getPeer2Addresses(0) << "\n";
                            if (nat->getMulti() && nat->getPeer1AddressesArraySize() > 1 && nat->getPeer2AddressesArraySize() > 1) {
                                EV << " peer1Address2=" << nat->getPeer1Addresses(1) << " peer2Address2=" << nat->getPeer2Addresses(1) << endl;
                            }
                            if ((*it)->peer2 == 0) {
                                (*it)->peer2 = nat->getPeer1();
                            }
                            if ((*it)->peer1 == 0) {
                                (*it)->peer1 = nat->getPeer2();
                            }
                            if ((*it)->peer2Address1.isUnspecified()) {
                                (*it)->peer2Address1 = ind->getRemoteAddr();
                                (*it)->peer2Assoc = assocId;
                                (*it)->peer2Port = nat->getPortPeer1();
                                (*it)->peer2Gate = ind->getGate();
                                EV << "set peer2Address1=" << ind->getRemoteAddr() << " peer2Assoc=" << assocId << " peer2Port=" << nat->getPortPeer1() << "\n";
                            }
                            else if ((*it)->multi && !(*it)->peer2Address2.isUnspecified())
                                (*it)->peer2Address2 = ind->getRemoteAddr();

                            if (!(*it)->multi || ((*it)->multi && !(*it)->peer2Address2.isUnspecified() && !(*it)->peer1Address2.isUnspecified()
                                                  && !(*it)->peer2Address1.isUnspecified() && !(*it)->peer1Address1.isUnspecified()))
                            {
                                EV << "entry now: Peer1=" << (*it)->peer1 << " Peer2=" << (*it)->peer2 << " peer1Address1=" << (*it)->peer1Address1 << " peer1Address2=" << (*it)->peer1Address2 << " peer2Address1=" << (*it)->peer2Address1 << " peer2Address2=" << (*it)->peer2Address2 << " peer1Port=" << (*it)->peer1Port << "peer2Port=" << (*it)->peer2Port << "\n";
                                sendInfo((*it));
                            }
                            found = true;
                            break;
                        }
                    }
                }
                if (natVector.size() == 0 || !found) {
                    EV << "make new Info for ";
                    NatInfo *info = new NatInfo();
                    info->peer1 = nat->getPeer1();
                    EV << info->peer1 << " and assoc " << assocId << "\n";
                    ;
                    info->multi = nat->getMulti();
                    info->peer1Address1 = ind->getRemoteAddr();
                    if (info->multi) {
                        info->peer1Address2 = L3Address();
                        info->peer2Address2 = L3Address();
                    }
                    info->peer1Port = nat->getPortPeer1();
                    info->peer1Assoc = assocId;
                    info->peer1Gate = ind->getGate();
                    info->peer2 = nat->getPeer2();
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

                delete ind;
                delete nat;
                break;
            }

            case SCTP_I_SHUTDOWN_RECEIVED: {
                SCTPCommand *command = check_and_cast<SCTPCommand *>(msg->removeControlInfo());
                id = command->getAssocId();
                EV << "server: SCTP_I_SHUTDOWN_RECEIVED for assoc " << id << "\n";
                cPacket *cmsg = new cPacket("Request");
                SCTPInfo *qinfo = new SCTPInfo("Info");
                cmsg->setKind(SCTP_C_NO_OUTSTANDING);
                qinfo->setAssocId(id);
                cmsg->setControlInfo(qinfo);
                send(cmsg, "sctpOut");

                delete command;
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
                SCTPCommand *ind = check_and_cast<SCTPCommand *>(msg->removeControlInfo());
                bool found = false;
                printNatVector();
                EV << " address added: LOCAL=" << ind->getLocalAddr() << ", remote=" << ind->getRemoteAddr() << " assoc=" << assocId << "\n";
                if (natVector.size() > 0) {
                    for (NatVector::iterator it = natVector.begin(); it != natVector.end(); it++) {
                        if ((*it)->peer1Assoc == assocId) {
                            EV << "found entry for assoc1 = " << assocId << "  Peer1 = " << (*it)->peer1 << "  peer1Address1=" << (*it)->peer1Address1 << " peer1Address2=" << (*it)->peer1Address2 << " peer2=" << (*it)->peer2 << " peer2Address1=" << (*it)->peer2Address1 << " peer2Address2=" << (*it)->peer2Address2 << "\n";
                            if ((*it)->multi && (*it)->peer1Address2.isUnspecified()) {
                                (*it)->peer1Address2 = ind->getRemoteAddr();
                                EV << "added peer1Address2=" << ind->getRemoteAddr() << "\n";
                            }
                            if (!(*it)->peer2Address1.isUnspecified()) {
                                if (!(*it)->multi || ((*it)->multi && !(*it)->peer2Address2.isUnspecified() && !(*it)->peer1Address2.isUnspecified())) {
                                    EV << "entry now: Peer1=" << (*it)->peer1 << " Peer2=" << (*it)->peer2 << " peer1Address1=" << (*it)->peer1Address1 << " peer1Address2=" << (*it)->peer1Address2 << " peer2Address1=" << (*it)->peer2Address1 << " peer2Address2=" << (*it)->peer2Address2 << " peer2Port=" << (*it)->peer2Port << "\n";
                                    sendInfo((*it));
                                }
                            }
                            found = true;
                            break;
                        }
                        else if ((*it)->peer2Assoc == assocId) {
                            EV << "opposite way: found entry for assoc2 = " << assocId << "  peer1Address1=" << (*it)->peer1Address1 << " peer1Address2=" << (*it)->peer1Address2 << " peer2Address1=" << (*it)->peer2Address1 << " peer2Address2=" << (*it)->peer2Address2 << "\n";
                            if ((*it)->multi)
                                (*it)->peer2Address2 = ind->getRemoteAddr();

                            if (!(*it)->multi || ((*it)->multi && !(*it)->peer2Address2.isUnspecified() && !(*it)->peer1Address2.isUnspecified())) {
                                EV << "entry now: Peer1=" << (*it)->peer1 << " Peer2=" << (*it)->peer2 << " peer1Address1=" << (*it)->peer1Address1 << " peer1Address2=" << (*it)->peer1Address2 << " peer2Address1=" << (*it)->peer2Address1 << " peer2Address2=" << (*it)->peer2Address2 << " peer1Port=" << (*it)->peer1Port << "peer2Port=" << (*it)->peer2Port << "\n";
                                sendInfo((*it));
                            }
                            found = true;
                            break;
                        }
                        else if ((*it)->peer2Assoc == 0 && ((*it)->multi)) {
                            (*it)->peer2Address2 = ind->getRemoteAddr();
                            (*it)->peer2Assoc = assocId;
                            (*it)->peer2Port = ind->getRemotePort();
                            (*it)->peer2Gate = ind->getGate();
                            EV << "entry now: Peer1=" << (*it)->peer1 << " Peer2=" << (*it)->peer2 << " peer1Address1=" << (*it)->peer1Address1 << " peer1Address2=" << (*it)->peer1Address2 << " peer2Address1=" << (*it)->peer2Address1 << " peer2Address2=" << (*it)->peer2Address2 << " peer1Port=" << (*it)->peer1Port << "peer2Port=" << (*it)->peer2Port << "\n";

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
                    ;
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
                delete ind;
                delete msg;
                printNatVector();
                break;
            }

            default:
                EV << "Message type " << SCTPAssociation::indicationName(msg->getKind()) << " not implemented\n";
        }
    }
}

void SCTPNatServer::handleTimer(cMessage *msg)
{
    cPacket *cmsg;
    int32 id;

    SCTPConnectInfo *connectInfo = dynamic_cast<SCTPConnectInfo *>(msg->getControlInfo());
    switch (msg->getKind()) {
        case SCTP_C_SEND:
            if (numRequestsToSend > 0) {
                generateAndSend();
                numRequestsToSend--;
            }
            break;

        case SCTP_I_ABORT: {
            cmsg = new cPacket("CLOSE", SCTP_C_CLOSE);
            SCTPCommand *cmd = new SCTPCommand();
            id = atoi(msg->getName());
            cmd->setAssocId(id);
            cmsg->setControlInfo(cmd);
            send(cmsg, "sctpOut");
            break;
        }

        case SCTP_C_RECEIVE:
            send(msg, "sctpOut");
            break;

        default:
            break;
    }
    delete connectInfo;
}

void SCTPNatServer::printNatVector(void)
{
    for (NatVector::iterator it = natVector.begin(); it != natVector.end(); it++) {
        EV << "Peer1: " << (*it)->peer1 << " Assoc: " << (*it)->peer1Assoc << " Address1: " << (*it)->peer1Address1 << " Address2: " << (*it)->peer1Address2 << "Port: " << (*it)->peer1Port << endl;
        EV << "Peer2: " << (*it)->peer2 << " Assoc: " << (*it)->peer2Assoc << " Address1: " << (*it)->peer2Address1 << " Address2: " << (*it)->peer2Address2 << "Port: " << (*it)->peer2Port << endl;
    }
}

void SCTPNatServer::finish()
{
    EV << getFullPath() << ": opened " << numSessions << " sessions\n";
    EV << getFullPath() << ": sent " << bytesSent << " bytes in " << packetsSent << " packets\n";

    EV << getFullPath() << "Over all " << packetsRcvd << " packets received\n ";
    EV << getFullPath() << "Over all " << notifications << " notifications received\n ";
}

} // namespace inet

