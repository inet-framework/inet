//
// Copyright (C) 2005-2010 Irene Ruengeler
// Copyright (C) 2009-2010 Thomas Dreibholz
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


#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "SCTP.h"
#include "SCTPAssociation.h"
#include "SCTPCommand_m.h"
#include "IPv6ControlInfo.h"
#include "SCTPQueue.h"
#include "SCTPAlgorithm.h"
#include "RoutingTable.h"
#include "RoutingTableAccess.h"
#include "InterfaceTable.h"
#include "InterfaceTableAccess.h"
#include "IPv4InterfaceData.h"
#include "IPv6InterfaceData.h"
#include "IPv6Address.h"
#include "UDPControlInfo_m.h"



void SCTPAssociation::printSctpPathMap() const
{
    sctpEV3 << "SCTP PathMap:" << endl;
    for (SCTPPathMap::const_iterator iterator = sctpPathMap.begin();
            iterator != sctpPathMap.end(); ++iterator) {
        const SCTPPathVariables* path = iterator->second;
        sctpEV3 << " - " << path->remoteAddress << ":  osb=" << path->outstandingBytes
                << " cwnd=" << path->cwnd << endl;
    }
}

const char* SCTPAssociation::stateName(const int32 state)
{
#define CASE(x) case x: s=#x+7; break
    const char* s = "unknown";
    switch (state) {
        CASE(SCTP_S_CLOSED);
        CASE(SCTP_S_COOKIE_WAIT);
        CASE(SCTP_S_COOKIE_ECHOED);
        CASE(SCTP_S_ESTABLISHED);
        CASE(SCTP_S_SHUTDOWN_PENDING);
        CASE(SCTP_S_SHUTDOWN_SENT);
        CASE(SCTP_S_SHUTDOWN_RECEIVED);
        CASE(SCTP_S_SHUTDOWN_ACK_SENT);
    }
    return s;
#undef CASE
}

const char* SCTPAssociation::eventName(const int32 event)
{
#define CASE(x) case x: s=#x+7; break
    const char* s = "unknown";
    switch (event) {
        CASE(SCTP_E_OPEN_PASSIVE);
        CASE(SCTP_E_ASSOCIATE);
        CASE(SCTP_E_SHUTDOWN);
        CASE(SCTP_E_CLOSE);
        CASE(SCTP_E_ABORT);
        CASE(SCTP_E_SEND);
        CASE(SCTP_E_RCV_INIT);
        CASE(SCTP_E_RCV_ABORT);
        CASE(SCTP_E_RCV_VALID_COOKIE_ECHO);
        CASE(SCTP_E_RCV_INIT_ACK);
        CASE(SCTP_E_RCV_COOKIE_ACK);
        CASE(SCTP_E_RCV_SHUTDOWN);
        CASE(SCTP_E_RCV_SHUTDOWN_ACK);
        CASE(SCTP_E_RCV_SHUTDOWN_COMPLETE);
        CASE(SCTP_E_TIMEOUT_INIT_TIMER);
        CASE(SCTP_E_TIMEOUT_SHUTDOWN_TIMER);
        CASE(SCTP_E_TIMEOUT_RTX_TIMER);
        CASE(SCTP_E_TIMEOUT_HEARTBEAT_TIMER);
        CASE(SCTP_E_RECEIVE);
        CASE(SCTP_E_DUP_RECEIVED);
        CASE(SCTP_E_PRIMARY);
        CASE(SCTP_E_QUEUE_MSGS_LIMIT);
        CASE(SCTP_E_QUEUE_BYTES_LIMIT);
        CASE(SCTP_E_NO_MORE_OUTSTANDING);
        CASE(SCTP_E_IGNORE);
        CASE(SCTP_E_DELIVERED);
        CASE(SCTP_E_SEND_SHUTDOWN_ACK);
        CASE(SCTP_E_STOP_SENDING);
    }
    return s;
#undef CASE
}

const char* SCTPAssociation::indicationName(const int32 code)
{
#define CASE(x) case x: s=#x+7; break
    const char* s = "unknown";
    switch (code) {
        CASE(SCTP_I_DATA);
        CASE(SCTP_I_DATA_NOTIFICATION);
        CASE(SCTP_I_ESTABLISHED);
        CASE(SCTP_I_PEER_CLOSED);
        CASE(SCTP_I_CLOSED);
        CASE(SCTP_I_CONNECTION_REFUSED);
        CASE(SCTP_I_CONNECTION_RESET);
        CASE(SCTP_I_TIMED_OUT);
        CASE(SCTP_I_STATUS);
        CASE(SCTP_I_ABORT);
        CASE(SCTP_I_SHUTDOWN_RECEIVED);
        CASE(SCTP_I_SEND_MSG);
        CASE(SCTP_I_SENDQUEUE_FULL);
        CASE(SCTP_I_SENDQUEUE_ABATED);
    }
    return s;
#undef CASE
}


uint32 SCTPAssociation::chunkToInt(const char* type)
{
    if (strcmp(type, "DATA")==0) return 0;
    if (strcmp(type, "INIT")==0) return 1;
    if (strcmp(type, "INIT_ACK")==0) return 2;
    if (strcmp(type, "SACK")==0) return 3;
    if (strcmp(type, "HEARTBEAT")==0) return 4;
    if (strcmp(type, "HEARTBEAT_ACK")==0) return 5;
    if (strcmp(type, "ABORT")==0) return 6;
    if (strcmp(type, "SHUTDOWN")==0) return 7;
    if (strcmp(type, "SHUTDOWN_ACK")==0) return 8;
    if (strcmp(type, "ERRORTYPE")==0) return 9;
    if (strcmp(type, "COOKIE_ECHO")==0) return 10;
    if (strcmp(type, "COOKIE_ACK")==0) return 11;
    if (strcmp(type, "SHUTDOWN_COMPLETE")==0) return 14;
    sctpEV3 << "ChunkConversion not successful\n";
    return 0;
}

void SCTPAssociation::printConnBrief()
{
    sctpEV3 << "Connection " << this << " ";
    sctpEV3 << localAddr << ":" << localPort << " to " << remoteAddr << ":" << remotePort;
    sctpEV3 << "  on app[" << appGateIndex << "],assocId=" << assocId;
    sctpEV3 << "  in " << stateName(fsm->getState()) << "\n";
}

void SCTPAssociation::printSegmentBrief(SCTPMessage *sctpmsg)
{
    sctpEV3 << "." << sctpmsg->getSrcPort() << " > ";
    sctpEV3 << "." << sctpmsg->getDestPort() << ": ";
    sctpEV3 << "initTag " << sctpmsg->getTag() << "\n";
}

SCTPAssociation* SCTPAssociation::cloneAssociation()
{
    SCTPAssociation* assoc = new SCTPAssociation(sctpMain, appGateIndex, assocId);
    const char* queueClass = transmissionQ->getClassName();
    assoc->transmissionQ = check_and_cast<SCTPQueue *>(createOne(queueClass));
    assoc->retransmissionQ = check_and_cast<SCTPQueue *>(createOne(queueClass));

    const char* sctpAlgorithmClass = sctpAlgorithm->getClassName();
    assoc->sctpAlgorithm = check_and_cast<SCTPAlgorithm *>(createOne(sctpAlgorithmClass));
    assoc->sctpAlgorithm->setAssociation(assoc);
    assoc->sctpAlgorithm->initialize();
    assoc->state = assoc->sctpAlgorithm->createStateVariables();

    assoc->state->active = false;
    assoc->state->fork = true;
    assoc->localAddr = localAddr;
    assoc->localPort = localPort;
    assoc->localAddressList = localAddressList;

    FSM_Goto((*assoc->fsm), SCTP_S_CLOSED);
    sctpMain->printInfoConnMap();
    return assoc;
}

void SCTPAssociation::recordInPathVectors(SCTPMessage* pMsg,
        const IPvXAddress& rDest)
{
    uint32 n_chunks = pMsg->getChunksArraySize();
    if (n_chunks == 0)
        return;

    SCTPPathVariables* p_path = getPath(rDest);

    for (uint32 i = 0; i < n_chunks; i++) {
        const SCTPChunk* p_chunk = check_and_cast<const SCTPChunk *>(pMsg->getChunks(i));
        if (p_chunk->getChunkType() == DATA) {
            const SCTPDataChunk* p_data_chunk = check_and_cast<const SCTPDataChunk *>(p_chunk);
            p_path->pathTSN->record(p_data_chunk->getTsn());
        } else if (p_chunk->getChunkType() == HEARTBEAT) {
            p_path->numberOfHeartbeatsSent++;
            p_path->pathHb->record(p_path->numberOfHeartbeatsSent);
        } else if (p_chunk->getChunkType() == HEARTBEAT_ACK) {
            p_path->numberOfHeartbeatAcksSent++;
            p_path->pathHbAck->record(p_path->numberOfHeartbeatAcksSent);
        }
    }
}

void SCTPAssociation::sendToIP(SCTPMessage*       sctpmsg,
        const IPvXAddress& dest,
        const bool           qs)
{
    // Final touches on the segment before sending
    sctpmsg->setSrcPort(localPort);
    sctpmsg->setDestPort(remotePort);
    sctpmsg->setChecksumOk(true);
    const SCTPChunk* chunk = (const SCTPChunk*)(sctpmsg->peekFirstChunk());
    if (chunk->getChunkType() == ABORT) {
        const SCTPAbortChunk* abortChunk = check_and_cast<const SCTPAbortChunk *>(chunk);
        if (abortChunk->getT_Bit() == 1) {
            sctpmsg->setTag(peerVTag);
        }
        else {
            sctpmsg->setTag(localVTag);
        }
    }
    else if (sctpmsg->getTag() == 0) {
        sctpmsg->setTag(localVTag);
    }

    if ((bool)sctpMain->par("udpEncapsEnabled")) {
        sctpmsg->setKind(UDP_C_DATA);
        UDPControlInfo* controlInfo = new UDPControlInfo();
        controlInfo->setSrcPort(9899);
        controlInfo->setDestAddr(remoteAddr.get4());
        controlInfo->setDestPort(9899);
        sctpmsg->setControlInfo(controlInfo);
    }
    else {
        if (dest.isIPv6()) {
            IPv6ControlInfo* controlInfo = new IPv6ControlInfo();
            controlInfo->setProtocol(IP_PROT_SCTP);
            controlInfo->setSrcAddr(IPv6Address());
            controlInfo->setDestAddr(dest.get6());
            sctpmsg->setControlInfo(controlInfo);
            sctpMain->send(sctpmsg, "to_ipv6");
        }
        else {
            IPControlInfo* controlInfo = new IPControlInfo();
            controlInfo->setProtocol(IP_PROT_SCTP);
            controlInfo->setSrcAddr(IPAddress("0.0.0.0"));
            controlInfo->setDestAddr(dest.get4());
            sctpmsg->setControlInfo(controlInfo);
            sctpMain->send(sctpmsg, "to_ip");
        }
        recordInPathVectors(sctpmsg, dest);
    }
    sctpEV3 << "Sent to " << dest << endl;
}


