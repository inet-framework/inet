//
// Copyright (C) 2005-2010 Irene Ruengeler
// Copyright (C) 2009-2012 Thomas Dreibholz
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
#include "SCTPQueue.h"
#include "SCTPAlgorithm.h"
#include "RoutingTable.h"
#include "RoutingTableAccess.h"
#include "InterfaceTable.h"
#include "InterfaceTableAccess.h"
#include "IPv6Address.h"
#include "common.h"
#include "IPv4ControlInfo.h"
#include "IPv6ControlInfo.h"


#ifdef WITH_IPv4
#include "IPv4InterfaceData.h"
#endif

#ifdef WITH_IPv6
#include "IPv6InterfaceData.h"
#endif

#include "UDPControlInfo_m.h"


void SCTPAssociation::calculateRcvBuffer()
{
    if (SCTP::testing == true) {
        uint32 sumDelivery = 0;
        uint32 sumOrdered = 0;
        uint32 sumUnOrdered = 0;
        for (SCTPReceiveStreamMap::const_iterator iterator = receiveStreams.begin();
                iterator != receiveStreams.end(); iterator++) {
            const SCTPReceiveStream* stream = iterator->second;
            sumDelivery += stream->getDeliveryQ()->getQueueSize();
            sumOrdered += stream->getOrderedQ()->getQueueSize();
            sumUnOrdered += stream->getUnorderedQ()->getQueueSize();
        }
        sctpEV3 << "DeliveryQ= " << sumDelivery
                << ", OrderedQ=" << sumOrdered
                << ", UnorderedQ=" << sumUnOrdered
                << ", bufferedMessages=" << state->bufferedMessages
                << endl;
    }
}

void SCTPAssociation::listOrderedQ()
{
    for (SCTPReceiveStreamMap::iterator iter=receiveStreams.begin(); iter!=receiveStreams.end(); iter++)
    {
        sctpEV3 << "stream " << iter->second->getStreamId() << ":\n";
        iter->second->getOrderedQ()->printQueue();
        sctpEV3 << "\n";
    }
}


void SCTPAssociation::printSctpPathMap() const
{
    sctpEV3 <<"SCTP PathMap:" << endl;
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
        CASE(SCTP_E_STREAM_RESET);
        CASE(SCTP_E_SEND_ASCONF);
        CASE(SCTP_E_SET_STREAM_PRIO);
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
        CASE(SCTP_I_ABANDONED);
        CASE(SCTP_I_SEND_STREAMS_RESETTED);
        CASE(SCTP_I_RCV_STREAMS_RESETTED);
        CASE(SCTP_I_RESET_REQUEST_FAILED);
        CASE(SCTP_I_ADDRESS_ADDED);
    }
    return s;
#undef CASE
}


uint16 SCTPAssociation::chunkToInt(const char* type)
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
    if (strcmp(type, "AUTH")==0) return 15;
    if (strcmp(type, "NR-SACK")==0) return 16;
    if (strcmp(type, "ASCONF_ACK")==0) return 128;
    if (strcmp(type, "PKTDROP")==0) return 129;
    if (strcmp(type, "STREAM_RESET")==0) return 130;
    if (strcmp(type, "FORWARD_TSN")==0) return 192;
    if (strcmp(type, "ASCONF")==0) return 193;
    sctpEV3<<"ChunkConversion not successful\n";
    return (0xffff);
}

void SCTPAssociation::printAssocBrief()
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
    sctpEV3 << "initTag "<< sctpmsg->getTag() << "\n";
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
    
    if ((bool)sctpMain->par("auth")) {
        const char* chunks = sctpMain->par("chunks").stringValue();
        bool asc = false;
        bool asca = false;
        char* chunkscopy = (char *)malloc(strlen(chunks)+1);
        strcpy(chunkscopy, chunks);
        char* token;
        token = strtok((char*)chunkscopy, ",");
        while (token != NULL)
        {
            if (chunkToInt(token) == ASCONF)
                asc = true;
            if (chunkToInt(token) == ASCONF_ACK)
                asca = true;
            if (!typeInOwnChunkList(chunkToInt(token))) {
                this->state->chunkList.push_back(chunkToInt(token));
            }
            token = strtok(NULL, ",");
        }
        if ((bool)sctpMain->par("addIP")) {
            if (!asc && !typeInOwnChunkList(ASCONF))
                state->chunkList.push_back(ASCONF);
			if (!asca && !typeInOwnChunkList(ASCONF_ACK))
				state->chunkList.push_back(ASCONF_ACK);
        }
        free (chunkscopy);
    }

    assoc->state->active = false;
    assoc->state->fork = true;
    assoc->localAddr = localAddr;
    assoc->localPort = localPort;
    assoc->localAddressList = localAddressList;

    assoc->outboundStreams = outboundStreams;
    assoc->inboundStreams = inboundStreams;

    FSM_Goto((*assoc->fsm), SCTP_S_CLOSED);
    sctpMain->printInfoAssocMap();
    return assoc;
}

void SCTPAssociation::sendToIP(SCTPMessage*       sctpmsg,
                                         const IPvXAddress& dest)
{
    // Final touches on the segment before sending
    sctpmsg->setSrcPort(localPort);
    sctpmsg->setDestPort(remotePort);
    sctpmsg->setChecksumOk(true);
    sctpEV3<<"SendToIP: localPort="<<localPort<<" remotePort="<<remotePort<<" dest="<<dest<<"\n";
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

    if ((bool)sctpMain->par("udpEncapsEnabled"))
    {
        sctpMain->udpSocket.sendTo(sctpmsg, remoteAddr, SCTP_UDP_PORT);
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
            IPv4ControlInfo* controlInfo = new IPv4ControlInfo();
            controlInfo->setProtocol(IP_PROT_SCTP);
            controlInfo->setSrcAddr(IPv4Address("0.0.0.0"));
            controlInfo->setDestAddr(dest.get4());
            sctpmsg->setControlInfo(controlInfo);
            sctpMain->send(sctpmsg, "to_ip");
        }

        if (chunk->getChunkType() == HEARTBEAT) {
            SCTPPathVariables* path = getPath(dest);
            path->numberOfHeartbeatsSent++;
            path->vectorPathHb->record(path->numberOfHeartbeatsSent);
        }
        else if (chunk->getChunkType() == HEARTBEAT_ACK) {
            SCTPPathVariables* path = getPath(dest);
            path->numberOfHeartbeatAcksSent++;
            path->vectorPathHbAck->record(path->numberOfHeartbeatAcksSent);
        }
    }
    sctpEV3 << "Sent to " << dest << endl;
}


void SCTPAssociation::signalConnectionTimeout()
{
    sendIndicationToApp(SCTP_I_TIMED_OUT);
}

void SCTPAssociation::sendIndicationToApp(const int32 code, const int32 value)
{
    sctpEV3<<"sendIndicationToApp: " << indicationName(code) << endl;
    assert(code != SCTP_I_SENDQUEUE_ABATED);

    cPacket* msg = new cPacket(indicationName(code));
    msg->setKind(code);

    SCTPCommand* indication = new SCTPCommand(indicationName(code));
    indication->setAssocId(assocId);
    indication->setLocalAddr(localAddr);
    indication->setRemoteAddr(remoteAddr);
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

    char vectorName[128];
    for (uint16 i = 0; i < inboundStreams; i++) {
        snprintf(vectorName, sizeof(vectorName), "Stream %d Throughput", i);
        streamThroughputVectors[i] = new cOutVector(vectorName);
    }
}

void SCTPAssociation::sendToApp(cPacket *msg)
{
    sctpMain->send(msg, "to_appl", appGateIndex);
}

void SCTPAssociation::initAssociation(SCTPOpenCommand *openCmd)
{
    sctpEV3<<"SCTPAssociationUtil:initAssociation\n";
    // create send/receive queues
    const char *queueClass = openCmd->getQueueClass();
    transmissionQ = check_and_cast<SCTPQueue *>(createOne(queueClass));

    retransmissionQ = check_and_cast<SCTPQueue *>(createOne(queueClass));
    inboundStreams = openCmd->getInboundStreams();
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

    if ((bool)sctpMain->par("auth")) {
        const char* chunks = sctpMain->par("chunks").stringValue();
        bool asc = false;
        bool asca = false;
        char* chunkscopy = (char *)malloc(strlen(chunks)+1);
        strcpy(chunkscopy, chunks);
        char* token;
        token = strtok((char*)chunkscopy, ",");
        while (token != NULL)
        {
            if (chunkToInt(token) == ASCONF)
                asc = true;
            if (chunkToInt(token) == ASCONF_ACK)
                asca = true;
            this->state->chunkList.push_back(chunkToInt(token));
            token = strtok(NULL, ",");
        }
        if ((bool)sctpMain->par("addIP")) {
            if (!asc)
                state->chunkList.push_back(ASCONF);
			if (!asca)
				state->chunkList.push_back(ASCONF_ACK);
        }
        free (chunkscopy);
    }
}