void SCTPAssociation::signalConnectionTimeout()
{
    sendIndicationToApp(SCTP_I_TIMED_OUT);
}

void SCTPAssociation::sendIndicationToApp(const int32 code, const int32 value)
{
    sctpEV3 << "sendIndicationToApp: " << indicationName(code) << endl;

    cPacket* msg = new cPacket(indicationName(code));
    msg->setKind(code);

    SCTPCommand* indication = new SCTPCommand(indicationName(code));
    indication->setAssocId(assocId);
    indication->setLocalAddr(localAddr);
    indication->setRemoteAddr(remoteAddr);
    if (code == SCTP_I_SENDQUEUE_ABATED) {
        indication->setNumMsgs(value);
    }
    msg->setControlInfo(indication);
    sctpMain->send(msg, "to_appl", appGateIndex);
}

void SCTPAssociation::sendEstabIndicationToApp()
{
    sctpEV3 << "sendEstabIndicationToApp: localPort="
            << localPort << " remotePort=" << remotePort << endl;

    cPacket* msg = new cPacket(indicationName(SCTP_I_ESTABLISHED));
    msg->setKind(SCTP_I_ESTABLISHED);

    SCTPConnectInfo* establishIndication = new SCTPConnectInfo("CI");
    establishIndication->setAssocId(assocId);
    establishIndication->setLocalAddr(localAddr);
    establishIndication->setRemoteAddr(remoteAddr);
    establishIndication->setLocalPort(localPort);
    establishIndication->setRemotePort(remotePort);
    establishIndication->setRemoteAddresses(remoteAddressList);
    establishIndication->setInboundStreams(inboundStreams);
    establishIndication->setOutboundStreams(outboundStreams);
    establishIndication->setNumMsgs(state->sendQueueLimit);
    msg->setControlInfo(establishIndication);
    sctpMain->send(msg, "to_appl", appGateIndex);

}

void SCTPAssociation::sendToApp(cPacket *msg)
{
    sctpMain->send(msg, "to_appl", appGateIndex);
}

void SCTPAssociation::initAssociation(SCTPOpenCommand *openCmd)
{
    sctpEV3 << "SCTPAssociationUtil:initAssociation\n";
    // create send/receive queues
    const char *queueClass = openCmd->getQueueClass();
    transmissionQ = check_and_cast<SCTPQueue *>(createOne(queueClass));

    retransmissionQ = check_and_cast<SCTPQueue *>(createOne(queueClass));
    outboundStreams = openCmd->getOutboundStreams();
    // create algorithm
    const char *sctpAlgorithmClass = openCmd->getSctpAlgorithmClass();
    if (!sctpAlgorithmClass || !sctpAlgorithmClass[0])
        sctpAlgorithmClass = sctpMain->par("sctpAlgorithmClass");
    sctpAlgorithm = check_and_cast<SCTPAlgorithm *>(createOne(sctpAlgorithmClass));
    sctpAlgorithm->setAssociation(this);
    sctpAlgorithm->initialize();
    // create state block
    state = sctpAlgorithm->createStateVariables();
}


void SCTPAssociation::sendInit()
{
    //RoutingTableAccess routingTableAccess;
    InterfaceTableAccess interfaceTableAccess;
    AddressVector adv;
    uint32 length = SCTP_INIT_CHUNK_LENGTH;

    if (remoteAddr.isUnspecified() || remotePort==0)
        opp_error("Error processing command ASSOCIATE: foreign socket unspecified");
    if (localPort==0)
        opp_error("Error processing command ASSOCIATE: local port unspecified");
    state->setPrimaryPath(getPath(remoteAddr));
    // create message consisting of INIT chunk
    SCTPMessage *sctpmsg = new SCTPMessage();
    sctpmsg->setBitLength(SCTP_COMMON_HEADER*8);
    SCTPInitChunk *initChunk = new SCTPInitChunk("INIT");
    initChunk->setChunkType(INIT);
    initChunk->setInitTag((uint32)(fmod(intrand(INT32_MAX), 1.0+(double)(unsigned)0xffffffffUL)) & 0xffffffffUL);

    peerVTag = initChunk->getInitTag();
    sctpEV3 << "INIT from " << localAddr << ":InitTag=" << peerVTag << "\n";
    initChunk->setA_rwnd(sctpMain->par("arwnd"));
    state->localRwnd = (long)sctpMain->par("arwnd");
    initChunk->setNoOutStreams(outboundStreams);
    initChunk->setNoInStreams(inboundStreams);
    initChunk->setInitTSN(1000);
    state->nextTSN = initChunk->getInitTSN();
    state->lastTSN = initChunk->getInitTSN() + state->numRequests - 1;
    initTsn = initChunk->getInitTSN();
    IInterfaceTable *ift = interfaceTableAccess.get();
    sctpEV3 << "add local address\n";
    if (localAddressList.front() == IPvXAddress("0.0.0.0"))
    {
        for (int32 i=0; i<ift->getNumInterfaces(); ++i)
        {
            if (ift->getInterface(i)->ipv4Data()!=NULL)
            {
                adv.push_back(ift->getInterface(i)->ipv4Data()->getIPAddress());
            }
            else if (ift->getInterface(i)->ipv6Data()!=NULL)
            {
                for (int32 j=0; j<ift->getInterface(i)->ipv6Data()->getNumAddresses(); j++)
                {
                    sctpEV3 << "add address " << ift->getInterface(i)->ipv6Data()->getAddress(j) << "\n";
                    adv.push_back(ift->getInterface(i)->ipv6Data()->getAddress(j));
                }
            }
        }
    }
    else
    {
        adv = localAddressList;
        sctpEV3 << "gebundene Adresse " << localAddr << " wird hinzugefuegt\n";
    }
    uint32 addrNum = 0;
    bool friendly = false;
    if (remoteAddr.isIPv6())
    {
        for (AddressVector::iterator i=adv.begin(); i!=adv.end(); ++i)
        {
            if (!friendly)
            {
                initChunk->setAddressesArraySize(addrNum+1);
                initChunk->setAddresses(addrNum++, (*i));
                length += 20;
            }
            sctpMain->addLocalAddress(this, (*i));
            state->localAddresses.push_back((*i));
            if (localAddr.isUnspecified())
                localAddr = (*i);
        }
    }
    else
    {
        uint32 rlevel = getLevel(remoteAddr);
        sctpEV3 << "level of remote address=" << rlevel << "\n";
        for (AddressVector::iterator i=adv.begin(); i!=adv.end(); ++i)
        {
            sctpEV3 << "level of address " << (*i) << " = " << getLevel((*i)) << "\n";
            if (getLevel((*i))>=rlevel)
            {
                initChunk->setAddressesArraySize(addrNum+1);
                initChunk->setAddresses(addrNum++, (*i));
                length += 8;
                sctpMain->addLocalAddress(this, (*i));
                state->localAddresses.push_back((*i));
                if (localAddr.get4().getInt()==0)
                    localAddr = (*i);
            }
            else if (rlevel==4 && getLevel((*i))==3 && friendly)
            {
                sctpMain->addLocalAddress(this, (*i));
                state->localAddresses.push_back((*i));
                if (localAddr.get4().getInt()==0)
                    localAddr = (*i);
            }
        }
    }
    sctpMain->printInfoConnMap();
    initChunk->setBitLength(length*8);
    sctpmsg->addChunk(initChunk);
    // set path variables
    if (remoteAddressList.size()>0)
    {
        for (AddressVector::iterator it=remoteAddressList.begin(); it!=remoteAddressList.end(); it++)
        {
            sctpEV3 << __LINE__ << " get new path for " << (*it) << "\n";
            SCTPPathVariables* path = new SCTPPathVariables((*it), this);
            sctpPathMap[(*it)] = path;
            qCounter.roomTransQ[(*it)] = 0;
            qCounter.bookedTransQ[(*it)] = 0;
            qCounter.roomRetransQ[(*it)] = 0;
        }
    }
    else
    {
        sctpEV3 << __LINE__ << " get new path for " << remoteAddr << "\n";
        SCTPPathVariables* path = new SCTPPathVariables(remoteAddr, this);
        sctpPathMap[remoteAddr] = path;
        qCounter.roomTransQ[remoteAddr] = 0;
        qCounter.bookedTransQ[remoteAddr] = 0;
        qCounter.roomRetransQ[remoteAddr] = 0;
    }
    // send it
    state->initChunk = check_and_cast<SCTPInitChunk *>(initChunk->dup());
    state->initChunk->setName("StateInitChunk");
    printSctpPathMap();
    sctpEV3 << getFullPath() << " sendInit: localVTag=" << localVTag << " peerVTag=" << peerVTag << "\n";
    sendToIP(sctpmsg);
    sctpMain->assocList.push_back(this);
}

void SCTPAssociation::retransmitInit()
{
    SCTPMessage *sctpmsg = new SCTPMessage();
    sctpmsg->setBitLength(SCTP_COMMON_HEADER*8);
    SCTPInitChunk *sctpinit; // = new SCTPInitChunk("INIT");

    sctpEV3 << "Retransmit InitChunk=" << &sctpinit << "\n";

    sctpinit = check_and_cast<SCTPInitChunk *>(state->initChunk->dup());
    sctpinit->setChunkType(INIT);
    sctpmsg->addChunk(sctpinit);

    sendToIP(sctpmsg);
}