void SCTPAssociation::sendInit()
{
    InterfaceTableAccess interfaceTableAccess;
    AddressVector adv;
    uint32 length = SCTP_INIT_CHUNK_LENGTH;

    if (remoteAddr.isUnspecified() || remotePort==0)
        throw cRuntimeError("Error processing command ASSOCIATE: foreign socket unspecified");

    if (localPort==0)
        throw cRuntimeError("Error processing command ASSOCIATE: local port unspecified");

    state->setPrimaryPath(getPath(remoteAddr));
    // create message consisting of INIT chunk
    SCTPMessage *sctpmsg = new SCTPMessage();
    sctpmsg->setBitLength(SCTP_COMMON_HEADER*8);
    SCTPInitChunk *initChunk = new SCTPInitChunk("INIT");
    initChunk->setChunkType(INIT);
    initChunk->setInitTag((uint32)(fmod(intrand(INT32_MAX), 1.0+(double)(unsigned)0xffffffffUL)) & 0xffffffffUL);

    peerVTag = initChunk->getInitTag();
    sctpEV3<<"INIT from "<<localAddr<<":InitTag="<<peerVTag<<"\n";
    initChunk->setA_rwnd(sctpMain->par("arwnd"));
    state->localRwnd = (long)sctpMain->par("arwnd");
    initChunk->setNoOutStreams(outboundStreams);
    initChunk->setNoInStreams(inboundStreams);
    initChunk->setInitTSN(1000);
    initChunk->setMsg_rwnd(sctpMain->par("messageAcceptLimit"));
    state->nextTSN = initChunk->getInitTSN();
    state->lastTSN = initChunk->getInitTSN() + state->numRequests - 1;
    state->streamResetSequenceNumber = state->nextTSN;
    state->asconfSn = 1000;

    initTsn = initChunk->getInitTSN();
    IInterfaceTable *ift = interfaceTableAccess.get();
    sctpEV3<<"add local address\n";
    if (localAddressList.front() == IPvXAddress("0.0.0.0"))
    {
        for (int32 i=0; i<ift->getNumInterfaces(); ++i)
        {
#ifdef WITH_IPv4
            if (ift->getInterface(i)->ipv4Data()!=NULL)
            {
                adv.push_back(ift->getInterface(i)->ipv4Data()->getIPAddress());
            }
            else
#endif
#ifdef WITH_IPv6
            if (ift->getInterface(i)->ipv6Data()!=NULL)
            {
                for (int32 j=0; j<ift->getInterface(i)->ipv6Data()->getNumAddresses(); j++)
                {
                    sctpEV3<<"add address "<<ift->getInterface(i)->ipv6Data()->getAddress(j)<<"\n";
                    adv.push_back(ift->getInterface(i)->ipv6Data()->getAddress(j));
                }
            }
            else
#endif
            ;
        }
    }
    else
    {
        adv = localAddressList;
        sctpEV3<<"gebundene Adresse "<<localAddr<<" wird hinzugefuegt\n";
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
        int rlevel = getAddressLevel(remoteAddr);
        sctpEV3<<"level of remote address="<<rlevel<<"\n";
        for (AddressVector::iterator i=adv.begin(); i!=adv.end(); ++i)
        {
            int addressLevel = getAddressLevel(*i);
            sctpEV3<<"level of address "<<(*i)<<" = "<<addressLevel<<"\n";
            if (addressLevel>=rlevel)
            {
                initChunk->setAddressesArraySize(addrNum+1);
                initChunk->setAddresses(addrNum++, (*i));
                length += 8;
                sctpMain->addLocalAddress(this, (*i));
                state->localAddresses.push_back((*i));
                if (localAddr.get4().getInt()==0)
                    localAddr = (*i);
            }
            else if (rlevel==4 && addressLevel==3 && friendly)
            {
                sctpMain->addLocalAddress(this, (*i));
                state->localAddresses.push_back((*i));
                if (localAddr.get4().getInt()==0)
                    localAddr = (*i);
            }
        }
    }

    uint16 count = 0;
    if (sctpMain->auth==true)
    {
        initChunk->setSepChunksArraySize(++count);
        initChunk->setSepChunks(count-1, AUTH);
        state->keyVector[0] = (uint8_t)RANDOM;
        state->keyVector[2] = 36;
        for (int32 k=0; k<32; k++)
        {
            initChunk->setRandomArraySize(k+1);
            initChunk->setRandom(k, (uint8)(intrand(256)));
            state->keyVector[k+2] = initChunk->getRandom(k);
        }
        state->sizeKeyVector = 36;
        state->keyVector[state->sizeKeyVector] = (uint8_t)CHUNKS;
        state->sizeKeyVector += 2;
        state->keyVector[state->sizeKeyVector ] = state->chunkList.size()+4;
        state->sizeKeyVector += 2;
        initChunk->setChunkTypesArraySize(state->chunkList.size());
        int32 k = 0;
        for (std::vector<uint16>::iterator it=state->chunkList.begin(); it!=state->chunkList.end(); it++)
        {
            initChunk->setChunkTypes(k, (*it));
            state->keyVector[state->sizeKeyVector] = (*it);
            state->sizeKeyVector ++;
            k++;
        }
        state->keyVector[state->sizeKeyVector] = (uint8_t)HMAC_ALGO;
        state->sizeKeyVector += 2;
        state->keyVector[state->sizeKeyVector] = 1+4;
        state->sizeKeyVector += 2;
        state->keyVector[state->sizeKeyVector] = 1;
        state->sizeKeyVector ++;
        initChunk->setHmacTypesArraySize(1);
        initChunk->setHmacTypes(0, 1);
        length += initChunk->getChunkTypesArraySize()+46;
    }

    if (sctpMain->pktdrop) {
        initChunk->setSepChunksArraySize(++count);
        initChunk->setSepChunks(count-1, PKTDROP);
    }
    if (state->streamReset == true) {
        initChunk->setSepChunksArraySize(++count);
        initChunk->setSepChunks(count-1, STREAM_RESET);
    }
    if ((bool)sctpMain->par("addIP") == true) {
        initChunk->setSepChunksArraySize(++count);
        initChunk->setSepChunks(count-1, ASCONF);
        initChunk->setSepChunksArraySize(++count);
        initChunk->setSepChunks(count-1, ASCONF_ACK);
    }
    if (state->prMethod != 0) {
        initChunk->setForwardTsn(true);
    }

    sctpMain->printInfoAssocMap();
    initChunk->setBitLength(length*8);
    sctpmsg->addChunk(initChunk);
    // set path variables
    if (remoteAddressList.size()>0)
    {
        for (AddressVector::iterator it=remoteAddressList.begin(); it!=remoteAddressList.end(); it++)
        {
            sctpEV3<<__LINE__<<" get new path for "<<(*it)<<"\n";
            SCTPPathVariables* path = new SCTPPathVariables((*it), this);
            sctpPathMap[(*it)] = path;
            qCounter.roomTransQ[(*it)] = 0;
            qCounter.bookedTransQ[(*it)] = 0;
            qCounter.roomRetransQ[(*it)] = 0;
        }
    }
    else
    {
        sctpEV3<<__LINE__<<" get new path for "<<remoteAddr<<"\n";
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
    sctpEV3<<getFullPath()<<" sendInit: localVTag="<<localVTag<<" peerVTag="<<peerVTag<<"\n";
    sendToIP(sctpmsg);
    sctpMain->assocList.push_back(this);
}

void SCTPAssociation::retransmitInit()
{
    SCTPMessage *sctpmsg = new SCTPMessage();
    sctpmsg->setBitLength(SCTP_COMMON_HEADER*8);
    SCTPInitChunk *sctpinit; // = new SCTPInitChunk("INIT");

    sctpEV3<<"Retransmit InitChunk="<<&sctpinit<<"\n";

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
    sctpEV3<<"sendInitAck at "<<localAddr<<". Provided InitTag="<<initChunk->getInitTag()<<"\n";
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
        state->asconfSn = 2000;
        state->streamResetSequenceNumber = state->nextTSN;
        cookie->setLocalTag(localVTag);
        cookie->setPeerTag(peerVTag);
        for (int32 i=0; i<32; i++)
        {
            cookie->setLocalTieTag(i, 0);
            cookie->setPeerTieTag(i, 0);
        }
        sctpinitack->setTag(localVTag);
        sctpEV3<<"state=closed: localVTag="<<localVTag<<" peerVTag="<<peerVTag<<"\n";
    }
    else if (fsm->getState()==SCTP_S_COOKIE_WAIT || fsm->getState()==SCTP_S_COOKIE_ECHOED)
    {
        initAckChunk->setInitTag(peerVTag);
        sctpEV3<<"different state:set InitTag in InitAck: "<<initAckChunk->getInitTag()<<"\n";
        initAckChunk->setInitTSN(state->nextTSN);
        initPeerTsn = initChunk->getInitTSN();
        state->gapList.forwardCumAckTSN(initPeerTsn - 1);
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
        sctpEV3<<"VTag in InitAck: "<<sctpinitack->getTag()<<"\n";
    }
    else
    {
        sctpEV3<<"other state\n";
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
    initAckChunk->setMsg_rwnd(sctpMain->par("messageAcceptLimit"));
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

    uint16 count = 0;
    if (sctpMain->auth==true)
    {
        initAckChunk->setSepChunksArraySize(++count);
        initAckChunk->setSepChunks(count-1, AUTH);
        for (int32 k=0; k<32; k++)
        {
            initAckChunk->setRandomArraySize(k+1);
            initAckChunk->setRandom(k, (uint8)(intrand(256)));
        }
        initAckChunk->setChunkTypesArraySize(state->chunkList.size());
        int32 k = 0;
        for (std::vector<uint16>::iterator it=state->chunkList.begin(); it!=state->chunkList.end(); it++)
        {
            initAckChunk->setChunkTypes(k, (*it));
            k++;
        }
        initAckChunk->setHmacTypesArraySize(1);
        initAckChunk->setHmacTypes(0, 1);
        length += initAckChunk->getChunkTypesArraySize()+46;

    }
    uint32 unknownLen = initChunk->getUnrecognizedParametersArraySize();
    if (unknownLen>0)
    {
        sctpEV3<<"Found unrecognized Parameters in INIT chunk with a length of "<<unknownLen<<" bytes.\n";
        initAckChunk->setUnrecognizedParametersArraySize(unknownLen);
        for (uint32 i=0; i<unknownLen; i++)
            initAckChunk->setUnrecognizedParameters(i, initChunk->getUnrecognizedParameters(i));
        length += unknownLen;
    }
    else
        initAckChunk->setUnrecognizedParametersArraySize(0);

    if (sctpMain->pktdrop)
    {
        initAckChunk->setSepChunksArraySize(++count);
        initAckChunk->setSepChunks(count-1, PKTDROP);
    }

    if (state->streamReset == true)
    {
        initAckChunk->setSepChunksArraySize(++count);
        initAckChunk->setSepChunks(count-1, STREAM_RESET);
    }
    if ((bool)sctpMain->par("addIP") == true)
    {
        initAckChunk->setSepChunksArraySize(++count);
        initAckChunk->setSepChunks(count-1, ASCONF);
        initAckChunk->setSepChunksArraySize(++count);
        initAckChunk->setSepChunks(count-1, ASCONF_ACK);
    }
    if (state->prMethod != 0)
    {
        initAckChunk->setForwardTsn(true);
    }
    length += count;

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
    SCTPAuthenticationChunk* authChunk;
    SCTPMessage *sctpcookieecho = new SCTPMessage();
    sctpcookieecho->setBitLength(SCTP_COMMON_HEADER*8);

    sctpEV3<<"SCTPAssociationUtil:sendCookieEcho\n";

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
        sctpEV3<<"Found unrecognized Parameters in INIT-ACK chunk with a length of "<<unknownLen<<" bytes.\n";
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

    if (state->auth && state->peerAuth && typeInChunkList(COOKIE_ECHO))
    {
        authChunk = createAuthChunk();
        sctpcookieecho->addChunk(authChunk);
        SCTP::AssocStatMap::iterator it = sctpMain->assocStatMap.find(assocId);
        it->second.numAuthChunksSent++;
    }

    sctpcookieecho->addChunk(cookieEchoChunk);
    sendToIP(sctpcookieecho);
}


void SCTPAssociation::retransmitCookieEcho()
{
    SCTPAuthenticationChunk* authChunk;
    SCTPMessage*                 sctpmsg = new SCTPMessage();
    sctpmsg->setBitLength(SCTP_COMMON_HEADER*8);
    SCTPCookieEchoChunk* cookieEchoChunk = check_and_cast<SCTPCookieEchoChunk*>(state->cookieChunk->dup());
    if (cookieEchoChunk->getCookieArraySize()==0)
    {
        cookieEchoChunk->setStateCookie(state->cookieChunk->getStateCookie()->dup());
    }
    if (state->auth && state->peerAuth && typeInChunkList(COOKIE_ECHO))
    {
        authChunk = createAuthChunk();
        sctpmsg->addChunk(authChunk);
        SCTP::AssocStatMap::iterator it = sctpMain->assocStatMap.find(assocId);
        it->second.numAuthChunksSent++;
    }
    sctpmsg->addChunk(cookieEchoChunk);

    sctpEV3<<"retransmitCookieEcho localAddr="<<localAddr<<"     remoteAddr"<<remoteAddr<<"\n";

    sendToIP(sctpmsg);
}

void SCTPAssociation::sendHeartbeat(const SCTPPathVariables* path)
{
    SCTPAuthenticationChunk* authChunk;
    SCTPMessage* sctpHeartbeatbeat = new SCTPMessage();
    sctpHeartbeatbeat->setBitLength(SCTP_COMMON_HEADER*8);

    sctpHeartbeatbeat->setSrcPort(localPort);
    sctpHeartbeatbeat->setDestPort(remotePort);
    SCTPHeartbeatChunk* heartbeatChunk = new SCTPHeartbeatChunk("HEARTBEAT");
    heartbeatChunk->setChunkType(HEARTBEAT);
    heartbeatChunk->setRemoteAddr(path->remoteAddress);
    heartbeatChunk->setTimeField(simTime());
    heartbeatChunk->setBitLength((SCTP_HEARTBEAT_CHUNK_LENGTH+12)*8);
    if (state->auth && state->peerAuth && typeInChunkList(HEARTBEAT)) {
        authChunk = createAuthChunk();
        sctpHeartbeatbeat->addChunk(authChunk);
        SCTP::AssocStatMap::iterator it = sctpMain->assocStatMap.find(assocId);
        it->second.numAuthChunksSent++;
    }
    sctpHeartbeatbeat->addChunk(heartbeatChunk);
    sctpEV3 << "sendHeartbeat: sendToIP to " << path->remoteAddress << endl;
    sendToIP(sctpHeartbeatbeat, path->remoteAddress);
}

void SCTPAssociation::sendHeartbeatAck(const SCTPHeartbeatChunk* heartbeatChunk,
                                                    const IPvXAddress&        src,
                                                    const IPvXAddress&        dest)
{
    SCTPAuthenticationChunk* authChunk;
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

    heartbeatAckChunk->setByteLength(heartbeatChunk->getByteLength());
    if (state->auth && state->peerAuth && typeInChunkList(HEARTBEAT_ACK)) {
        authChunk = createAuthChunk();
        sctpHeartbeatAck->addChunk(authChunk);
        SCTP::AssocStatMap::iterator it = sctpMain->assocStatMap.find(assocId);
        it->second.numAuthChunksSent++;
    }
    sctpHeartbeatAck->addChunk(heartbeatAckChunk);

    sctpEV3 << "sendHeartbeatAck: sendToIP from " << src << " to " << dest << endl;
    sendToIP(sctpHeartbeatAck, dest);
}

void SCTPAssociation::sendCookieAck(const IPvXAddress& dest)
{
    SCTPAuthenticationChunk* authChunk;
    SCTPMessage *sctpcookieack = new SCTPMessage();
    sctpcookieack->setBitLength(SCTP_COMMON_HEADER*8);

    sctpEV3<<"SCTPAssociationUtil:sendCookieACK\n";

    sctpcookieack->setSrcPort(localPort);
    sctpcookieack->setDestPort(remotePort);
    SCTPCookieAckChunk* cookieAckChunk = new SCTPCookieAckChunk("COOKIE_ACK");
    cookieAckChunk->setChunkType(COOKIE_ACK);
    cookieAckChunk->setBitLength(SCTP_COOKIE_ACK_LENGTH*8);
    if (state->auth && state->peerAuth && typeInChunkList(COOKIE_ACK))
    {
        authChunk = createAuthChunk();
        sctpcookieack->addChunk(authChunk);
        SCTP::AssocStatMap::iterator it = sctpMain->assocStatMap.find(assocId);
        it->second.numAuthChunksSent++;
    }
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

    sctpEV3<<"SCTPAssociationUtil:sendShutdownComplete\n";

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
    SCTPAuthenticationChunk* authChunk;
    SCTPMessage *msg = new SCTPMessage();
    msg->setBitLength(SCTP_COMMON_HEADER*8);

    sctpEV3<<"SCTPAssociationUtil:sendABORT localPort="<<localPort<<"    remotePort="<<remotePort<<"\n";

    msg->setSrcPort(localPort);
    msg->setDestPort(remotePort);
    SCTPAbortChunk* abortChunk = new SCTPAbortChunk("ABORT");
    abortChunk->setChunkType(ABORT);
    abortChunk->setT_Bit(0);
    abortChunk->setBitLength(SCTP_ABORT_CHUNK_LENGTH*8);
    if (state->auth && state->peerAuth && typeInChunkList(ABORT))
    {
        authChunk = createAuthChunk();
        msg->addChunk(authChunk);
        SCTP::AssocStatMap::iterator it = sctpMain->assocStatMap.find(assocId);
        it->second.numAuthChunksSent++;
    }
    msg->addChunk(abortChunk);
    sendToIP(msg, remoteAddr);
}

void SCTPAssociation::sendShutdown()
{
    SCTPAuthenticationChunk* authChunk;
    SCTPMessage *msg = new SCTPMessage();
    msg->setBitLength(SCTP_COMMON_HEADER*8);

    sctpEV3<<"SCTPAssociationUtil:sendShutdown localPort="<<localPort<<"     remotePort="<<remotePort<<"\n";

    msg->setSrcPort(localPort);
    msg->setDestPort(remotePort);
    SCTPShutdownChunk* shutdownChunk = new SCTPShutdownChunk("SHUTDOWN");
    shutdownChunk->setChunkType(SHUTDOWN);
    //shutdownChunk->setCumTsnAck(state->lastTsnAck);
    shutdownChunk->setCumTsnAck(state->gapList.getCumAckTSN());
    shutdownChunk->setBitLength(SCTP_SHUTDOWN_CHUNK_LENGTH*8);
    if (state->auth && state->peerAuth && typeInChunkList(SHUTDOWN))
    {
        authChunk = createAuthChunk();
        msg->addChunk(authChunk);
        SCTP::AssocStatMap::iterator it = sctpMain->assocStatMap.find(assocId);
        it->second.numAuthChunksSent++;
    }
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

    sctpEV3<<"retransmitShutdown localAddr="<<localAddr<<"  remoteAddr"<<remoteAddr<<"\n";

    sendToIP(sctpmsg);
}

void SCTPAssociation::retransmitShutdownAck()
{
    SCTPMessage *sctpmsg = new SCTPMessage();
    sctpmsg->setBitLength(SCTP_COMMON_HEADER*8);
    SCTPShutdownAckChunk* shutdownAckChunk;
    shutdownAckChunk = check_and_cast<SCTPShutdownAckChunk*>(state->shutdownAckChunk->dup());
    sctpmsg->addChunk(shutdownAckChunk);

    sctpEV3<<"retransmitShutdownAck localAddr="<<localAddr<<"  remoteAddr"<<remoteAddr<<"\n";

    sendToIP(sctpmsg);
}

void SCTPAssociation::sendPacketDrop(const bool flag)
{
    sctpEV3 << "sendPacketDrop:\t";
    SCTPMessage* drop = (SCTPMessage *)state->sctpmsg->dup();
    if (drop->getChunksArraySize()==1)
    {
        SCTPChunk* header = (SCTPChunk*)(drop->getChunks(0));
        if (header->getChunkType()==PKTDROP)
        {
            disposeOf(state->sctpmsg);
            delete drop;
            return;
        }
    }
    SCTPMessage *sctpmsg = new SCTPMessage();
    sctpmsg->setBitLength(SCTP_COMMON_HEADER*8);
    SCTPPacketDropChunk* pktdrop = new SCTPPacketDropChunk("PKTDROP");
    pktdrop->setChunkType(PKTDROP);
    pktdrop->setCFlag(false);
    pktdrop->setTFlag(false);
    pktdrop->setBFlag(flag);
    pktdrop->setMFlag(false);
    pktdrop->setMaxRwnd(sctpMain->par("arwnd"));
    pktdrop->setQueuedData(state->queuedReceivedBytes);
    pktdrop->setTruncLength(0);
    pktdrop->setByteLength(SCTP_PKTDROP_CHUNK_LENGTH);
    uint16 mss = getPath(remoteAddr)->pmtu - SCTP_COMMON_HEADER - SCTP_DATA_CHUNK_LENGTH - IP_HEADER_LENGTH;
    if (drop->getByteLength()>mss)
    {
        uint16 diff = drop->getByteLength()-mss;
        pktdrop->setTruncLength(drop->getByteLength());
        SCTPChunk* chunk = (SCTPChunk*)(drop->removeLastChunk());
        if (chunk->getChunkType()==DATA)
        {
            SCTPDataChunk* dataChunk = check_and_cast<SCTPDataChunk*>(chunk);
            SCTPSimpleMessage* smsg = check_and_cast <SCTPSimpleMessage*>(dataChunk->decapsulate());
            if (smsg->getDataLen() > diff)
            {
                uint16 newLength = smsg->getDataLen()-diff;
                smsg->setDataArraySize(newLength);
                for (uint16 i=0; i<newLength; i++)
                    smsg->setData(i, 'a');
                smsg->setDataLen(newLength);
                smsg->setEncaps(false);
                smsg->setByteLength(newLength);
                dataChunk->encapsulate(smsg);
                drop->addChunk(dataChunk);
            }
            else if (drop->getChunksArraySize()==1)
            {
                delete chunk;
                delete pktdrop;
                disposeOf(state->sctpmsg);
                disposeOf(drop);
                sctpEV3 << "laenge=" << drop->getByteLength() << " numberOfChunks=1\n";
                return;
            }
        }
        else
        {
            delete pktdrop;
            disposeOf(state->sctpmsg);
            disposeOf(drop);
            sctpEV3 << "laenge=" << drop->getByteLength() << " numberOfChunks=1\n";
            return;
        }
        pktdrop->setTFlag(true);
    }
    pktdrop->encapsulate(drop);
    sctpEV3 << "length of PKTDROP chunk=" << pktdrop->getBitLength()/8 << "\n";
    sctpmsg->addChunk(pktdrop);
    sctpEV3 << "total length now " << sctpmsg->getByteLength() << "\n";
    disposeOf(state->sctpmsg);
    state->pktDropSent = true;
    sctpMain->numPktDropReports++;
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

    sctpEV3<<"scheduleSack() : ackState is now: "<<state->ackState<<"\n";

    if (state->ackState <= sackFrequency - 1)
    {
        /* start a SACK timer if none is running, to expire 200 ms (or parameter) from now */
        if (!SackTimer->isScheduled())
        {
             startTimer(SackTimer, sackPeriod);
        }
        /* else: leave timer running, and do nothing... */ else {
            /* is this possible at all ? Check this... */

            sctpEV3<<"SACK timer running, but scheduleSack() called\n";

        }
    }
}

SCTPForwardTsnChunk* SCTPAssociation::createForwardTsnChunk(const IPvXAddress& pid)
{
    uint16 chunkLength = SCTP_FORWARD_TSN_CHUNK_LENGTH;
    SCTPDataVariables* chunk;
    typedef std::map<uint16,int16> SidMap;
    SidMap sidMap;

    sctpEV3 << "Create forwardTsnChunk for " << pid << "\n";
    SCTPForwardTsnChunk* forwChunk = new SCTPForwardTsnChunk("FORWARD_TSN");
    forwChunk->setChunkType(FORWARD_TSN);
    advancePeerTsn();
    forwChunk->setNewCumTsn(state->advancedPeerAckPoint);
    for (SCTPQueue::PayloadQueue::iterator it=retransmissionQ->payloadQueue.begin(); it!=retransmissionQ->payloadQueue.end(); it++)
    {
        chunk = it->second;
        sctpEV3 << "tsn=" << chunk->tsn << " lastDestination=" << chunk->getLastDestination() << " abandoned=" << chunk->hasBeenAbandoned << "\n";
        if (chunk->getLastDestination() == pid && chunk->hasBeenAbandoned && chunk->tsn <= forwChunk->getNewCumTsn())
        {
            if (chunk->ordered)
            {
                sidMap[chunk->sid] = chunk->ssn;
            }
            else
            {
                sidMap[chunk->sid] = -1;
            }
            /* Fake chunk retransmission */
            if (chunk->sendForwardIfAbandoned) {
                chunk->gapReports = 0;
                chunk->hasBeenFastRetransmitted = false;
                chunk->sendTime = simTime();
                chunk->numberOfRetransmissions++;
                chunk->sendForwardIfAbandoned = false;

                SCTPQueue::PayloadQueue::iterator itt = transmissionQ->payloadQueue.find(chunk->tsn);
                if (itt != transmissionQ->payloadQueue.end()) {
                    transmissionQ->payloadQueue.erase(itt);
                    chunk->enqueuedInTransmissionQ = false;
                    CounterMap::iterator i = qCounter.roomTransQ.find(pid);
                    i->second -= ADD_PADDING(chunk->len/8+SCTP_DATA_CHUNK_LENGTH);
                    CounterMap::iterator ib = qCounter.bookedTransQ.find(pid);
                    ib->second -= chunk->booksize;
                }
            }
        }
    }
    forwChunk->setSidArraySize(sidMap.size());
    forwChunk->setSsnArraySize(sidMap.size());
    int32 i = 0;
    for (SidMap::iterator j=sidMap.begin(); j!=sidMap.end(); j++)
    {
        forwChunk->setSid(i, j->first);
        forwChunk->setSsn(i, j->second);
        chunkLength += 4;
        i++;
    }
    forwChunk->setByteLength(chunkLength);
    SCTP::AssocStatMap::iterator iter = sctpMain->assocStatMap.find(assocId);
    iter->second.numForwardTsn++;
    return forwChunk;
}

static uint32 copyToRGaps(SCTPSackChunk*         sackChunk,
        const SCTPGapList*         gapList,
        const SCTPGapList::GapType type,
        size_t&                space)
{
    const uint32 count = gapList->getNumGaps(type);
    uint32       last = gapList->getCumAckTSN();
    uint32       keys = min(space / 4, count);   // Each entry occupies 2+2 bytes => at most space/4 entries

    sackChunk->setGapStartArraySize(keys);
    sackChunk->setGapStopArraySize(keys);
    sackChunk->setNumGaps(keys);

    for (uint32 key = 0; key < keys; key++) {
        // ====== Validity check ========================================
        assert(SCTPAssociation::tsnGt(gapList->getGapStart(type, key), last + 1));
        assert(SCTPAssociation::tsnGe(gapList->getGapStop(type, key), gapList->getGapStart(type, key)));
        sackChunk->setGapStart(key, gapList->getGapStart(type, key));
        sackChunk->setGapStop(key, gapList->getGapStop(type, key));
        last = gapList->getGapStop(type, key);
    }
    space = 4 * keys;

    return (keys);
}

static uint32 copyToNRGaps(SCTPSackChunk*         sackChunk,
        const SCTPGapList*         gapList,
        const SCTPGapList::GapType type,
        size_t&                space)
{
    const uint32 count = gapList->getNumGaps(type);
    uint32       last = gapList->getCumAckTSN();
    uint32       keys = min(space / 4, count);   // Each entry occupies 2+2 bytes => at most space/4 entries

    sackChunk->setNrGapStartArraySize(keys);
    sackChunk->setNrGapStopArraySize(keys);
    sackChunk->setNumNrGaps(keys);

    for (uint32 key = 0; key < keys; key++) {
        // ====== Validity check ========================================
        assert(SCTPAssociation::tsnGt(gapList->getGapStart(type, key), last + 1));
        assert(SCTPAssociation::tsnGe(gapList->getGapStop(type, key), gapList->getGapStart(type, key)));
        sackChunk->setNrGapStart(key, gapList->getGapStart(type, key));
        sackChunk->setNrGapStop(key, gapList->getGapStop(type, key));
        last = gapList->getGapStop(type, key);
    }
    space = 4 * keys;

    return (keys);
}


SCTPSackChunk* SCTPAssociation::createSack()
{
    sctpEV3<<simTime()<<"SCTPAssociationUtil:createSACK localAddress="<<localAddr<<"  remoteAddress="<<remoteAddr<<"\n";

    sctpEV3<<" localRwnd="<<state->localRwnd<<" queuedBytes="<<state->queuedReceivedBytes<<"\n";

    // ====== Get receiver window size to be advertised ======================
    uint32 arwnd = 0;
    uint32 msgRwnd = 0;
    calculateRcvBuffer();
    if ((state->messageAcceptLimit>0 && (int32)(state->localMsgRwnd - state->bufferedMessages) <= 0)
            || (state->messageAcceptLimit==0 && (int32)(state->localRwnd - state->queuedReceivedBytes - state->bufferedMessages*state->bytesToAddPerRcvdChunk) <= 0))
    {
        msgRwnd = 0;
    }
    else if ((state->messageAcceptLimit>0 && (int32)(state->localMsgRwnd - state->bufferedMessages) < 3)
            || (state->messageAcceptLimit==0 && state->localRwnd - state->queuedReceivedBytes - state->bufferedMessages*state->bytesToAddPerRcvdChunk < state->swsLimit) || state->swsMsgInvoked == true)
    {
        msgRwnd = 1;
        state->swsMsgInvoked = true;
    }
    else {
        if (state->messageAcceptLimit > 0) {
            msgRwnd = state->localMsgRwnd - state->bufferedMessages;
        }
        else {
            msgRwnd = state->localRwnd -
                    state->queuedReceivedBytes -
                    state->bufferedMessages*state->bytesToAddPerRcvdChunk;
        }
    }
    if (state->tellArwnd) {
        arwnd = msgRwnd;
    }
    else {
        // ====== Receiver buffer is full =====================================
        if ((int32)(state->localRwnd - state->queuedReceivedBytes) <= 0) {
            arwnd = 0;
            if (state->swsLimit > 0) {
                state->swsAvoidanceInvoked = true;
            }
        }
        // ====== Silly window syndrome avoidance =============================
        else if ((state->localRwnd - state->queuedReceivedBytes < state->swsLimit) ||
                (state->swsAvoidanceInvoked == true))
        {
            arwnd = 1;
            if (state->swsLimit > 0)
                state->swsAvoidanceInvoked = true;
            sctpEV3<<"arwnd=1; createSack : SWS Avoidance ACTIVE !!!\n";
        }
        // ====== There is space in the receiver buffer =======================
        else
        {
            arwnd = state->localRwnd - state->queuedReceivedBytes;
            sctpEV3<<simTime()<<" arwnd = "<<state->localRwnd<<" - "<<state->queuedReceivedBytes<<" = "<<arwnd<<"\n";
        }
    }


    // ====== Record statistics ==============================================
    if (state->messageAcceptLimit > 0) {
        advMsgRwnd->record(msgRwnd);
    }
    statisticsQueuedReceivedBytes->record(state->queuedReceivedBytes);
    advRwnd->record(arwnd);

    // ====== Create SACK chunk ==============================================
    SCTPSackChunk* sackChunk = new SCTPSackChunk("SACK");
    if (state->nrSack == true) {
        sackChunk->setChunkType(NR_SACK);
        sackChunk->setName("NR_SACK");
    } else {
        sackChunk->setChunkType(SACK);
    }
    sackChunk->setCumTsnAck(state->gapList.getCumAckTSN());
    sackChunk->setA_rwnd(arwnd);
    sackChunk->setIsNrSack(state->nrSack);
    sackChunk->setSackSeqNum(++state->outgoingSackSeqNum);
    if (state->messageAcceptLimit > 0) {
        sackChunk->setMsg_rwnd(state->messageAcceptLimit-state->bufferedMessages);
    }
    else {
        sackChunk->setMsg_rwnd(0);
    }


    // ====== What has to be stored in the SACK? =============================
    const uint32 mtu = getPath(remoteAddr)->pmtu;
    const uint32 allowedLength = mtu -
            ((remoteAddr.isIPv6()) ? 40 : 20) -
            SCTP_COMMON_HEADER -
            SCTP_SACK_CHUNK_LENGTH;
    uint32 numDups = state->dupList.size();
    uint32 numRevokableGaps = state->gapList.getNumGaps(SCTPGapList::GT_Revokable);
    uint32 numNonRevokableGaps = state->gapList.getNumGaps(SCTPGapList::GT_NonRevokable);
    size_t revokableGapsSpace = ~0;
    size_t nonRevokableGapsSpace = ~0;
    size_t sackHeaderLength = ~0;
    const uint32 totalGaps = state->gapList.getNumGaps(SCTPGapList::GT_Any);

    // ====== Record statistics ==============================================
    statisticsNumTotalGapBlocksStored->record(totalGaps);
    statisticsNumRevokableGapBlocksStored->record(numRevokableGaps);
    statisticsNumNonRevokableGapBlocksStored->record(numNonRevokableGaps);
    statisticsNumDuplicatesStored->record(numDups);

	// ------ Regular NR-SACK ---------------------------
	if (state->nrSack == true) {
	   sackHeaderLength = SCTP_NRSACK_CHUNK_LENGTH;

	   numRevokableGaps = copyToRGaps(sackChunk,  &state->gapList, SCTPGapList::GT_Revokable, revokableGapsSpace);    // Add R-acks only
	   numNonRevokableGaps = copyToNRGaps(sackChunk, &state->gapList, SCTPGapList::GT_NonRevokable, nonRevokableGapsSpace); // Add NR-acks only
	}
	// ------ Regular SACK ------------------------------
	else {
	   sackHeaderLength = SCTP_SACK_CHUNK_LENGTH;
	   numRevokableGaps = copyToRGaps(sackChunk, &state->gapList,  SCTPGapList::GT_Any, revokableGapsSpace);            // Add ALL
	   numNonRevokableGaps = 0;
	   nonRevokableGapsSpace = 0;
	}


    // ====== SACK has to be shorted to fit in MTU ===========================
    uint32 sackLength = sackHeaderLength + revokableGapsSpace + nonRevokableGapsSpace + numDups*4;
    if (sackLength > allowedLength) {
        // Strategy to reduce the SACK size:
        // - Report no duplicates (they are not used for congestion control)
        // - Split the remaining space equally between
        //   revokable and non-revokable GapAcks

        // ====== Drop duplicates list ========================================
        numDups = 0;
        sackLength -= 4 * numDups;

        if (sackLength > allowedLength) {
            // Unfortunately, dropping the duplicates has not solved the problem.
            //    => Now, the gap lists have to be shortened!

            SCTP::AssocStatMap::iterator iter = sctpMain->assocStatMap.find(assocId);
            iter->second.numOverfullSACKs++;

            revokableGapsSpace = allowedLength - sackHeaderLength;
            if (totalGaps < (state->gapList.getNumGaps(SCTPGapList::GT_Revokable))) {
                numRevokableGaps = copyToRGaps(sackChunk,  &state->gapList, SCTPGapList::GT_Any, revokableGapsSpace);    // Add ALL
            }
            else {
                numRevokableGaps = copyToRGaps(sackChunk,  &state->gapList, SCTPGapList::GT_Revokable, revokableGapsSpace);    // Add R-acks only
            }
            sackLength = sackHeaderLength + revokableGapsSpace + nonRevokableGapsSpace + numDups*4;

            // ====== Shorten gap lists ========================================

			if (sackLength > allowedLength) {
			   const uint32 blocksBeRemoved = (sackLength - allowedLength) / 4;
			   const double revokableFraction = numRevokableGaps / (double)(numRevokableGaps + numNonRevokableGaps);

			   const uint32 removeRevokable = (uint32)ceil(blocksBeRemoved * revokableFraction);
			   const uint32 removeNonRevokable = (uint32)ceil(blocksBeRemoved * (1.0 - revokableFraction));
			   numRevokableGaps -= std::min(removeRevokable, numRevokableGaps);
			   numNonRevokableGaps -= std::min(removeNonRevokable, numNonRevokableGaps);
			   revokableGapsSpace = 4 * numRevokableGaps;
			   nonRevokableGapsSpace = 4 * numNonRevokableGaps;
			   numRevokableGaps = copyToRGaps(sackChunk,  &state->gapList, SCTPGapList::GT_Revokable, revokableGapsSpace);    // Add R-acks only
			   numNonRevokableGaps = copyToNRGaps(sackChunk, &state->gapList, SCTPGapList::GT_NonRevokable, nonRevokableGapsSpace); // Add NR-acks only
			   sackLength = sackHeaderLength + revokableGapsSpace + nonRevokableGapsSpace + numDups*4;
			}

            assert(sackLength <= allowedLength);

            // Update values in SACK chunk ...
            sackChunk->setNumGaps(numRevokableGaps);
            sackChunk->setNumNrGaps(numNonRevokableGaps);
        }
    }
    sackChunk->setNumDupTsns(numDups);
    sackChunk->setBitLength(sackLength * 8);

    // ====== Add duplicates =================================================
    if (numDups > 0) {
        sackChunk->setDupTsnsArraySize(numDups);
        uint32 key = 0;
        for (std::list<uint32>::iterator iterator = state->dupList.begin();
                iterator != state->dupList.end(); iterator++) {
            sackChunk->setDupTsns(key, *iterator);
            key++;
            if (key == numDups)
                break;
        }
        state->dupList.clear();
    }

    // ====== Record statistics ==============================================
    statisticsSACKLengthSent->record(sackLength);
    statisticsNumRevokableGapBlocksSent->record(numRevokableGaps);
    statisticsNumNonRevokableGapBlocksSent->record(numNonRevokableGaps);
    statisticsNumDuplicatesSent->record(numDups);

    // ====== Print information ==============================================
    if (SCTP::testing == true) {
        sctpEV3 << "createSack:"
                << " bufferedMessages=" << state->bufferedMessages
                << " msgRwnd=" << msgRwnd
                << " arwnd=" << sackChunk->getA_rwnd()
                << " cumAck=" << state->gapList.getCumAckTSN()
                << " numRGaps=" << numRevokableGaps
                << " numNRGaps=" << numNonRevokableGaps
                << " numDups=" << numDups
                << endl;
        state->gapList.print(sctpEV3);
    }
    return sackChunk;
}

void SCTPAssociation::sendSack()
{
    SCTPAuthenticationChunk* authChunk;
    SCTPSackChunk*           sackChunk;

    sctpEV3 << "Sending SACK" << endl;

    /* sack timer has expired, reset flag, and send SACK */
    stopTimer(SackTimer);
    state->ackState = 0;
    sackChunk = createSack();

    SCTPMessage* sctpmsg = new SCTPMessage();
    sctpmsg->setBitLength(SCTP_COMMON_HEADER*8);
    if (state->auth && state->peerAuth && typeInChunkList(SACK)) {
        authChunk = createAuthChunk();
        sctpmsg->addChunk(authChunk);
        SCTP::AssocStatMap::iterator it = sctpMain->assocStatMap.find(assocId);
        it->second.numAuthChunksSent++;
    }
    sctpmsg->addChunk(sackChunk);

    sendSACKviaSelectedPath(sctpmsg);
}

void SCTPAssociation::sendDataArrivedNotification(uint16 sid)
{

    sctpEV3<<"SendDataArrivedNotification\n";

    cPacket* cmsg = new cPacket("DataArrivedNotification");
    cmsg->setKind(SCTP_I_DATA_NOTIFICATION);
    SCTPCommand *cmd = new SCTPCommand("notification");
    cmd->setAssocId(assocId);
    cmd->setSid(sid);
    cmd->setNumMsgs(1);
    cmsg->setControlInfo(cmd);

    sendToApp(cmsg);
}

void SCTPAssociation::sendHMacError(const uint16 id)
{
    SCTPMessage *sctpmsg = new SCTPMessage();
    sctpmsg->setBitLength(SCTP_COMMON_HEADER*8);
    SCTPErrorChunk* errorChunk = new SCTPErrorChunk("ErrorChunk");
    errorChunk->setChunkType(ERRORTYPE);
    SCTPSimpleErrorCauseParameter* cause = new SCTPSimpleErrorCauseParameter("Cause");
    cause->setParameterType(UNSUPPORTED_HMAC);
    cause->setBitLength(6*8);
    cause->setValue(id);
    errorChunk->setBitLength(4*8);
    errorChunk->addParameters(cause);
    sctpmsg->addChunk(errorChunk);
}

void SCTPAssociation::putInDeliveryQ(uint16 sid)
{
    SCTPReceiveStream* rStream = receiveStreams.find(sid)->second;
    sctpEV3 << "putInDeliveryQ: SSN=" << rStream->getExpectedStreamSeqNum()
              << " SID=" << sid
              << " QueueSize="<< rStream->getOrderedQ()->getQueueSize() << endl;
    while (rStream->getOrderedQ()->getQueueSize()>0)
    {
        /* dequeue first from reassembly Q */
        SCTPDataVariables* chunk =
            rStream->getOrderedQ()-> dequeueChunkBySSN(rStream->getExpectedStreamSeqNum());
        if (chunk) {
            sctpEV3 << "putInDeliveryQ::chunk " <<chunk->tsn
                      <<", sid " << chunk->sid <<" and ssn " << chunk->ssn
                      <<" dequeued from ordered queue. queuedReceivedBytes="
                      << state->queuedReceivedBytes << " will be reduced by "
                      << chunk->len/8 << endl;
            state->bufferedMessages--;
            state->queuedReceivedBytes -= chunk->len/8;
            qCounter.roomSumRcvStreams -= ADD_PADDING(chunk->len/8 + SCTP_DATA_CHUNK_LENGTH);

            if (rStream->getDeliveryQ()->checkAndInsertChunk(chunk->tsn, chunk)) {
                state->bufferedMessages++;
                state->queuedReceivedBytes += chunk->len/8;

                sctpEV3 << "data put in deliveryQ; queuedBytes now "
                          << state->queuedReceivedBytes << endl;
                qCounter.roomSumRcvStreams += ADD_PADDING(chunk->len/8 + SCTP_DATA_CHUNK_LENGTH);
                int32 seqnum = rStream->getExpectedStreamSeqNum();
                rStream->setExpectedStreamSeqNum(++seqnum);
                if (rStream->getExpectedStreamSeqNum() > 65535) {
                    rStream->setExpectedStreamSeqNum(0);
                }
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

    statisticsQueuedReceivedBytes->record(state->queuedReceivedBytes);

    sctpEV3 << simTime() << " Calling pushUlp(" << state->queuedReceivedBytes
              << " bytes queued) ..." << endl
              << "messagesToPush=" << state->messagesToPush
              << " pushMessagesLeft=" << state->pushMessagesLeft
              << " restrict=" << restrict
              << " buffered Messages=" << state->bufferedMessages << endl;
    uint32 i = state->nextRSid;
    uint64 tempQueuedBytes = 0;
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

            // ====== Non-revokably acknowledge chunks of the message ==========
            bool dummy;
            for (uint32 j = chunk->tsn; j < chunk->tsn + chunk->fragments; j++)
                state->gapList.updateGapList(j, dummy, false);

            tempQueuedBytes = state->queuedReceivedBytes;
            state->queuedReceivedBytes -= chunk->len/8;
            state->bufferedMessages--;
            sctpEV3 << "buffered Messages now " << state->bufferedMessages << endl;
            if (state->swsAvoidanceInvoked) {
                statisticsQueuedReceivedBytes->record(state->queuedReceivedBytes);
                /* now check, if user has read enough so that window opens up more than one MTU */
                if ((state->messageAcceptLimit>0 &&
                        (int32)state->localMsgRwnd - state->bufferedMessages >= 3 &&
                        (int32)state->localMsgRwnd - state->bufferedMessages <= 8)
                        ||
                        (state->messageAcceptLimit==0 &&
                                (int32)(state->localRwnd - state->queuedReceivedBytes-state->bufferedMessages*state->bytesToAddPerRcvdChunk) >= (int32)(state->swsLimit) &&
                                (int32)(state->localRwnd - state->queuedReceivedBytes-state->bufferedMessages*state->bytesToAddPerRcvdChunk) <= (int32)(state->swsLimit+state->assocPmtu))) {
                    state->swsMsgInvoked = false;
                    /* only if the window has opened up more than one MTU we will send a SACK */
                    state->swsAvoidanceInvoked = false;
                    sctpEV3<<"pushUlp: Window opens up to "<<(int32)state->localRwnd-state->queuedReceivedBytes<<" bytes: sending a SACK. SWS Avoidance INACTIVE\n";

                    sendSack();
                }
            }
            else if ((int32)(state->swsLimit) == 0) {
                sendSack();
            } else if ((tempQueuedBytes > state->localRwnd * 3 / 4) && (state->queuedReceivedBytes <= state->localRwnd * 3 / 4)) {
                sendSack();
            }
            sctpEV3 << "Push TSN " << chunk->tsn
                      << ": sid="     << chunk->sid << " ssn=" << chunk->ssn << endl;
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
            EndToEndDelay->record(simTime() - chunk->firstSendTime);
            SCTP::AssocStatMap::iterator iter = sctpMain->assocStatMap.find(assocId);
            if (iter->second.numEndToEndMessages >= iter->second.startEndToEndDelay &&
                    (iter->second.numEndToEndMessages < iter->second.stopEndToEndDelay || !iter->second.stopEndToEndDelay)) {
                iter->second.cumEndToEndDelay += (simTime() - chunk->firstSendTime);
            }
            iter->second.numEndToEndMessages++;

            // set timestamp to sending time
            chunk->userData->setTimestamp(chunk->firstSendTime);
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
    dataChunk->setIBit(chunk->ibit);
    dataChunk->setEnqueuingTime(chunk->enqueuingTime);
    dataChunk->setFirstSendTime(chunk->firstSendTime);
    dataChunk->setBitLength(SCTP_DATA_CHUNK_LENGTH*8);
    msg->setBitLength(chunk->len);
    dataChunk->encapsulate(msg);
    return dataChunk;
}

void SCTPAssociation::addPath(const IPvXAddress& addr)
{
    sctpEV3<<"Add Path remote address: "<<addr<<"\n";

    SCTPPathMap::iterator i = sctpPathMap.find(addr);
    if (i==sctpPathMap.end())
    {
        sctpEV3<<__LINE__<<" get new path for "<<addr<<"\n";
        SCTPPathVariables* path = new SCTPPathVariables(addr, this);
        sctpPathMap[addr] = path;
        qCounter.roomTransQ[addr] = 0;
        qCounter.bookedTransQ[addr] = 0;
        qCounter.roomRetransQ[addr] = 0;
    }
    sctpEV3<<"path added\n";
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
        stopTimer(path->ResetTimer);
        delete path->ResetTimer;
        stopTimer(path->AsconfTimer);
        delete path->AsconfTimer;
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
    sctpEV3 << simTime() << ":\tmakeRoomForTsn:"
            << " tsn=" << tsn
            << " length=" << length
            << " highestTSN=" << state->gapList.getHighestTSNReceived() << endl;
    calculateRcvBuffer();

    // Reneging may not happen when it is turned off!
    assert(state->disableReneging == false);

    // Get the highest TSN of the GapAck blocks.
    uint32 tryTSN = state->gapList.getHighestTSNReceived();
    uint32  sum = 0;
    while ((sum < length) &&
            (tryTSN > state->gapList.getCumAckTSN())) {

        // ====== New TSN is larger than highest one in GapList? ==============
        if (tsnGt(tsn, tryTSN)) {
            // There is no space for a TSN that high!
            sctpEV3 << "makeRoomForTsn:"
                    << " tsn=" << tryTSN
                    << " tryTSN=" << tryTSN << " -> no space" << endl;
            return false;
        }

        const uint32 oldSum = sum;
        // ====== Iterate all streams to find chunk with TSN "tryTSN" =========
        for (SCTPReceiveStreamMap::iterator iterator = receiveStreams.begin();
                iterator != receiveStreams.end(); iterator++) {
            SCTPReceiveStream* receiveStream = iterator->second;

            // ====== Get chunk to drop ========================================
            SCTPQueue* queue;
            if (uBit) {
                queue = receiveStream->getUnorderedQ();   // Look in unordered queue
            }
            else {
                queue = receiveStream->getOrderedQ();     // Look in ordered queue
            }
            SCTPDataVariables* chunk = queue->getChunk(tryTSN);
            if (chunk == NULL) {   // 12.06.08
                sctpEV3 << tryTSN << " not found in orderedQ. Try deliveryQ" << endl;
                // Chunk is already in delivery queue.
                queue = receiveStream->getDeliveryQ();
                chunk = queue->getChunk(tryTSN);
            }

            // ====== A chunk has been found -> drop it ========================
            if (chunk != NULL) {
                sum += chunk->len;
                if (queue->deleteMsg(tryTSN)) {
                    sctpEV3 << tryTSN << " found and deleted" << endl;
                    state->bufferedMessages--;
                    state->queuedReceivedBytes -= chunk->len/8;
                    if (ssnGt(receiveStream->getExpectedStreamSeqNum(), chunk->ssn)) {
                        receiveStream->setExpectedStreamSeqNum(chunk->ssn);
                    }

                    SCTP::AssocStatMap::iterator iter = sctpMain->assocStatMap.find(assocId);
                    iter->second.numChunksReneged++;
                }
                qCounter.roomSumRcvStreams -= ADD_PADDING(chunk->len/8 + SCTP_DATA_CHUNK_LENGTH);
                state->gapList.removeFromGapList(tryTSN);

                break;
            }
            else {
                sctpEV3 << "TSN " << tryTSN << " not found in stream "
                        << receiveStream->getStreamId() << endl;
            }
        }
        if (sum == oldSum) {
            sctpEV3 << tryTSN << " not found in any stream" << endl;
        }
        tryTSN--;
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
    return state->gapList.tsnInGapList(tsn);
}

SCTPDataVariables* SCTPAssociation::makeVarFromMsg(SCTPDataChunk* dataChunk)
{
    SCTPDataVariables* chunk = new SCTPDataVariables();

    chunk->bbit = dataChunk->getBBit();
    chunk->ebit = dataChunk->getEBit();
    chunk->ibit = dataChunk->getIBit();
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
    sctpEV3 << "smsg encapsulate? " << smsg->getEncaps() << endl;
    if (smsg->getEncaps())
        chunk->len = smsg->getBitLength();
    else
        chunk->len = smsg->getDataLen()*8;
    chunk->firstSendTime = dataChunk->getFirstSendTime();
    calculateRcvBuffer();

    sctpEV3 << "makeVarFromMsg: queuedBytes has been increased to "
              << state->queuedReceivedBytes << endl;
    return chunk;
}

void SCTPAssociation::advancePeerTsn()
{
    // Rewrote code for efficiency, it consomed >40% of total CPU time before!
    // Find the highest TSN to advance to, not just the first one.
    SCTPQueue::PayloadQueue::iterator iterator = retransmissionQ->payloadQueue.find(state->advancedPeerAckPoint + 1);
    while (iterator != retransmissionQ->payloadQueue.end()) {
        if ((iterator->second->hasBeenAbandoned == true)) {
            state->advancedPeerAckPoint = iterator->second->tsn;
            state->ackPointAdvanced = true;
            iterator++;
        }
        else {
            if (iterator->second->hasBeenAcked == true)
                iterator++;
            else
                break;
        }
    }

    sctpEV3 << "advancedPeerTsnAck now=" << state->advancedPeerAckPoint << endl;
}

SCTPDataVariables* SCTPAssociation::getOutboundDataChunk(const SCTPPathVariables* path,
                                                                            const int32                  availableSpace,
                                                                            const int32                  availableCwnd)
{
    /* are there chunks in the transmission queue ? If Yes -> dequeue and return it */
    sctpEV3 << "getOutboundDataChunk(" << path->remoteAddress << "):"
              << " availableSpace=" << availableSpace
              << " availableCwnd="  << availableCwnd
              << endl;
    if (!transmissionQ->payloadQueue.empty()) {
        for (SCTPQueue::PayloadQueue::iterator it = transmissionQ->payloadQueue.begin();
             it != transmissionQ->payloadQueue.end(); it++) {
            SCTPDataVariables* chunk = it->second;
            if ( (chunkHasBeenAcked(chunk) == false) && !chunk->hasBeenAbandoned &&
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
    sctpEV3 << "no chunk found in transmissionQ\n";
    return NULL;
}


bool SCTPAssociation::chunkMustBeAbandoned(SCTPDataVariables* chunk, SCTPPathVariables* sackPath)
{
	switch (chunk->prMethod)
	{
	    case PR_TTL:
            if (chunk->expiryTime > 0 && chunk->expiryTime <= simTime()) {
                if (!chunk->hasBeenAbandoned) {
                    sctpEV3 << "TSN " << chunk->tsn << " will be abandoned"
                                      << " (expiryTime=" << chunk->expiryTime
                                      << " sendTime=" << chunk->sendTime << ")" << endl;
                    chunk->hasBeenAbandoned = true;
                    chunk->sendForwardIfAbandoned = true;
                    sendIndicationToApp(SCTP_I_ABANDONED);
                }
            }
            break;
        case PR_RTX:
			if (chunk->numberOfRetransmissions >= chunk->allowedNoRetransmissions) {
				if (!chunk->hasBeenAbandoned) {
					sctpEV3 << "chunkMustBeAbandoned: TSN " << chunk->tsn << " will be abandoned"
							<< " (maxRetransmissions=" << chunk->allowedNoRetransmissions << ")" << endl;
					chunk->hasBeenAbandoned = true;
					chunk->sendForwardIfAbandoned = true;
					decreaseOutstandingBytes(chunk);
					chunk->countsAsOutstanding = false;
					sendIndicationToApp(SCTP_I_ABANDONED);
				}
			}
			break;
	}

	if (chunk->hasBeenAbandoned) {
        return true;
	}
	return false;
}

SCTPDataVariables* SCTPAssociation::peekAbandonedChunk(const SCTPPathVariables* path)
{
    SCTPDataVariables* retChunk = NULL;

    if (state->prMethod != 0 && !retransmissionQ->payloadQueue.empty())
    {
        for (SCTPQueue::PayloadQueue::iterator it = retransmissionQ->payloadQueue.begin();
             it != retransmissionQ->payloadQueue.end(); it++) {
            SCTPDataVariables* chunk = it->second;

            if (chunk->getLastDestinationPath() == path) {
                /* Apply policies if necessary */
                if (!chunk->hasBeenAbandoned && !chunk->hasBeenAcked &&
                        (chunk->hasBeenFastRetransmitted || chunk->hasBeenTimerBasedRtxed)) {
                    switch (chunk->prMethod) {
                        case PR_TTL:
                            if (chunk->expiryTime > 0 && chunk->expiryTime <= simTime()) {
                                if (!chunk->hasBeenAbandoned) {
                                    sctpEV3 << "TSN " << chunk->tsn << " will be abandoned"
                                            << " (expiryTime=" << chunk->expiryTime
                                            << " sendTime=" << chunk->sendTime << ")" << endl;
                                    chunk->hasBeenAbandoned = true;
                                    sendIndicationToApp(SCTP_I_ABANDONED);
                                }
                            }
                            break;
                        case PR_RTX:
                            if (chunk->hasBeenFastRetransmitted && chunk->numberOfRetransmissions >= chunk->allowedNoRetransmissions) {
                                if (!chunk->hasBeenAbandoned) {
                                    sctpEV3 << "peekAbandonedChunk: TSN " << chunk->tsn << " will be abandoned"
                                            << " (maxRetransmissions=" << chunk->allowedNoRetransmissions << ")" << endl;
                                    chunk->hasBeenAbandoned = true;
                                    sendIndicationToApp(SCTP_I_ABANDONED);
                                }
                            }
                            break;
                    }
                }

                if (chunk->hasBeenAbandoned && chunk->sendForwardIfAbandoned) {
                    retChunk = chunk;
                }
            }

        }
    }
    return retChunk;
}

SCTPDataMsg* SCTPAssociation::dequeueOutboundDataMsg(SCTPPathVariables* path,
        const int32        availableSpace,
        const int32        availableCwnd)
{
    SCTPDataMsg* datMsg = NULL;
    cPacketQueue* streamQ = NULL;
    int32 nextStream = -1;

    sctpEV3<<"dequeueOutboundDataMsg: " << availableSpace <<" bytes left to be sent" << endl;
    /* Only change stream if we don't have to finish a fragmented message */
    if (state->lastMsgWasFragment)
        nextStream = state->lastStreamScheduled;
    else
        nextStream = (this->*ssFunctions.ssGetNextSid)(path, false);

    if (nextStream == -1)
        return NULL;

    sctpEV3<<"dequeueOutboundDataMsg: now stream "<< nextStream << endl;

    for (SCTPSendStreamMap::iterator iter=sendStreams.begin(); iter!=sendStreams.end(); ++iter)
    {
        if ((int32)iter->first==nextStream)
        {
            SCTPSendStream* stream = iter->second;
            streamQ = NULL;

            if (!stream->getUnorderedStreamQ()->empty())
            {
                streamQ = stream->getUnorderedStreamQ();
                sctpEV3<<"DequeueOutboundDataMsg() found chunks in stream "<<iter->first<<" unordered queue, queue size="<<stream->getUnorderedStreamQ()->getLength()<<"\n";
            }
            else if (!stream->getStreamQ()->empty())
            {
                streamQ = stream->getStreamQ();
                sctpEV3<<"DequeueOutboundDataMsg() found chunks in stream "<<iter->first<<" ordered queue, queue size="<<stream->getStreamQ()->getLength()<<"\n";
            }

            if (streamQ)
            {
                int32 b = ADD_PADDING( ((SCTPDataMsg*)streamQ->front())->getEncapsulatedPacket()->getByteLength()+SCTP_DATA_CHUNK_LENGTH);

                /* check if chunk found in queue has to be fragmented */
                if (b > (int32)state->assocPmtu - IP_HEADER_LENGTH - SCTP_COMMON_HEADER)
                {
                    /* START FRAGMENTATION */
                    SCTPDataMsg* datMsgQueued = (SCTPDataMsg*)streamQ->pop();
                    cPacket*     datMsgQueuedEncMsg = datMsgQueued->getEncapsulatedPacket();
                    SCTPDataMsg* datMsgLastFragment = NULL;
                    uint32       offset = 0;
                    uint32       msgbytes = state->assocPmtu - IP_HEADER_LENGTH - SCTP_COMMON_HEADER - SCTP_DATA_CHUNK_LENGTH;
                    const uint16 fullSizedPackets = (uint16)(datMsgQueued->getByteLength() / msgbytes);
                    sctpEV3<<"Fragmentation: chunk " << &datMsgQueued << ", size = " << datMsgQueued->getByteLength() << endl;
                    sctpEV3<<assocId<<": number of fullSizedPackets: "<<fullSizedPackets<<endl;
                    uint16 pcounter = 0;

                    while (datMsgQueued)
                    {
                        /* detemine size of fragment, either max payload or what's left */

                        if (msgbytes > datMsgQueuedEncMsg->getByteLength() - offset)
                            msgbytes = datMsgQueuedEncMsg->getByteLength() - offset;

                        /* new DATA msg */
                        SCTPDataMsg* datMsgFragment = new SCTPDataMsg();
                        datMsgFragment->setSid(datMsgQueued->getSid());
                        datMsgFragment->setPpid(datMsgQueued->getPpid());
                        if (++pcounter == fullSizedPackets && sctpMain->sackNow)
                            datMsgFragment->setSackNow(true);
                        else
                            datMsgFragment->setSackNow(datMsgQueued->getSackNow());
                        datMsgFragment->setInitialDestination(datMsgQueued->getInitialDestination());
                        datMsgFragment->setEnqueuingTime(datMsgQueued->getEnqueuingTime());
                        datMsgFragment->setPrMethod(datMsgQueued->getPrMethod());
                        datMsgFragment->setPriority(datMsgQueued->getPriority());
                        datMsgFragment->setStrReset(datMsgQueued->getStrReset());
                        datMsgFragment->setMsgNum(datMsgQueued->getMsgNum());
                        datMsgFragment->setOrdered(datMsgQueued->getOrdered());
                        datMsgFragment->setExpiryTime(datMsgQueued->getExpiryTime());
                        datMsgFragment->setRtx(datMsgQueued->getRtx());
                        datMsgFragment->setFragment(true);
                        if (state->padding)
                            datMsgFragment->setBooksize(ADD_PADDING(msgbytes + state->header));
                        else
                            datMsgFragment->setBooksize(msgbytes + state->header);

                        /* is this the first fragment? */
                        if (offset == 0)
                            datMsgFragment->setBBit(true);

                        /* new msg */
                        cPacket* datMsgFragmentEncMsg = datMsgQueuedEncMsg->dup();

                        datMsgFragmentEncMsg->setByteLength(msgbytes);

                        SCTPSimpleMessage* datMsgQueuedSimple = dynamic_cast<SCTPSimpleMessage*>(datMsgQueuedEncMsg);
                        SCTPSimpleMessage* datMsgFragmentSimple = dynamic_cast<SCTPSimpleMessage*>(datMsgFragmentEncMsg);
                        if ((datMsgQueuedSimple != NULL) &&
                                (datMsgFragmentSimple != NULL) &&
                                (datMsgQueuedSimple->getDataArraySize() >= msgbytes + offset)) {
                            datMsgFragmentSimple->setDataArraySize(msgbytes);
                            datMsgFragmentSimple->setDataLen(msgbytes);
                            /* copy data */
                            for (uint32 i = offset; i < offset + msgbytes; i++) {
                                datMsgFragmentSimple->setData(i - offset, datMsgQueuedSimple->getData(i));
                            }
                        }

                        offset += msgbytes;
                        datMsgFragment->encapsulate(datMsgFragmentEncMsg);

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
                        sctpEV3 << "Fragmentation: fragment " << &datMsgFragment << " created, length = " << datMsgFragmentEncMsg->getByteLength() << ", queue size = " << streamQ->getLength() << endl;

                        datMsgLastFragment = datMsgFragment;

                        /* all fragments done? */
                        if (datMsgQueuedEncMsg->getByteLength() == offset)
                        {
                            datMsgFragment->setEBit(true);
                            if (sctpMain->sackNow)
                                datMsgFragment->setSackNow(true);
                            /* remove original element */
                            sctpEV3<<"Fragmentation: delete " << &datMsgQueued << endl;
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

                    b = ADD_PADDING(((SCTPDataMsg*)streamQ->front())->getEncapsulatedPacket()->getByteLength()+SCTP_DATA_CHUNK_LENGTH);
                    /* FRAGMENTATION DONE */
                }

                if ((b <= availableSpace) &&
                     ( (int32)((SCTPDataMsg*)streamQ->front())->getBooksize() <= availableCwnd)) {
                    datMsg = (SCTPDataMsg*)streamQ->pop();
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

                    sctpEV3<<"DequeueOutboundDataMsg() found chunk ("<<&datMsg<<") in the stream queue "<<&iter->first<<"("<<streamQ<<") queue size="<<streamQ->getLength()<<"\n";
                 }
            }
            break;
        }
    }
    if (datMsg != NULL)
    {
        qCounter.roomSumSendStreams -= ADD_PADDING(datMsg->getEncapsulatedPacket()->getByteLength()+SCTP_DATA_CHUNK_LENGTH);
        qCounter.bookedSumSendStreams -= datMsg->getBooksize();
    }
    return (datMsg);
}


bool SCTPAssociation::nextChunkFitsIntoPacket(SCTPPathVariables* path, int32 bytes)
{
    int32 nextStream = -1;
    SCTPSendStream* stream;

    /* Only change stream if we don't have to finish a fragmented message */
    if (state->lastMsgWasFragment)
        nextStream = state->lastStreamScheduled;
    else
        nextStream = (this->*ssFunctions.ssGetNextSid)(path, true);

    if (nextStream == -1)
        return false;

    stream = sendStreams.find(nextStream)->second;

    if (stream)
    {
        cPacketQueue* streamQ = NULL;

        if (!stream->getUnorderedStreamQ()->empty())
            streamQ = stream->getUnorderedStreamQ();
        else if (!stream->getStreamQ()->empty())
            streamQ = stream->getStreamQ();

        if (streamQ)
        {
            int32 b = ADD_PADDING(((SCTPDataMsg*)streamQ->front())->getEncapsulatedPacket()->getByteLength()+SCTP_DATA_CHUNK_LENGTH);

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
            SCTPPathVariables* newPath = iterator->second;
            if (newPath == oldPath) {
                if (++hit == 1) {
                    continue;
                }
                else {
                    break;
                }
            }
            if (newPath->activePath) {
                return newPath;
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
            sctpEV3 << "Chunk " << chunk->tsn << " is scheduled for FastRetransmission. Next destination = "
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
    if ( (!state->sendHeartbeatsOnActivePaths) || (!state->enableHeartbeats) ) {
        /* restart hb_timer on this path */
        stopTimer(path->HeartbeatTimer);
        if (state->enableHeartbeats)
        {
            path->heartbeatTimeout = path->pathRto + (double)sctpMain->par("hbInterval");
            startTimer(path->HeartbeatTimer, path->heartbeatTimeout);
            sctpEV3 << "Restarting HB timer on path " << path->remoteAddress
                    << " to expire at time " << path->heartbeatTimeout << endl;
        }
    }

    path->cwndTimeout = path->pathRto;
    stopTimer(path->CwndTimer);
    startTimer(path->CwndTimer, path->cwndTimeout);

    sctpEV3 << "Restarting CWND timer on path "<< path->remoteAddress
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
        sctpEV3 << "Path MTU of Interface "<< i << " = " << path->pmtu <<"\n";
        if (path->pmtu < state->assocPmtu)
        {
            state->assocPmtu = path->pmtu;
        }
        initCCParameters(path);
        path->pathRto = (double)sctpMain->par("rtoInitial");
        path->srtt = path->pathRto;
        path->rttvar = SIMTIME_ZERO;
        /* from now on we may have one update per RTO/SRTT */
        path->rttUpdateTime = SIMTIME_ZERO;


        path->partialBytesAcked = 0;
        path->outstandingBytes = 0;
        path->activePath = true;
        // Timer probably not running, but stop it anyway I.R.
        stopTimer(path->T3_RtxTimer);

        if (path->remoteAddress == state->initialPrimaryPath && !path->confirmed) {
            path->confirmed = true;
        }
        sctpEV3 << getFullPath() << " numberOfLocalAddresses=" << state->localAddresses.size() << "\n";
        if (state->enableHeartbeats) {
            path->heartbeatTimeout = (double)sctpMain->par("hbInterval")+i*path->pathRto;
            stopTimer(path->HeartbeatTimer);
            sendHeartbeat(path);
            startTimer(path->HeartbeatTimer, path->heartbeatTimeout);
            startTimer(path->HeartbeatIntervalTimer, path->heartbeatIntervalTimeout);
        }
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
    state->errorCount = 0;
    path->pathErrorCount = 0;
    if (path->activePath == false) {
        /* notify the application */
        pathStatusIndication(path, true);
        sctpEV3 << "Path " << path->remoteAddress
                  << " state changes from INACTIVE to ACTIVE !!!" << endl;
        path->activePath = true;   // Mark path as active!
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
        if (simTime() > path->rttUpdateTime) {
            if (path->rttUpdateTime == SIMTIME_ZERO) {
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
            // RFC 2960, sect. 6.3.1: new RTT measurements SHOULD be made no more
            //                                than once per round-trip.
            path->rttUpdateTime = simTime() + path->srtt;
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

int SCTPAssociation::getAddressLevel(const IPvXAddress& addr)
{
    if (addr.isIPv6())
    {
        switch(addr.get6().getScope())
        {
            case IPv6Address::UNSPECIFIED:
            case IPv6Address::MULTICAST:
                return 0;

            case IPv6Address::LOOPBACK:
                return 1;

            case IPv6Address::LINK:
                return 2;

            case IPv6Address::SITE:
                return 3;

            case IPv6Address::GLOBAL:
                return 4;

            default:
                throw cRuntimeError("Unknown IPv6 scope: %d", (int)(addr.get6().getScope()));
        }
    }
    else
    {
        switch(addr.get4().getAddressCategory())
        {
            case IPv4Address::UNSPECIFIED:
            case IPv4Address::THIS_NETWORK:
            case IPv4Address::MULTICAST:
            case IPv4Address::BROADCAST:
            case IPv4Address::BENCHMARK:
            case IPv4Address::IPv6_TO_IPv4_RELAY:
            case IPv4Address::IETF:
            case IPv4Address::TEST_NET:
            case IPv4Address::RESERVED:
                return 0;

            case IPv4Address::LOOPBACK:
                return 1;

            case IPv4Address::LINKLOCAL:
                return 2;

            case IPv4Address::PRIVATE_NETWORK:
                return 3;

            case IPv4Address::GLOBAL:
                return 4;

            default:
                throw cRuntimeError("Unknown IPv4 address category: %d", (int)(addr.get4().getAddressCategory()));
        }
    }
}

void SCTPAssociation::putInTransmissionQ(const uint32 tsn, SCTPDataVariables* chunk)
{
    if (chunk->countsAsOutstanding) {
        decreaseOutstandingBytes(chunk);
    }
    SCTPQueue::PayloadQueue::iterator it = transmissionQ->payloadQueue.find(tsn);
    if (it == transmissionQ->payloadQueue.end()) {
        sctpEV3 << "putInTransmissionQ: insert tsn=" << tsn << endl;
        chunk->wasDropped = true;
        chunk->wasPktDropped = true;
        chunk->hasBeenFastRetransmitted = true;
        chunk->setNextDestination(chunk->getLastDestinationPath());
        if (!transmissionQ->checkAndInsertChunk(chunk->tsn, chunk)) {
            sctpEV3 << "putInTransmissionQ: cannot add message/chunk (TSN="
                    << tsn << ") to the transmissionQ" << endl;
        }
        else {
            chunk->enqueuedInTransmissionQ = true;
            CounterMap::iterator q = qCounter.roomTransQ.find(chunk->getNextDestination());
            q->second += ADD_PADDING(chunk->len/8+SCTP_DATA_CHUNK_LENGTH);
            CounterMap::iterator qb = qCounter.bookedTransQ.find(chunk->getNextDestination());
            qb->second += chunk->booksize;
            sctpEV3 << "putInTransmissionQ: " << transmissionQ->getQueueSize() << " chunks="
                    << q->second << " bytes" << endl;
        }
    }
}