void SCTPAssociation::sendInitAck(SCTPInitChunk* initChunk)
{
    uint32 length = SCTP_INIT_CHUNK_LENGTH;

    state->setPrimaryPath(getPath(remoteAddr));
    // create segment
    SCTPMessage *sctpinitack = new SCTPMessage();
    sctpinitack->setBitLength(SCTP_COMMON_HEADER*8);

    sctpinitack->setSrcPort(localPort);
    sctpinitack->setDestPort(remotePort);
    sctpEV3 << "sendInitAck at " << localAddr << ". Provided InitTag=" << initChunk->getInitTag() << "\n";
    SCTPInitAckChunk *initAckChunk = new SCTPInitAckChunk("INIT_ACK");
    initAckChunk->setChunkType(INIT_ACK);
    SCTPCookie *cookie = new SCTPCookie("CookieUtil");
    cookie->setCreationTime(simTime());
    cookie->setLocalTieTagArraySize(32);
    cookie->setPeerTieTagArraySize(32);
    if (fsm->getState()==SCTP_S_CLOSED)
    {
        while (peerVTag==0)
        {
            peerVTag = (uint32)intrand(INT32_MAX);
        }
        initAckChunk->setInitTag(peerVTag);
        initAckChunk->setInitTSN(2000);
        state->nextTSN = initAckChunk->getInitTSN();
        state->lastTSN = initAckChunk->getInitTSN() + state->numRequests - 1;
        cookie->setLocalTag(localVTag);
        cookie->setPeerTag(peerVTag);
        for (int32 i=0; i<32; i++)
        {
            cookie->setLocalTieTag(i, 0);
            cookie->setPeerTieTag(i, 0);
        }
        sctpinitack->setTag(localVTag);
        sctpEV3 << "state=closed: localVTag=" << localVTag << " peerVTag=" << peerVTag << "\n";
    }
    else if (fsm->getState()==SCTP_S_COOKIE_WAIT || fsm->getState()==SCTP_S_COOKIE_ECHOED)
    {
        initAckChunk->setInitTag(peerVTag);
        sctpEV3 << "different state:set InitTag in InitAck: " << initAckChunk->getInitTag() << "\n";
        initAckChunk->setInitTSN(state->nextTSN);
        initPeerTsn = initChunk->getInitTSN();
        state->cTsnAck = initPeerTsn - 1;
        cookie->setLocalTag(initChunk->getInitTag());
        cookie->setPeerTag(peerVTag);
        for (int32 i=0; i<32; i++)
        {
            cookie->setPeerTieTag(i, (uint8)(intrand(256)));
            state->peerTieTag[i] = cookie->getPeerTieTag(i);
            if (fsm->getState()==SCTP_S_COOKIE_ECHOED)
            {
                cookie->setLocalTieTag(i, (uint8)(intrand(256)));
                state->localTieTag[i] = cookie->getLocalTieTag(i);
            }
            else
                cookie->setLocalTieTag(i, 0);
        }
        sctpinitack->setTag(initChunk->getInitTag());
        sctpEV3 << "VTag in InitAck: " << sctpinitack->getTag() << "\n";
    }
    else
    {
        sctpEV3 << "other state\n";
        uint32 tag = 0;
        while (tag==0)
        {
            tag = (uint32)(fmod(intrand(INT32_MAX), 1.0+(double)(unsigned)0xffffffffUL)) & 0xffffffffUL;
        }
        initAckChunk->setInitTag(tag);
        initAckChunk->setInitTSN(state->nextTSN);
        cookie->setLocalTag(localVTag);
        cookie->setPeerTag(peerVTag);
        for (int32 i=0; i<32; i++)
        {
            cookie->setPeerTieTag(i, state->peerTieTag[i]);
            cookie->setLocalTieTag(i, state->localTieTag[i]);
        }
        sctpinitack->setTag(initChunk->getInitTag());
    }
    cookie->setBitLength(SCTP_COOKIE_LENGTH*8);
    initAckChunk->setStateCookie(cookie);
    initAckChunk->setCookieArraySize(0);
    initAckChunk->setA_rwnd(sctpMain->par("arwnd"));
    state->localRwnd = (long)sctpMain->par("arwnd");
    initAckChunk->setNoOutStreams((unsigned int)min(outboundStreams, initChunk->getNoInStreams()));
    initAckChunk->setNoInStreams((unsigned int)min(inboundStreams, initChunk->getNoOutStreams()));
    initTsn = initAckChunk->getInitTSN();
    uint32 addrNum = 0;
    bool friendly = false;
    if (!friendly)
        for (AddressVector::iterator k=state->localAddresses.begin(); k!=state->localAddresses.end(); ++k)
        {
            initAckChunk->setAddressesArraySize(addrNum+1);
            initAckChunk->setAddresses(addrNum++, (*k));
            length += 8;
        }
    uint32 unknownLen = initChunk->getUnrecognizedParametersArraySize();
    if (unknownLen>0)
    {
        sctpEV3 << "Found unrecognized Parameters in INIT chunk with a length of " << unknownLen << " bytes.\n";
        initAckChunk->setUnrecognizedParametersArraySize(unknownLen);
        for (uint32 i=0; i<unknownLen; i++)
            initAckChunk->setUnrecognizedParameters(i, initChunk->getUnrecognizedParameters(i));
        length += unknownLen;
    }
    else
        initAckChunk->setUnrecognizedParametersArraySize(0);

    initAckChunk->setBitLength((length+initAckChunk->getCookieArraySize())*8 + cookie->getBitLength());
    inboundStreams = ((initChunk->getNoOutStreams()<initAckChunk->getNoInStreams())?initChunk->getNoOutStreams():initAckChunk->getNoInStreams());
    outboundStreams = ((initChunk->getNoInStreams()<initAckChunk->getNoOutStreams())?initChunk->getNoInStreams():initAckChunk->getNoOutStreams());
    (this->*ssFunctions.ssInitStreams)(inboundStreams, outboundStreams);
    sctpinitack->addChunk(initAckChunk);
    if (fsm->getState()==SCTP_S_CLOSED)
    {
        sendToIP(sctpinitack, state->initialPrimaryPath);
    }
    else
    {
        sendToIP(sctpinitack);

    }
    sctpMain->assocList.push_back(this);
    printSctpPathMap();
}

void SCTPAssociation::sendCookieEcho(SCTPInitAckChunk* initAckChunk)
{
    SCTPMessage *sctpcookieecho = new SCTPMessage();
    sctpcookieecho->setBitLength(SCTP_COMMON_HEADER*8);

    sctpEV3 << "SCTPAssociationUtil:sendCookieEcho\n";

    sctpcookieecho->setSrcPort(localPort);
    sctpcookieecho->setDestPort(remotePort);
    SCTPCookieEchoChunk* cookieEchoChunk = new SCTPCookieEchoChunk("COOKIE_ECHO");
    cookieEchoChunk->setChunkType(COOKIE_ECHO);
    int32 len = initAckChunk->getCookieArraySize();
    cookieEchoChunk->setCookieArraySize(len);
    if (len>0)
    {
        for (int32 i=0; i<len; i++)
            cookieEchoChunk->setCookie(i, initAckChunk->getCookie(i));
        cookieEchoChunk->setBitLength((SCTP_COOKIE_ACK_LENGTH+len)*8);
    }
    else
    {
        SCTPCookie* cookie = check_and_cast <SCTPCookie*> (initAckChunk->getStateCookie());
        cookieEchoChunk->setStateCookie(cookie);
        cookieEchoChunk->setBitLength(SCTP_COOKIE_ACK_LENGTH*8 + cookie->getBitLength());
    }
    uint32 unknownLen = initAckChunk->getUnrecognizedParametersArraySize();
    if (unknownLen>0)
    {
        sctpEV3 << "Found unrecognized Parameters in INIT-ACK chunk with a length of " << unknownLen << " bytes.\n";
        cookieEchoChunk->setUnrecognizedParametersArraySize(unknownLen);
        for (uint32 i=0; i<unknownLen; i++)
            cookieEchoChunk->setUnrecognizedParameters(i, initAckChunk->getUnrecognizedParameters(i));
    }
    else
        cookieEchoChunk->setUnrecognizedParametersArraySize(0);
    state->cookieChunk = check_and_cast<SCTPCookieEchoChunk*>(cookieEchoChunk->dup());
    if (len==0)
    {
        state->cookieChunk->setStateCookie(initAckChunk->getStateCookie()->dup());
    }
    sctpcookieecho->addChunk(cookieEchoChunk);
    sendToIP(sctpcookieecho);
}


void SCTPAssociation::retransmitCookieEcho()
{
    SCTPMessage*                 sctpmsg = new SCTPMessage();
    sctpmsg->setBitLength(SCTP_COMMON_HEADER*8);
    SCTPCookieEchoChunk* cookieEchoChunk = check_and_cast<SCTPCookieEchoChunk*>(state->cookieChunk->dup());
    if (cookieEchoChunk->getCookieArraySize()==0)
    {
        cookieEchoChunk->setStateCookie(state->cookieChunk->getStateCookie()->dup());
    }
    sctpmsg->addChunk(cookieEchoChunk);

    sctpEV3 << "retransmitCookieEcho localAddr=" << localAddr << "     remoteAddr" << remoteAddr << "\n";

    sendToIP(sctpmsg);
}

void SCTPAssociation::sendHeartbeat(const SCTPPathVariables* path)
{
    SCTPMessage* sctpHeartbeatbeat = new SCTPMessage();
    sctpHeartbeatbeat->setBitLength(SCTP_COMMON_HEADER*8);

    sctpHeartbeatbeat->setSrcPort(localPort);
    sctpHeartbeatbeat->setDestPort(remotePort);
    SCTPHeartbeatChunk* heartbeatChunk = new SCTPHeartbeatChunk("HEARTBEAT");
    heartbeatChunk->setChunkType(HEARTBEAT);
    heartbeatChunk->setRemoteAddr(path->remoteAddress);
    heartbeatChunk->setTimeField(simTime());
    heartbeatChunk->setBitLength((SCTP_HEARTBEAT_CHUNK_LENGTH+12)*8);
    sctpHeartbeatbeat->addChunk(heartbeatChunk);
    sctpEV3 << "sendHeartbeat: sendToIP to " << path->remoteAddress << endl;
    sendToIP(sctpHeartbeatbeat, path->remoteAddress);
}

void SCTPAssociation::sendHeartbeatAck(const SCTPHeartbeatChunk* heartbeatChunk,
        const IPvXAddress&        src,
        const IPvXAddress&        dest)
{
    SCTPMessage*                 sctpHeartbeatAck = new SCTPMessage();
    sctpHeartbeatAck->setBitLength(SCTP_COMMON_HEADER*8);
    sctpHeartbeatAck->setSrcPort(localPort);
    sctpHeartbeatAck->setDestPort(remotePort);
    SCTPHeartbeatAckChunk* heartbeatAckChunk = new SCTPHeartbeatAckChunk("HEARTBEAT_ACK");
    heartbeatAckChunk->setChunkType(HEARTBEAT_ACK);
    heartbeatAckChunk->setRemoteAddr(heartbeatChunk->getRemoteAddr());
    heartbeatAckChunk->setTimeField(heartbeatChunk->getTimeField());
    const int32 len = heartbeatChunk->getInfoArraySize();
    if (len > 0){
        heartbeatAckChunk->setInfoArraySize(len);
        for (int32 i=0; i<len; i++)
            heartbeatAckChunk->setInfo(i, heartbeatChunk->getInfo(i));
    }

    heartbeatAckChunk->setBitLength(heartbeatChunk->getBitLength());
    sctpHeartbeatAck->addChunk(heartbeatAckChunk);

    sctpEV3 << "sendHeartbeatAck: sendToIP from " << src << " to " << dest << endl;
    sendToIP(sctpHeartbeatAck, dest);
}

void SCTPAssociation::sendCookieAck(const IPvXAddress& dest)
{
    SCTPMessage *sctpcookieack = new SCTPMessage();
    sctpcookieack->setBitLength(SCTP_COMMON_HEADER*8);

    sctpEV3 << "SCTPAssociationUtil:sendCookieACK\n";

    sctpcookieack->setSrcPort(localPort);
    sctpcookieack->setDestPort(remotePort);
    SCTPCookieAckChunk* cookieAckChunk = new SCTPCookieAckChunk("COOKIE_ACK");
    cookieAckChunk->setChunkType(COOKIE_ACK);
    cookieAckChunk->setBitLength(SCTP_COOKIE_ACK_LENGTH*8);
    sctpcookieack->addChunk(cookieAckChunk);
    sendToIP(sctpcookieack, dest);
}

void SCTPAssociation::sendShutdownAck(const IPvXAddress& dest)
{
    sendOnAllPaths(getPath(dest));
    if (getOutstandingBytes() == 0) {
        performStateTransition(SCTP_E_NO_MORE_OUTSTANDING);
        SCTPMessage *sctpshutdownack = new SCTPMessage();
        sctpshutdownack->setBitLength(SCTP_COMMON_HEADER*8);

        sctpEV3 << "SCTPAssociationUtil:sendShutdownACK" << endl;

        sctpshutdownack->setSrcPort(localPort);
        sctpshutdownack->setDestPort(remotePort);
        SCTPShutdownAckChunk* shutdownAckChunk = new SCTPShutdownAckChunk("SHUTDOWN_ACK");
        shutdownAckChunk->setChunkType(SHUTDOWN_ACK);
        shutdownAckChunk->setBitLength(SCTP_COOKIE_ACK_LENGTH*8);
        sctpshutdownack->addChunk(shutdownAckChunk);
        state->initRexmitTimeout = SCTP_TIMEOUT_INIT_REXMIT;
        state->initRetransCounter = 0;
        stopTimer(T2_ShutdownTimer);
        startTimer(T2_ShutdownTimer, state->initRexmitTimeout);
        stopTimer(T5_ShutdownGuardTimer);
        startTimer(T5_ShutdownGuardTimer, SHUTDOWN_GUARD_TIMEOUT);
        state->shutdownAckChunk = check_and_cast<SCTPShutdownAckChunk*>(shutdownAckChunk->dup());
        sendToIP(sctpshutdownack, dest);
    }
}

void SCTPAssociation::sendShutdownComplete()
{
    SCTPMessage *sctpshutdowncomplete = new SCTPMessage();
    sctpshutdowncomplete->setBitLength(SCTP_COMMON_HEADER*8);

    sctpEV3 << "SCTPAssociationUtil:sendShutdownComplete\n";

    sctpshutdowncomplete->setSrcPort(localPort);
    sctpshutdowncomplete->setDestPort(remotePort);
    SCTPShutdownCompleteChunk* shutdownCompleteChunk = new SCTPShutdownCompleteChunk("SHUTDOWN_COMPLETE");
    shutdownCompleteChunk->setChunkType(SHUTDOWN_COMPLETE);
    shutdownCompleteChunk->setTBit(0);
    shutdownCompleteChunk->setBitLength(SCTP_SHUTDOWN_ACK_LENGTH*8);
    sctpshutdowncomplete->addChunk(shutdownCompleteChunk);
    sendToIP(sctpshutdowncomplete);
}


void SCTPAssociation::sendAbort()
{
    SCTPMessage *msg = new SCTPMessage();
    msg->setBitLength(SCTP_COMMON_HEADER*8);

    sctpEV3 << "SCTPAssociationUtil:sendABORT localPort=" << localPort << "    remotePort=" << remotePort << "\n";

    msg->setSrcPort(localPort);
    msg->setDestPort(remotePort);
    SCTPAbortChunk* abortChunk = new SCTPAbortChunk("ABORT");
    abortChunk->setChunkType(ABORT);
    abortChunk->setT_Bit(0);
    abortChunk->setBitLength(SCTP_ABORT_CHUNK_LENGTH*8);
    msg->addChunk(abortChunk);
    sendToIP(msg, remoteAddr);
}

void SCTPAssociation::sendShutdown()
{
    SCTPMessage *msg = new SCTPMessage();
    msg->setBitLength(SCTP_COMMON_HEADER*8);

    sctpEV3 << "SCTPAssociationUtil:sendShutdown localPort=" << localPort << "     remotePort=" << remotePort << "\n";

    msg->setSrcPort(localPort);
    msg->setDestPort(remotePort);
    SCTPShutdownChunk* shutdownChunk = new SCTPShutdownChunk("SHUTDOWN");
    shutdownChunk->setChunkType(SHUTDOWN);
    //shutdownChunk->setCumTsnAck(state->lastTsnAck);
    shutdownChunk->setCumTsnAck(state->cTsnAck);
    shutdownChunk->setBitLength(SCTP_SHUTDOWN_CHUNK_LENGTH*8);
    state->initRexmitTimeout = SCTP_TIMEOUT_INIT_REXMIT;
    state->initRetransCounter = 0;
    stopTimer(T5_ShutdownGuardTimer);
    startTimer(T5_ShutdownGuardTimer, SHUTDOWN_GUARD_TIMEOUT);
    stopTimer(T2_ShutdownTimer);
    startTimer(T2_ShutdownTimer, state->initRexmitTimeout);
    state->shutdownChunk = check_and_cast<SCTPShutdownChunk*>(shutdownChunk->dup());
    msg->addChunk(shutdownChunk);
    sendToIP(msg, remoteAddr);
    performStateTransition(SCTP_E_NO_MORE_OUTSTANDING);
}


void SCTPAssociation::retransmitShutdown()
{
    SCTPMessage *sctpmsg = new SCTPMessage();
    sctpmsg->setBitLength(SCTP_COMMON_HEADER*8);
    SCTPShutdownChunk* shutdownChunk;
    shutdownChunk = check_and_cast<SCTPShutdownChunk*>(state->shutdownChunk->dup());
    sctpmsg->addChunk(shutdownChunk);

    sctpEV3 << "retransmitShutdown localAddr=" << localAddr << "  remoteAddr" << remoteAddr << "\n";

    sendToIP(sctpmsg);
}

void SCTPAssociation::retransmitShutdownAck()
{
    SCTPMessage *sctpmsg = new SCTPMessage();
    sctpmsg->setBitLength(SCTP_COMMON_HEADER*8);
    SCTPShutdownAckChunk* shutdownAckChunk;
    shutdownAckChunk = check_and_cast<SCTPShutdownAckChunk*>(state->shutdownAckChunk->dup());
    sctpmsg->addChunk(shutdownAckChunk);

    sctpEV3 << "retransmitShutdownAck localAddr=" << localAddr << "  remoteAddr" << remoteAddr << "\n";

    sendToIP(sctpmsg);
}


void SCTPAssociation::scheduleSack()
{
    /* increase SACK counter, we received another data PACKET */
    if (state->firstChunkReceived)
        state->ackState++;
    else
    {
        state->ackState = sackFrequency;
        state->firstChunkReceived = true;
    }

    sctpEV3 << "scheduleSack() : ackState is now: " << state->ackState << "\n";

    if (state->ackState <= sackFrequency - 1)
    {
        /* start a SACK timer if none is running, to expire 200 ms (or parameter) from now */
        if (!SackTimer->isScheduled())
        {
            startTimer(SackTimer, sackPeriod);
        }
        /* else: leave timer running, and do nothing... */ else {
            /* is this possible at all ? Check this... */

            sctpEV3 << "SACK timer running, but scheduleSack() called\n";

        }
    }
}


SCTPSackChunk* SCTPAssociation::createSack()
{
    uint32 key = 0, arwnd = 0;

    sctpEV3 << "SCTPAssociationUtil:createSACK localAddress=" << localAddr << "  remoteAddress=" << remoteAddr << "\n";

    sctpEV3 << " localRwnd=" << state->localRwnd << " queuedBytes=" << state->queuedReceivedBytes << "\n";
    if ((int32)(state->localRwnd - state->queuedReceivedBytes) <= 0)
    {
        arwnd = 0;
        if (state->swsLimit > 0)
            state->swsAvoidanceInvoked = true;
    }
    else if (state->localRwnd - state->queuedReceivedBytes < state->swsLimit || state->swsAvoidanceInvoked == true)
    {
        arwnd = 1;
        if (state->swsLimit > 0)
            state->swsAvoidanceInvoked = true;
        sctpEV3 << "arwnd=1; createSack : SWS Avoidance ACTIVE !!!\n";
    }
    else
    {
        arwnd = state->localRwnd - state->queuedReceivedBytes;
        sctpEV3 << simTime() << " arwnd = " << state->localRwnd << " - " << state->queuedReceivedBytes << " = " << arwnd << "\n";
    }
    advRwnd->record(arwnd);
    SCTPSackChunk* sackChunk = new SCTPSackChunk("SACK");
    sackChunk->setChunkType(SACK);
    sackChunk->setCumTsnAck(state->cTsnAck);
    sackChunk->setA_rwnd(arwnd);
    uint32 numGaps = state->numGaps;
    uint32 numDups = state->dupList.size();
    uint16 sackLength = SCTP_SACK_CHUNK_LENGTH + numGaps*4 + numDups*4;
    uint32 mtu = getPath(remoteAddr)->pmtu;

    if (sackLength > mtu-32) // FIXME
    {
        if (SCTP_SACK_CHUNK_LENGTH + numGaps*4 > mtu-32)
        {
            numDups = 0;
            numGaps = (uint32)((mtu-32-SCTP_SACK_CHUNK_LENGTH)/4);
        }
        else
        {
            numDups = (uint32)((mtu-32-SCTP_SACK_CHUNK_LENGTH - numGaps*4)/4);
        }
        sackLength = SCTP_SACK_CHUNK_LENGTH + numGaps*4 + numDups*4;
    }
    sackChunk->setNumGaps(numGaps);
    sackChunk->setNumDupTsns(numDups);
    sackChunk->setBitLength(sackLength*8);

    sctpEV3 << "Sack arwnd=" << sackChunk->getA_rwnd() << " ctsnAck=" << state->cTsnAck << " numGaps=" << numGaps << " numDups=" << numDups << "\n";

    if (numGaps > 0)
    {
        sackChunk->setGapStartArraySize(numGaps);
        sackChunk->setGapStopArraySize(numGaps);

        uint32 last = state->cTsnAck;
        for (key=0; key<numGaps; key++)
        {
            // ====== Validity check ===========================================
            assert(tsnGt(state->gapStartList[key], last + 1));
            assert(tsnGe(state->gapStopList[key], state->gapStartList[key]));
            last = state->gapStopList[key];

            sackChunk->setGapStart(key, state->gapStartList[key]);
            sackChunk->setGapStop(key, state->gapStopList[key]);
        }
    }
    if (numDups > 0)
    {
        sackChunk->setDupTsnsArraySize(numDups);
        key = 0;
        for (std::list<uint32>::iterator iter=state->dupList.begin(); iter!=state->dupList.end(); iter++)
        {
            sackChunk->setDupTsns(key, (*iter));
            key++;
            if (key == numDups)
                break;
        }
        state->dupList.clear();
    }
    sctpEV3 << endl;
    for (uint32 i=0; i<numGaps; i++)
        sctpEV3 << sackChunk->getGapStart(i) << " - " << sackChunk->getGapStop(i) << "\n";

    sctpEV3 << "send " << sackChunk->getName() << " from " << localAddr << " to " << state->lastDataSourceAddress << "\n";
    return sackChunk;
}

void SCTPAssociation::sendSack()
{
    SCTPSackChunk*               sackChunk;

    sctpEV3 << "Sending SACK" << endl;

    /* sack timer has expired, reset flag, and send SACK */
    stopTimer(SackTimer);
    state->ackState = 0;
    sackChunk = createSack();

    SCTPMessage* sctpmsg = new SCTPMessage();
    sctpmsg->setBitLength(SCTP_COMMON_HEADER*8);
    sctpmsg->addChunk(sackChunk);

    // Return the SACK to the address where we last got a data chunk from
    sendToIP(sctpmsg, state->lastDataSourceAddress);
}

void SCTPAssociation::sendDataArrivedNotification(uint16 sid)
{

    sctpEV3 << "SendDataArrivedNotification\n";

    cPacket* cmsg = new cPacket("DataArrivedNotification");
    cmsg->setKind(SCTP_I_DATA_NOTIFICATION);
    SCTPCommand *cmd = new SCTPCommand("notification");
    cmd->setAssocId(assocId);
    cmd->setSid(sid);
    cmd->setNumMsgs(1);
    cmsg->setControlInfo(cmd);

    sendToApp(cmsg);
}


void SCTPAssociation::putInDeliveryQ(uint16 sid)
{
    SCTPReceiveStreamMap::iterator iter = receiveStreams.find(sid);
    SCTPReceiveStream* rStream = iter->second;
    sctpEV3 << "putInDeliveryQ: SSN=" << rStream->getExpectedStreamSeqNum()
                      << " SID=" << sid
                      << " QueueSize=" << rStream->getOrderedQ()->getQueueSize() << endl;
    while (rStream->getOrderedQ()->getQueueSize()>0)
    {
        /* dequeue first from reassembly Q */
        SCTPDataVariables* chunk =
                rStream->getOrderedQ()-> dequeueChunkBySSN(rStream->getExpectedStreamSeqNum());
        if (chunk) {
            sctpEV3 << "putInDeliveryQ::chunk " << chunk->tsn
                    <<", sid " << chunk->sid << " and ssn " << chunk->ssn
                    <<" dequeued from ordered queue. queuedReceivedBytes="
                    << state->queuedReceivedBytes << " will be reduced by "
                    << chunk->len/8 << endl;
            state->queuedReceivedBytes -= chunk->len/8;


            qCounter.roomSumRcvStreams -= ADD_PADDING(chunk->len/8 + SCTP_DATA_CHUNK_LENGTH);
            if (rStream->getDeliveryQ()->checkAndInsertChunk(chunk->tsn, chunk)) {
                state->queuedReceivedBytes += chunk->len/8;

                sctpEV3 << "data put in deliveryQ; queuedBytes now "
                        << state->queuedReceivedBytes << endl;
                qCounter.roomSumRcvStreams += ADD_PADDING(chunk->len/8 + SCTP_DATA_CHUNK_LENGTH);
                int32 seqnum = rStream->getExpectedStreamSeqNum();
                rStream->setExpectedStreamSeqNum(++seqnum);
                if (rStream->getExpectedStreamSeqNum() > 65535) {
                    rStream->setExpectedStreamSeqNum(0);
                }
                sendDataArrivedNotification(sid);
            }
        }
        else {
            break;
        }
    }
}

void SCTPAssociation::pushUlp()
{
    int32                    count = 0;

    for (unsigned int i = 0; i < inboundStreams; i++) { //12.06.08
        putInDeliveryQ(i);
    }
    if (state->pushMessagesLeft <= 0) {
        state->pushMessagesLeft = state->messagesToPush;
    }
    bool restrict = false;
    if (state->pushMessagesLeft > 0) {
        restrict = true;
    }


    sctpEV3 << simTime() << " Calling pushUlp(" << state->queuedReceivedBytes
            << " bytes queued) ..." << endl;
    uint32 i = state->nextRSid;
    do {
        SCTPReceiveStreamMap::iterator iter = receiveStreams.find(i);
        SCTPReceiveStream* rStream = iter->second;
        sctpEV3 << "Size of stream " << iter->first << ": "
                << rStream->getDeliveryQ()->getQueueSize() << endl;

        while ( (!rStream->getDeliveryQ()->payloadQueue.empty()) &&
                (!restrict || (restrict && state->pushMessagesLeft>0)) ) {
            SCTPDataVariables* chunk = rStream->getDeliveryQ()->extractMessage();
            qCounter.roomSumRcvStreams -= ADD_PADDING(chunk->len/8 + SCTP_DATA_CHUNK_LENGTH);

            if (state->pushMessagesLeft > 0)
                state->pushMessagesLeft--;

            state->queuedReceivedBytes -= chunk->len/8;
            if (state->swsAvoidanceInvoked) {
                if ((int32)(state->localRwnd - state->queuedReceivedBytes) >= (int32)(state->swsLimit) &&
                        (int32)(state->localRwnd - state->queuedReceivedBytes) <= (int32)(state->swsLimit+state->assocPmtu)) {
                    /* only if the window has opened up more than one MTU we will send a SACK */
                    state->swsAvoidanceInvoked = false;
                    sctpEV3 << "pushUlp: Window opens up to " << (int32)state->localRwnd-state->queuedReceivedBytes << " bytes: sending a SACK. SWS Avoidance INACTIVE\n";

                    sendSack();
                }
            }
            else if ((int32)(state->swsLimit) == 0) {
                sendSack();
            }
            sctpEV3 << "Push TSN " << chunk->tsn
                    << ": sid=" << chunk->sid << " ssn=" << chunk->ssn << endl;
            cPacket* msg = (cPacket *)chunk->userData;
            msg->setKind(SCTP_I_DATA);
            SCTPRcvCommand *cmd = new SCTPRcvCommand("push");
            cmd->setAssocId(assocId);
            cmd->setGate(appGateIndex);
            cmd->setSid(chunk->sid);
            cmd->setSsn(chunk->ssn);
            cmd->setSendUnordered(!chunk->ordered);
            cmd->setLocalAddr(localAddr);
            cmd->setRemoteAddr(remoteAddr);
            cmd->setPpid(chunk->ppid);
            cmd->setTsn(chunk->tsn);
            cmd->setCumTsn(state->lastTsnAck);
            msg->setControlInfo(cmd);
            state->numMsgsReq[count]--;
            delete chunk;
            sendToApp(msg);
        }
        i = (i + 1) % inboundStreams;
        count++;
    } while (i != state->nextRSid);

    state->nextRSid = (state->nextRSid + 1) % inboundStreams;
    if ( (state->queuedReceivedBytes == 0) && (fsm->getState() == SCTP_S_SHUTDOWN_ACK_SENT)) {
        sctpEV3 << "SCTP_E_CLOSE" << endl;
        performStateTransition(SCTP_E_CLOSE);
    }
}

SCTPDataChunk* SCTPAssociation::transformDataChunk(SCTPDataVariables* chunk)
{
    SCTPDataChunk*       dataChunk = new SCTPDataChunk("DATA");
    SCTPSimpleMessage* msg = check_and_cast<SCTPSimpleMessage*>(chunk->userData->dup());
    dataChunk->setChunkType(DATA);
    dataChunk->setBBit(chunk->bbit);
    dataChunk->setEBit(chunk->ebit);
    if (chunk->ordered) {
        dataChunk->setUBit(0);
    }
    else {
        dataChunk->setUBit(1);
    }
    dataChunk->setTsn(chunk->tsn);
    dataChunk->setSid(chunk->sid);
    dataChunk->setSsn(chunk->ssn);
    dataChunk->setPpid(chunk->ppid);
    dataChunk->setEnqueuingTime(chunk->enqueuingTime);
    dataChunk->setBitLength(SCTP_DATA_CHUNK_LENGTH*8);
    msg->setBitLength(chunk->len);
    dataChunk->encapsulate(msg);
    return dataChunk;
}

void SCTPAssociation::addPath(const IPvXAddress& addr)
{
    sctpEV3 << "Add Path remote address: " << addr << "\n";

    SCTPPathMap::iterator i = sctpPathMap.find(addr);
    if (i==sctpPathMap.end())
    {
        sctpEV3 << __LINE__ << " get new path for " << addr << "\n";
        SCTPPathVariables* path = new SCTPPathVariables(addr, this);
        sctpPathMap[addr] = path;
        qCounter.roomTransQ[addr] = 0;
        qCounter.bookedTransQ[addr] = 0;
        qCounter.roomRetransQ[addr] = 0;
    }
    sctpEV3 << "path added\n";
}

void SCTPAssociation::removePath(const IPvXAddress& addr)
{
    SCTPPathMap::iterator pathIterator = sctpPathMap.find(addr);
    if (pathIterator != sctpPathMap.end())
    {
        SCTPPathVariables* path = pathIterator->second;
        path->cwnd = 0;
        path->ssthresh = 0;
        recordCwndUpdate(path);

        stopTimer(path->HeartbeatTimer);
        delete path->HeartbeatTimer;
        stopTimer(path->HeartbeatIntervalTimer);
        delete path->HeartbeatIntervalTimer;
        stopTimer(path->T3_RtxTimer);
        delete path->T3_RtxTimer;
        stopTimer(path->CwndTimer);
        delete path->CwndTimer;
        sctpPathMap.erase(pathIterator);
        delete path;
    }
}

void SCTPAssociation::deleteStreams()
{
    for (SCTPSendStreamMap::iterator it=sendStreams.begin(); it != sendStreams.end(); it++)
    {
        it->second->deleteQueue();
    }
    for (SCTPReceiveStreamMap::iterator it=receiveStreams.begin(); it != receiveStreams.end(); it++)
    {
        delete it->second;
    }
}

bool SCTPAssociation::makeRoomForTsn(const uint32 tsn, const uint32 length, const bool uBit)
{
    SCTPQueue* stream, dStream;
    uint32 sum = 0;
    uint32 comp = 0;
    bool     delQ = false;
    uint32 high = state->highestTsnStored;

    sctpEV3 << "makeRoomForTsn: tsn=" << tsn
            << ", length=" << length << " high=" << high << endl;
    while ((sum < length) && (state->highestTsnReceived>state->lastTsnAck)) {
        comp = sum;
        for (SCTPReceiveStreamMap::iterator iter = receiveStreams.begin();
                iter!=receiveStreams.end(); iter++) {
            if (tsn > high) {
                return false;
            }
            if (uBit) {
                stream = iter->second->getUnorderedQ();
            }
            else {
                stream = iter->second->getOrderedQ();
            }
            SCTPDataVariables* chunk = stream->getChunk(high);
            if (chunk == NULL) {      //12.06.08
                sctpEV3 << high << " not found in orderedQ. Try deliveryQ" << endl;
                stream = iter->second->getDeliveryQ();
                chunk = stream->getChunk(high);
                delQ = true;
            }
            if (chunk != NULL) {
                sum += chunk->len;
                if (stream->deleteMsg(high)) {
                    sctpEV3 << high << " found and deleted" << endl;

                    state->queuedReceivedBytes -= chunk->len/8; //12.06.08
                    if (ssnGt(iter->second->getExpectedStreamSeqNum(), chunk->ssn)) {
                        iter->second->setExpectedStreamSeqNum(chunk->ssn);
                    }
                }
                qCounter.roomSumRcvStreams -= ADD_PADDING(chunk->len/8 + SCTP_DATA_CHUNK_LENGTH);
                if (high == state->highestTsnReceived) {
                    state->highestTsnReceived--;
                }
                removeFromGapList(high);

                if (tsn > state->highestTsnReceived) {
                    state->highestTsnReceived = tsn;
                }
                high--;
                break;
            }
            else {
                sctpEV3 << "TSN " << high << " not found in stream "
                        << iter->second->getStreamId() << endl;
            }
        }

        if (comp == sum) {
            sctpEV3 << high << " not found in any stream" << endl;
            high--;
        }
        state->highestTsnStored = high;

        if (tsn > state->highestTsnReceived) {
            return false;
        }
    }
    return true;
}

bool SCTPAssociation::tsnIsDuplicate(const uint32 tsn) const
{
    for (std::list<uint32>::const_iterator iterator = state->dupList.begin();
            iterator != state->dupList.end(); iterator++)
    {
        if ((*iterator) == tsn)
            return true;
    }
    for (uint32 i=0; i < state->numGaps; i++) {
        if (tsnBetween(state->gapStartList[i], tsn, state->gapStopList[i])) {
            return true;
        }
    }
    return false;
}

void SCTPAssociation::removeFromGapList(uint32 removedTsn)
{
    int32 gapsize, numgaps;

    numgaps = state->numGaps;
    sctpEV3 << "remove TSN " << removedTsn << " from GapList. " << numgaps << " gaps present, cumTsnAck=" << state->cTsnAck << "\n";
    for (int32 j=0; j<numgaps; j++)
        sctpEV3 << state->gapStartList[j] << " - " << state->gapStopList[j] << "\n";
    for (int32 i=numgaps-1; i>=0; i--)
    {
        sctpEV3 << "gapStartList[" << i << "]=" << state->gapStartList[i] << ", state->gapStopList[" << i << "]=" << state->gapStopList[i] << "\n";
        if (tsnBetween(state->gapStartList[i], removedTsn, state->gapStopList[i]))
        {
            gapsize = (int32)(state->gapStopList[i] - state->gapStartList[i]+1);
            if (gapsize>1)
            {
                if (state->gapStopList[i]==removedTsn)
                {
                    state->gapStopList[i]--;
                }
                else if (state->gapStartList[i]==removedTsn)
                {
                    state->gapStartList[i]++;
                }
                else //gap is split in two
                {
                    for (int32 j=numgaps-1; j>=i; j--)
                    {
                        state->gapStopList[j+1] = state->gapStopList[j];
                        state->gapStartList[j+1] = state->gapStartList[j];
                    }
                    state->gapStopList[i] = removedTsn-1;
                    state->gapStartList[i+1] = removedTsn+1;
                    state->numGaps = min(state->numGaps + 1, MAX_GAP_COUNT);      // T.D. 18.12.09: Enforce upper limit!
                }
            }
            else
            {
                for (int32 j=i; j<=numgaps-1; j++)
                {
                    state->gapStopList[j] = state->gapStopList[j+1];
                    state->gapStartList[j] = state->gapStartList[j+1];
                }
                state->gapStartList[numgaps-1] = 0;
                state->gapStopList[numgaps-1] = 0;
                state->numGaps--;
                if (state->numGaps == 0)
                {
                    if (removedTsn == state->lastTsnAck+1)
                    {
                        state->lastTsnAck = removedTsn;
                    }
                }
            }
        }
    }
    if (state->numGaps>0)
        state->highestTsnReceived = state->gapStopList[state->numGaps-1];
    else
        state->highestTsnReceived = state->cTsnAck;
}

bool SCTPAssociation::updateGapList(const uint32 receivedTsn)
{
    sctpEV3 << "Entering updateGapList (tsn=" << receivedTsn
            << " cTsnAck=" << state->cTsnAck << " Number of Gaps="
            << state->numGaps << endl;

    uint32 lo = state->cTsnAck + 1;
    if ((int32)(state->localRwnd-state->queuedReceivedBytes) <= 0)
    {
        sctpEV3 << "Window full" << endl;
        // Only check if cumTsnAck can be advanced
        if (receivedTsn == lo) {
            sctpEV3 << "Window full, but cumTsnAck can be advanced:" << lo << endl;
        }
        else
            return false;
    }

    if (tsnGt(receivedTsn, state->highestTsnStored)) {    // 17.06.08
        state->highestTsnStored = receivedTsn;
    }

    for (uint32 i = 0; i<state->numGaps; i++) {
        if (state->gapStartList[i] > 0) {
            const uint32 hi = state->gapStartList[i] - 1;
            if (tsnBetween(lo, receivedTsn, hi)) {
                const uint32 gapsize = hi - lo + 1;
                if (gapsize > 1) {
                    /**
                     * TSN either sits at the end of one gap, and thus changes gap
                     * boundaries, or it is in between two gaps, and becomes a new gap
                     */
                    if (receivedTsn == hi) {
                        state->gapStartList[i] = receivedTsn;
                        state->newChunkReceived = true;
                        return true;
                    }
                    else if (receivedTsn == lo) {
                        if (receivedTsn == (state->cTsnAck + 1)) {
                            state->cTsnAck++;
                            state->newChunkReceived = true;
                            return true;
                        }
                        /* some gap must increase its upper bound */
                        state->gapStopList[i-1] = receivedTsn;
                        state->newChunkReceived = true;
                        return true;
                    }
                    else {  /* a gap in between */
                        state->numGaps = min(state->numGaps + 1, MAX_GAP_COUNT);      // T.D. 18.12.09: Enforce upper limit!

                        for (uint32 j = state->numGaps - 1; j > i; j--) {    // T.D. 18.12.09: Fixed invalid start value.
                            state->gapStartList[j] = state->gapStartList[j-1];
                            state->gapStopList[j] = state->gapStopList[j-1];
                        }
                        state->gapStartList[i] = receivedTsn;
                        state->gapStopList[i] = receivedTsn;
                        state->newChunkReceived = true;
                        return true;
                    }
                }
                else {  /* alright: gapsize is 1: our received tsn may close gap between fragments */
                    if (lo == state->cTsnAck + 1) {
                        state->cTsnAck = state->gapStopList[i];
                        if (i == state->numGaps-1) {
                            state->gapStartList[i] = 0;
                            state->gapStopList[i] = 0;
                        }
                        else {
                            for (uint32 j = i; j < state->numGaps - 1; j++) {        // T.D. 18.12.09: Fixed invalid end value.
                                state->gapStartList[j] = state->gapStartList[j + 1];
                                state->gapStopList[j] = state->gapStopList[j + 1];
                            }
                        }
                        state->numGaps--;
                        state->newChunkReceived = true;
                        return true;
                    }
                    else {
                        state->gapStopList[i-1] = state->gapStopList[i];
                        if (i == state->numGaps-1) {
                            state->gapStartList[i] = 0;
                            state->gapStopList[i] = 0;
                        }
                        else {
                            for (uint32 j = i; j < state->numGaps - 1; j++) {        // T.D. 18.12.09: Fixed invalid end value.
                                state->gapStartList[j] = state->gapStartList[j + 1];
                                state->gapStopList[j] = state->gapStopList[j + 1];
                            }
                        }
                        state->numGaps--;
                        state->newChunkReceived = true;
                        return true;
                    }
                }
            }
            else {  /* receivedTsn is not in the gap between these fragments... */
                lo = state->gapStopList[i] + 1;
            }
        } /* end: for */
    }/* end: for */

    /* (NULL LIST)   OR  (End of Gap List passed) */
    if (receivedTsn == lo) {    // just increase ctsna, handle further update of ctsna later
        if (receivedTsn == state->cTsnAck + 1) {
            state->cTsnAck = receivedTsn;
            state->newChunkReceived = true;
            return true;
        }
        /* Update last fragment....increase stop_tsn by one */
        state->gapStopList[state->numGaps-1]++;

        state->newChunkReceived = true;
        return true;

    }
    else {  // A new fragment altogether, past the end of the list
        if (state->numGaps + 1 <= MAX_GAP_COUNT) {     // T.D. 18.12.09: Enforce upper limit!
            state->gapStartList[state->numGaps] = receivedTsn;
            state->gapStopList[state->numGaps] = receivedTsn;
            state->numGaps++;
            state->newChunkReceived = true;
        }
        return true;
    }

    return false;
}

bool SCTPAssociation::advanceCtsna()
{
    int32 listLength, counter;

    ev << "Entering advanceCtsna(ctsna now ==" << state->cTsnAck << "\n";;

    listLength = state->numGaps;

    /* if there are no fragments, we cannot advance the ctsna */
    if (listLength == 0) return false;
    counter = 0;

    while (counter < listLength)
    {
        /* if we take out a fragment here, we need to modify either counter or list_length */

        if (state->cTsnAck + 1 == state->gapStartList[0])
        {
            /* BINGO ! */
            state->cTsnAck = state->gapStopList[0];
            /* we can take out a maximum of list_length fragments */
            counter++;
            for (uint32 i=1; i<state->numGaps; i++)
            {
                state->gapStartList[i-1] = state->gapStartList[i];
                state->gapStopList[i-1] = state->gapStopList[i];
            }

        }
        else
        {
            ev << "Entering advanceCtsna(when leaving: ctsna==" << state->cTsnAck << "\n";
            return false;
        }

    }    /* end while */

    ev << "Entering advanceCtsna(when leaving: ctsna==" << state->cTsnAck << "\n";
    return true;
}

SCTPDataVariables* SCTPAssociation::makeVarFromMsg(SCTPDataChunk* dataChunk)
{
    SCTPDataVariables* chunk = new SCTPDataVariables();

    chunk->bbit = dataChunk->getBBit();
    chunk->ebit = dataChunk->getEBit();
    chunk->sid = dataChunk->getSid();
    chunk->ssn = dataChunk->getSsn();
    chunk->ppid = dataChunk->getPpid();
    chunk->tsn = dataChunk->getTsn();
    if (!dataChunk->getUBit()) {
        chunk->ordered = true;
    }
    else {
        chunk->ordered = false;
    }
    SCTPSimpleMessage* smsg = check_and_cast<SCTPSimpleMessage*>(dataChunk->decapsulate());

    chunk->userData = smsg;
    chunk->len = smsg->getDataLen()*8;

    sctpEV3 << "makeVarFromMsg: queuedBytes has been increased to "
            << state->queuedReceivedBytes << endl;
    return chunk;
}



SCTPDataVariables* SCTPAssociation::getOutboundDataChunk(const SCTPPathVariables* path,
        const int32                  availableSpace,
        const int32                  availableCwnd)
{
    /* are there chunks in the transmission queue ? If Yes -> dequeue and return it */
    sctpEV3 << "getOutboundDataChunk(" << path->remoteAddress << "):"
            << " availableSpace=" << availableSpace
            << " availableCwnd=" << availableCwnd
            << endl;
    if (!transmissionQ->payloadQueue.empty()) {
        for (SCTPQueue::PayloadQueue::iterator it = transmissionQ->payloadQueue.begin();
                it != transmissionQ->payloadQueue.end(); it++) {
            SCTPDataVariables* chunk = it->second;
            if ( (chunkHasBeenAcked(chunk) == false) &&
                    (chunk->getNextDestinationPath() == path) ) {
                const int32 len = ADD_PADDING(chunk->len/8+SCTP_DATA_CHUNK_LENGTH);
                sctpEV3 << "getOutboundDataChunk() found chunk " << chunk->tsn
                        <<" in the transmission queue, length=" << len << endl;
                if ((len <= availableSpace) &&
                        ((int32)chunk->booksize <= availableCwnd)) {
                    // T.D. 05.01.2010: The bookkeeping counters may only be decreased when
                    //                        this chunk is actually dequeued. Therefore, the check
                    //                        for "chunkHasBeenAcked==false" has been moved into the
                    //                        "if" statement above!
                    transmissionQ->payloadQueue.erase(it);
                    chunk->enqueuedInTransmissionQ = false;
                    CounterMap::iterator i = qCounter.roomTransQ.find(path->remoteAddress);
                    i->second -= ADD_PADDING(chunk->len/8+SCTP_DATA_CHUNK_LENGTH);
                    CounterMap::iterator ib = qCounter.bookedTransQ.find(path->remoteAddress);
                    ib->second -= chunk->booksize;
                    return chunk;
                }
            }
        }
    }
    return NULL;
}


SCTPDataVariables* SCTPAssociation::peekAbandonedChunk(const SCTPPathVariables* path)
{
    // Are there chunks in the retransmission queue? If Yes -> dequeue and return it.
    if (!retransmissionQ->payloadQueue.empty())
    {
        for (SCTPQueue::PayloadQueue::iterator it = retransmissionQ->payloadQueue.begin();
                it != retransmissionQ->payloadQueue.end(); it++) {
            SCTPDataVariables* chunk = it->second;
            sctpEV3 << "peek Chunk " << chunk->tsn << "\n";
            if (chunk->getLastDestinationPath() == path && chunk->hasBeenAbandoned) {
                sctpEV3 << "peekAbandonedChunk() found chunk in the retransmission queue\n";
                return chunk;
            }
        }
    }
    return NULL;
}


SCTPDataMsg* SCTPAssociation::peekOutboundDataMsg()
{
    SCTPDataMsg* datMsg = NULL;
    int32 nextStream = -1;
    nextStream = (this->*ssFunctions.ssGetNextSid)(true);

    if (nextStream == -1)
    {

        sctpEV3 << "peekOutboundDataMsg(): no valid stream found -> returning NULL !\n";

        return NULL;
    }


    for (SCTPSendStreamMap::iterator iter=sendStreams.begin(); iter!=sendStreams.end(); ++iter)
    {
        if ((int32)iter->first==nextStream)
        {
            SCTPSendStream* stream = iter->second;
            if (!stream->getUnorderedStreamQ()->empty())
            {
                return (datMsg);

            }
            if (!stream->getStreamQ()->empty())
            {
                return (datMsg);

            }
        }
    }
    return NULL;

}

SCTPDataMsg* SCTPAssociation::dequeueOutboundDataMsg(const int32 availableSpace,
        const int32 availableCwnd)
{
    SCTPDataMsg* datMsg = NULL;
    int32 nextStream = -1;

    sctpEV3 << "dequeueOutboundDataMsg: " << availableSpace << " bytes left to be sent" << endl;
    /* Only change stream if we don't have to finish a fragmented message */
    if (state->lastMsgWasFragment)
        nextStream = state->lastStreamScheduled;
    else
        nextStream = (this->*ssFunctions.ssGetNextSid)(false);

    if (nextStream == -1)
        return NULL;

    sctpEV3 << "dequeueOutboundDataMsg: now stream " << nextStream << endl;

    for (SCTPSendStreamMap::iterator iter=sendStreams.begin(); iter!=sendStreams.end(); ++iter)
    {
        if ((int32)iter->first==nextStream)
        {
            SCTPSendStream* stream = iter->second;
            cQueue* streamQ = NULL;

            if (!stream->getUnorderedStreamQ()->empty())
            {
                streamQ = stream->getUnorderedStreamQ();
                sctpEV3 << "DequeueOutboundDataMsg() found chunks in stream " << iter->first << " unordered queue, queue size=" << stream->getUnorderedStreamQ()->getLength() << "\n";
            }
            else if (!stream->getStreamQ()->empty())
            {
                streamQ = stream->getStreamQ();
                sctpEV3 << "DequeueOutboundDataMsg() found chunks in stream " << iter->first << " ordered queue, queue size=" << stream->getStreamQ()->getLength() << "\n";
            }

            if (streamQ)
            {
                int32 b = ADD_PADDING( (check_and_cast<SCTPSimpleMessage*>(((SCTPDataMsg*)streamQ->front())->getEncapsulatedPacket())->getByteLength()+SCTP_DATA_CHUNK_LENGTH));

                /* check if chunk found in queue has to be fragmented */
                if (b > (int32)state->assocPmtu - IP_HEADER_LENGTH - SCTP_COMMON_HEADER)
                {
                    /* START FRAGMENTATION */
                    SCTPDataMsg* datMsgQueued = (SCTPDataMsg*)streamQ->pop();
                    SCTPSimpleMessage *datMsgQueuedSimple = check_and_cast<SCTPSimpleMessage*>(datMsgQueued->getEncapsulatedPacket());

                    SCTPDataMsg* datMsgLastFragment = NULL;
                    uint32 offset = 0;

                    sctpEV3 << "Fragmentation: chunk " << &datMsgQueued << ", size = " << datMsgQueued->getByteLength() << endl;

                    while (datMsgQueued)
                    {
                        /* detemine size of fragment, either max payload or what's left */
                        uint32 msgbytes = state->assocPmtu - IP_HEADER_LENGTH - SCTP_COMMON_HEADER - SCTP_DATA_CHUNK_LENGTH;
                        if (msgbytes > datMsgQueuedSimple->getDataLen() - offset)
                            msgbytes = datMsgQueuedSimple->getDataLen() - offset;

                        /* new DATA msg */
                        SCTPDataMsg* datMsgFragment = new SCTPDataMsg();
                        datMsgFragment->setSid(datMsgQueued->getSid());
                        datMsgFragment->setPpid(datMsgQueued->getPpid());
                        datMsgFragment->setInitialDestination(datMsgQueued->getInitialDestination());
                        datMsgFragment->setEnqueuingTime(datMsgQueued->getEnqueuingTime());
                        datMsgFragment->setMsgNum(datMsgQueued->getMsgNum());
                        datMsgFragment->setOrdered(datMsgQueued->getOrdered());
                        datMsgFragment->setExpiryTime(datMsgQueued->getExpiryTime());
                        datMsgFragment->setRtx(datMsgQueued->getRtx());
                        datMsgFragment->setFragment(true);
                        datMsgFragment->setBooksize(msgbytes + state->header);

                        /* is this the first fragment? */
                        if (offset == 0)
                            datMsgFragment->setBBit(true);

                        /* new msg */
                        SCTPSimpleMessage *datMsgFragmentSimple = new SCTPSimpleMessage();

                        datMsgFragmentSimple->setName(datMsgQueuedSimple->getName());
                        datMsgFragmentSimple->setCreationTime(datMsgQueuedSimple->getCreationTime());

                        datMsgFragmentSimple->setDataArraySize(msgbytes);
                        datMsgFragmentSimple->setDataLen(msgbytes);
                        datMsgFragmentSimple->setByteLength(msgbytes);

                        /* copy data */
                        for (uint32 i = offset; i < offset + msgbytes; i++)
                            datMsgFragmentSimple->setData(i - offset, datMsgQueuedSimple->getData(i));

                        offset += msgbytes;
                        datMsgFragment->encapsulate(datMsgFragmentSimple);

                        /* insert fragment into queue */
                        if (!streamQ->empty())
                        {
                            if (!datMsgLastFragment)
                            {
                                /* insert first fragment at the begining of the queue*/
                                streamQ->insertBefore((SCTPDataMsg*)streamQ->front(), datMsgFragment);
                            }
                            else
                            {
                                /* insert fragment after last inserted   */
                                streamQ->insertAfter(datMsgLastFragment, datMsgFragment);
                            }
                        }
                        else
                            streamQ->insert(datMsgFragment);

                        state->queuedMessages++;
                        qCounter.roomSumSendStreams += ADD_PADDING(datMsgFragment->getByteLength() + SCTP_DATA_CHUNK_LENGTH);
                        qCounter.bookedSumSendStreams += datMsgFragment->getBooksize();
                        sctpEV3 << "Fragmentation: fragment " << &datMsgFragment << " created, length = " << datMsgFragmentSimple->getByteLength() << ", queue size = " << streamQ->getLength() << endl;

                        datMsgLastFragment = datMsgFragment;

                        /* all fragments done? */
                        if (datMsgQueuedSimple->getDataLen() == offset)
                        {
                            datMsgFragment->setEBit(true);

                            /* remove original element */
                            sctpEV3 << "Fragmentation: delete " << &datMsgQueued << endl;
                            //streamQ->pop();
                            qCounter.roomSumSendStreams -= ADD_PADDING(datMsgQueued->getByteLength() + SCTP_DATA_CHUNK_LENGTH);
                            qCounter.bookedSumSendStreams -= datMsgQueued->getBooksize();
                            delete datMsgQueued;
                            datMsgQueued = NULL;
                            state->queuedMessages--;
                        }
                    }

                    /* the next chunk returned will always be a fragment */
                    state->lastMsgWasFragment = true;

                    b = ADD_PADDING( (check_and_cast<SCTPSimpleMessage*>(((SCTPDataMsg*)streamQ->front())->getEncapsulatedPacket())->getBitLength()/8+SCTP_DATA_CHUNK_LENGTH));
                    /* FRAGMENTATION DONE */
                }

                if ((b <= availableSpace) &&
                        ( (int32)((SCTPDataMsg*)streamQ->front())->getBooksize() <= availableCwnd)) {
                    datMsg = (SCTPDataMsg*)streamQ->pop();
                    /*if (!state->appSendAllowed && streamQ->getLength()<=state->sendQueueLimit)
                    {
                        state->appSendAllowed = true;
                        sendIndicationToApp(SCTP_I_SENDQUEUE_ABATED);
                    }*/
                    sendQueue->record(streamQ->getLength());

                    if (!datMsg->getFragment())
                    {
                        datMsg->setBBit(true);
                        datMsg->setEBit(true);
                        state->lastMsgWasFragment = false;
                    }
                    else
                    {
                        if (datMsg->getEBit())
                            state->lastMsgWasFragment = false;
                        else
                            state->lastMsgWasFragment = true;
                    }

                    sctpEV3 << "DequeueOutboundDataMsg() found chunk (" << &datMsg << ") in the stream queue " << &iter->first << "(" << streamQ << ") queue size=" << streamQ->getLength() << "\n";
                }
            }
            break;
        }
    }
    if (datMsg != NULL)
    {
        qCounter.roomSumSendStreams -= ADD_PADDING( (check_and_cast<SCTPSimpleMessage*>(datMsg->getEncapsulatedPacket())->getBitLength()/8+SCTP_DATA_CHUNK_LENGTH));
        qCounter.bookedSumSendStreams -= datMsg->getBooksize();
    }
    return (datMsg);
}


bool SCTPAssociation::nextChunkFitsIntoPacket(int32 bytes)
{
    int32 nextStream = -1;
    SCTPSendStream* stream;

    /* Only change stream if we don't have to finish a fragmented message */
    if (state->lastMsgWasFragment)
        nextStream = state->lastStreamScheduled;
    else
        nextStream = (this->*ssFunctions.ssGetNextSid)(true);

    if (nextStream == -1)
        return false;

    stream = sendStreams.find(nextStream)->second;

    if (stream)
    {
        cQueue* streamQ = NULL;

        if (!stream->getUnorderedStreamQ()->empty())
            streamQ = stream->getUnorderedStreamQ();
        else if (!stream->getStreamQ()->empty())
            streamQ = stream->getStreamQ();

        if (streamQ)
        {
            int32 b = ADD_PADDING( (check_and_cast<SCTPSimpleMessage*>(((SCTPDataMsg*)streamQ->front())->getEncapsulatedPacket())->getByteLength()+SCTP_DATA_CHUNK_LENGTH));

            /* Check if next message would be fragmented */
            if (b > (int32) state->assocPmtu - IP_HEADER_LENGTH - SCTP_COMMON_HEADER)
            {
                /* Test if fragment fits */
                if (bytes >= (int32) state->assocPmtu - IP_HEADER_LENGTH - SCTP_COMMON_HEADER - SCTP_DATA_CHUNK_LENGTH)
                    return true;
                else
                    return false;
            }

            /* Message doesn't need to be fragmented, just try if it fits */
            if (b <= bytes)
                return true;
            else
                return false;
        }
    }

    return false;
}


SCTPPathVariables* SCTPAssociation::getNextPath(const SCTPPathVariables* oldPath) const
{
    int32 hit = 0;
    if (sctpPathMap.size() > 1) {
        for (SCTPPathMap::const_iterator iterator = sctpPathMap.begin();
                iterator != sctpPathMap.end(); iterator++) {
            if (iterator->second == oldPath) {
                if (++hit == 1) {
                    continue;
                }
                else {
                    break;
                }
            }
            if (iterator->second->activePath) {
                return iterator->second;
            }
        }
    }
    return (NULL);
}

SCTPPathVariables* SCTPAssociation::getNextDestination(SCTPDataVariables* chunk) const
{
    SCTPPathVariables* next;
    SCTPPathVariables* last;

    sctpEV3 << "Running getNextDestination()" << endl;
    if (chunk->numberOfTransmissions == 0) {
        if (chunk->getInitialDestinationPath() == NULL) {
            next = state->getPrimaryPath();
        }
        else {
            next = chunk->getInitialDestinationPath();
        }
    }
    else {
        if (chunk->hasBeenFastRetransmitted) {
            sctpEV3 << "Chunk is scheduled for FastRetransmission. Next destination = "
                    << chunk->getLastDestination() << endl;
            return (chunk->getLastDestinationPath());
        }
        // If this is a retransmission, we should choose another, active path.
        last = chunk->getLastDestinationPath();
        next = getNextPath(last);
        if ( (next == NULL) || (next->confirmed == false) ) {
            next = last;
        }
    }

    sctpEV3 << "getNextDestination(): chunk was last sent to " << last->remoteAddress
            << ", will next be sent to path " << next->remoteAddress << endl;
    return (next);
}


void SCTPAssociation::pmDataIsSentOn(SCTPPathVariables* path)
{
    /* restart hb_timer on this path */
    stopTimer(path->HeartbeatTimer);
    if (state->enableHeartbeats)
    {
        path->heartbeatTimeout = path->pathRto + (double)sctpMain->par("hbInterval");
        startTimer(path->HeartbeatTimer, path->heartbeatTimeout);
        sctpEV3 << "Restarting HB timer on path " << path->remoteAddress
                << " to expire at time " << path->heartbeatTimeout << endl;
    }

    path->cwndTimeout = path->pathRto;
    stopTimer(path->CwndTimer);
    startTimer(path->CwndTimer, path->cwndTimeout);

    sctpEV3 << "Restarting CWND timer on path " << path->remoteAddress
            << " to expire at time " << path->cwndTimeout << endl;
}

void SCTPAssociation::pmStartPathManagement()
{
    RoutingTableAccess routingTableAccess;
    SCTPPathVariables* path;
    int32 i = 0;
    /* populate path structures !!! */
    /* set a high start value...this is appropriately decreased later (below) */
    state->assocPmtu = state->localRwnd;
    for (SCTPPathMap::iterator piter=sctpPathMap.begin(); piter!=sctpPathMap.end(); piter++)
    {
        path = piter->second;
        path->pathErrorCount = 0;
        InterfaceEntry *rtie = routingTableAccess.get()->getInterfaceForDestAddr(path->remoteAddress.get4());
        path->pmtu = rtie->getMTU();
        sctpEV3 << "Path MTU of Interface " << i << " = " << path->pmtu << "\n";
        if (path->pmtu < state->assocPmtu)
        {
            state->assocPmtu = path->pmtu;
        }
        initCCParameters(path);
        path->pathRto = (double)sctpMain->par("rtoInitial");
        path->srtt = path->pathRto;
        path->rttvar = SIMTIME_ZERO;
        /* from now on we may have one update per RTO/SRTT */
        path->updateTime = SIMTIME_ZERO;


        path->partialBytesAcked = 0;
        path->outstandingBytes = 0;
        path->activePath = true;
        // Timer probably not running, but stop it anyway I.R.
        stopTimer(path->T3_RtxTimer);

        if (path->remoteAddress == state->initialPrimaryPath && !path->confirmed) {
            path->confirmed = true;
        }
        sctpEV3 << getFullPath() << " numberOfLocalAddresses=" << state->localAddresses.size() << "\n";
        path->heartbeatTimeout = (double)sctpMain->par("hbInterval")+i*path->pathRto;
        stopTimer(path->HeartbeatTimer);
        sendHeartbeat(path);
        startTimer(path->HeartbeatTimer, path->heartbeatTimeout);
        startTimer(path->HeartbeatIntervalTimer, path->heartbeatIntervalTimeout);
        path->statisticsPathRTO->record(path->pathRto);
        i++;
    }
}


int32 SCTPAssociation::getOutstandingBytes() const
{
    int32 osb = 0;
    for (SCTPPathMap::const_iterator pm = sctpPathMap.begin(); pm != sctpPathMap.end(); pm++) {
        osb += pm->second->outstandingBytes;
    }
    return osb;
}

void SCTPAssociation::pmClearPathCounter(SCTPPathVariables* path)
{
    path->pathErrorCount = 0;
    if (path->activePath == false) {
        /* notify the application */
        pathStatusIndication(path, true);
        sctpEV3 << "Path " << path->remoteAddress
                << " state changes from INACTIVE to ACTIVE !!!" << endl;
    }
}

void SCTPAssociation::pathStatusIndication(const SCTPPathVariables* path,
        const bool                   status)
{
    cPacket* msg = new cPacket("StatusInfo");
    msg->setKind(SCTP_I_STATUS);
    SCTPStatusInfo* cmd = new SCTPStatusInfo();
    cmd->setPathId(path->remoteAddress);
    cmd->setAssocId(assocId);
    cmd->setActive(status);
    msg->setControlInfo(cmd);
    if (!status) {
        SCTP::AssocStatMap::iterator iter = sctpMain->assocStatMap.find(assocId);
        iter->second.numPathFailures++;
    }
    sendToApp(msg);
}

void SCTPAssociation::pmRttMeasurement(SCTPPathVariables* path,
        const simtime_t&     rttEstimation)
{
    if (rttEstimation < MAXTIME) {
        if (simTime() > path->updateTime) {
            if (path->updateTime == SIMTIME_ZERO) {
                path->rttvar = rttEstimation.dbl() / 2;
                path->srtt = rttEstimation;
                path->pathRto = 3.0 * rttEstimation.dbl();
                path->pathRto = max(min(path->pathRto.dbl(), (double)sctpMain->par("rtoMax")),
                        (double)sctpMain->par("rtoMin"));
            }
            else {
                path->rttvar = (1.0 - (double)sctpMain->par("rtoBeta")) * path->rttvar.dbl() +
                        (double)sctpMain->par("rtoBeta") * fabs(path->srtt.dbl() - rttEstimation.dbl());
                path->srtt = (1.0 - (double)sctpMain->par("rtoAlpha")) * path->srtt.dbl() +
                        (double)sctpMain->par("rtoAlpha") * rttEstimation.dbl();
                path->pathRto = path->srtt.dbl() + 4.0 * path->rttvar.dbl();
                path->pathRto = max(min(path->pathRto.dbl(), (double)sctpMain->par("rtoMax")),
                        (double)sctpMain->par("rtoMin"));
            }
            /*
            std::cout << simTime() << ": Updating timer values for path " << path->remoteAddress << ":"
                      << " RTO=" << path->pathRto
                      << " rttEstimation=" << rttEstimation
                      << " SRTT=" << path->srtt
                      << " -->  RTTVAR=" << path->rttvar << endl;
             */
            // RFC 2960, sect. 6.3.1: new RTT measurements SHOULD be made no more
            //                                than once per round-trip.
            path->updateTime = simTime() + path->srtt;
            path->statisticsPathRTO->record(path->pathRto);
            path->statisticsPathRTT->record(rttEstimation);
        }
    }
}

bool SCTPAssociation::allPathsInactive() const
{
    for (SCTPPathMap::const_iterator it = sctpPathMap.begin(); it != sctpPathMap.end(); it++) {
        if (it->second->activePath) {
            return false;
        }
    }
    return true;
}


void SCTPAssociation::disposeOf(SCTPMessage* sctpmsg)
{
    SCTPChunk* chunk;
    uint32 numberOfChunks = sctpmsg->getChunksArraySize();
    if (numberOfChunks>0)
        for (uint32 i=0; i<numberOfChunks; i++)
        {
            chunk = (SCTPChunk*)(sctpmsg->removeChunk());
            if (chunk->getChunkType()==DATA)
                delete (SCTPSimpleMessage*)chunk->decapsulate();
            delete chunk;
        }
    delete sctpmsg;
}

