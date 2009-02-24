//
// Copyright (C) 2008 Irene Ruengeler
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


#include <string.h>
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
#include <stdlib.h>


//
// helper functions
//
inline uint32 uint32rand() {
	return (uint32)intrand(0xffff) << 16 + (uint32)intrand(0xffff);
}

void SCTPAssociation::printSctpPathMap()
{
	SCTPPathVariables* path;
	sctpEV3<<"\nSCTP PathMap\n";
	for (SCTPPathMap::iterator i = sctpPathMap.begin(); i!=sctpPathMap.end(); ++i)
	{
		path = i->second;

			sctpEV3<<"address: "<<path->remoteAddress<<"  osb: "<<path->outstandingBytes<<" cwnd: "<<path->cwnd<<"\n";

	}
}

const char *SCTPAssociation::stateName(int32 state)
{
#define CASE(x) case x: s=#x+7; break
	const char *s = "unknown";
	switch (state)
	{
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

const char *SCTPAssociation::eventName(int32 event)
{
#define CASE(x) case x: s=#x+7; break
	const char *s = "unknown";
	switch (event)
	{
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
		CASE(SCTP_E_QUEUE);
		CASE(SCTP_E_NO_MORE_OUTSTANDING);
		CASE(SCTP_E_IGNORE);
		CASE(SCTP_E_DELIVERED);
		CASE(SCTP_E_SEND_SHUTDOWN_ACK);
		CASE(SCTP_E_STOP_SENDING);
	}
	return s;
#undef CASE
}

const char *SCTPAssociation::indicationName(int32 code)
{
#define CASE(x) case x: s=#x+7; break
	const char *s = "unknown";
	switch (code)
	{
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
	}
	return s;
#undef CASE
}


uint32 SCTPAssociation::chunkToInt(char* type)
{
	if (strcmp(type, "DATA")==0) return 0;
	if (strcmp(type,  "INIT")==0) return 1;
	if (strcmp(type,  "INIT_ACK")==0) return 2;
	if (strcmp(type,  "SACK")==0) return 3;
	if (strcmp(type,  "HEARTBEAT")==0) return 4;
	if (strcmp(type,  "HEARTBEAT_ACK")==0) return 5;
	if (strcmp(type,  "ABORT")==0) return 6;
	if (strcmp(type,  "SHUTDOWN")==0) return 7;
	if (strcmp(type,  "SHUTDOWN_ACK")==0) return 8;
	if (strcmp(type,  "ERRORTYPE")==0) return 9;
	if (strcmp(type,  "COOKIE_ECHO")==0) return 10;
	if (strcmp(type,  "COOKIE_ACK")==0) return 11;
	if (strcmp(type,  "SHUTDOWN_COMPLETE")==0) return 14;
	sctpEV3<<"ChunkConversion not successful\n";
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

	sctpEV3 << "initTag "<< sctpmsg->getTag() << "\n";

}

SCTPAssociation *SCTPAssociation::cloneAssociation()
{
	SCTPAssociation *assoc = new SCTPAssociation(sctpMain,appGateIndex,assocId);
	const char *queueClass = transmissionQ->getClassName();
	assoc->transmissionQ = check_and_cast<SCTPQueue *>(createOne(queueClass));
	assoc->retransmissionQ = check_and_cast<SCTPQueue *>(createOne(queueClass));

	const char *sctpAlgorithmClass = sctpAlgorithm->getClassName();
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

void SCTPAssociation::sendToIP(SCTPMessage *sctpmsg)
{
	// final touches on the segment before sending
	sctpmsg->setSrcPort(localPort);
	sctpmsg->setDestPort(remotePort);
	if (sctpmsg->getTag()==0)
		sctpmsg->setTag(localVTag);
	sctpmsg->setChecksumOk(true);
	if (remoteAddr.isIPv6())
	{
		IPv6ControlInfo *controlInfo = new IPv6ControlInfo();
		controlInfo->setProtocol(IP_PROT_SCTP);
		controlInfo->setSrcAddr(IPv6Address());
		controlInfo->setDestAddr(remoteAddr.get6());
		sctpmsg->setControlInfo(controlInfo);
		sctpMain->send(sctpmsg,"to_ipv6");
	}
	else
	{
		IPControlInfo *controlInfo = new IPControlInfo();
		controlInfo->setProtocol(IP_PROT_SCTP);
		controlInfo->setSrcAddr(IPAddress("0.0.0.0"));
		controlInfo->setDestAddr(remoteAddr.get4());
		sctpmsg->setControlInfo(controlInfo);
		sctpMain->send(sctpmsg,"to_ip");
	}
}

void SCTPAssociation::sendToIP(SCTPMessage *sctpmsg, IPvXAddress dest)
{

	sctpmsg->setSrcPort(localPort);
	sctpmsg->setDestPort(remotePort);
	sctpmsg->setChecksumOk(true);
	SCTPChunk* chunk = (SCTPChunk*)(sctpmsg->peekFirstChunk());
	if (chunk->getChunkType() == ABORT)
	{
		SCTPAbortChunk* abortChunk;
		abortChunk = check_and_cast<SCTPAbortChunk *>(chunk);
		if (abortChunk->getT_Bit()==1)
			sctpmsg->setTag(peerVTag);
		else
			sctpmsg->setTag(localVTag);
	}
	else if (sctpmsg->getTag()==0)
		sctpmsg->setTag(localVTag);

	if (dest.isIPv6())
	{
		IPv6ControlInfo *controlInfo = new IPv6ControlInfo();
		controlInfo->setProtocol(IP_PROT_SCTP);
		controlInfo->setSrcAddr(IPv6Address());
		controlInfo->setDestAddr(dest.get6());
		sctpmsg->setControlInfo(controlInfo);
		sctpMain->send(sctpmsg,"to_ipv6");
	}
	else
	{
		IPControlInfo *controlInfo = new IPControlInfo();
		controlInfo->setProtocol(IP_PROT_SCTP);
		controlInfo->setSrcAddr(IPAddress("0.0.0.0"));
		controlInfo->setDestAddr(dest.get4());
		sctpmsg->setControlInfo(controlInfo);
		sctpMain->send(sctpmsg,"to_ip");
	}

	sctpEV3 << "Sent to "<<dest<<"\n ";

}

void SCTPAssociation::sendToIP(SCTPMessage *sctpmsg, IPvXAddress src, IPvXAddress dest)
{

	sctpmsg->setSrcPort(localPort);
	sctpmsg->setDestPort(remotePort);
	sctpmsg->setChecksumOk(true);
	SCTPChunk* chunk = (SCTPChunk*)(sctpmsg->peekFirstChunk());
	if (chunk->getChunkType() == ABORT)
	{
		SCTPAbortChunk* abortChunk;
		abortChunk = check_and_cast<SCTPAbortChunk *>(chunk);
		if (abortChunk->getT_Bit()==1)
			sctpmsg->setTag(peerVTag);
		else
			sctpmsg->setTag(localVTag);
	}
 	else if (sctpmsg->getTag()==0)
		sctpmsg->setTag(localVTag);

	if (dest.isIPv6())
	{
		IPv6ControlInfo *controlInfo = new IPv6ControlInfo();
		controlInfo->setProtocol(IP_PROT_SCTP);
		controlInfo->setSrcAddr(src.get6());
		controlInfo->setDestAddr(dest.get6());
		sctpmsg->setControlInfo(controlInfo);
		sctpMain->send(sctpmsg,"to_ipv6");
	}
	else
	{
		IPControlInfo *controlInfo = new IPControlInfo();
		controlInfo->setProtocol(IP_PROT_SCTP);
		controlInfo->setSrcAddr(src.get4());
		controlInfo->setDestAddr(dest.get4());
		sctpmsg->setControlInfo(controlInfo);
		sctpMain->send(sctpmsg,"to_ip");
	}

	sctpEV3 << "Sent to "<<dest<<"\n ";

}

void SCTPAssociation::signalConnectionTimeout()
{
	sendIndicationToApp(SCTP_I_TIMED_OUT);
}

void SCTPAssociation::sendIndicationToApp(int32 code)
{
	cPacket *msg = new cPacket(indicationName(code));
	msg->setKind(code);
	SCTPCommand *ind = new SCTPCommand(indicationName(code));
	ind->setAssocId(assocId);
	ind->setLocalAddr(localAddr);
	ind->setRemoteAddr(remoteAddr);
	msg->setControlInfo(ind);
	sctpMain->send(msg, "to_appl", appGateIndex);
}

void SCTPAssociation::sendEstabIndicationToApp()
{
	cPacket *msg = new cPacket(indicationName(SCTP_I_ESTABLISHED));
	msg->setKind(SCTP_I_ESTABLISHED);

	SCTPConnectInfo *ind = new SCTPConnectInfo("CI");
	ind->setAssocId(assocId);
	ind->setLocalAddr(localAddr);
	ind->setRemoteAddr(remoteAddr);
	ind->setLocalPort(localPort);
	ind->setRemotePort(remotePort);
	ind->setRemoteAddresses(remoteAddressList);
	ind->setInboundStreams(inboundStreams);
	ind->setOutboundStreams(outboundStreams);
	msg->setControlInfo(ind);
	sctpMain->send(msg, "to_appl", appGateIndex);
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
	state->primaryPathIndex = remoteAddr;
	// create message consisting of INIT chunk
	SCTPMessage *sctpmsg = new SCTPMessage();
	sctpmsg->setBitLength(SCTP_COMMON_HEADER*8);
	SCTPInitChunk *initChunk = new SCTPInitChunk("INIT");
	initChunk->setChunkType(INIT);

	initChunk->setInitTag(uint32rand());

	peerVTag = initChunk->getInitTag();
	sctpEV3<<"INIT from "<<localAddr<<":InitTag="<<peerVTag<<"\n";
	initChunk->setA_rwnd(sctpMain->par("arwnd"));
	initChunk->setNoOutStreams(outboundStreams);
	initChunk->setNoInStreams(SCTP_DEFAULT_INBOUND_STREAMS);
	initChunk->setInitTSN(1000);
	state->nextTSN=initChunk->getInitTSN();
	state->lastTSN = initChunk->getInitTSN() + state->numRequests - 1;
	initTsn=initChunk->getInitTSN();
	IInterfaceTable *ift = interfaceTableAccess.get();
	sctpEV3<<"add local address\n";
	if (localAddressList.front() == IPvXAddress("0.0.0.0"))
	{
		for (int32 i=0; i<ift->getNumInterfaces(); ++i)
		{
			if (ift->getInterface(i)->ipv4Data()!=NULL)
			{
				sctpEV3<<"save address "<<ift->getInterfaceById(i)->ipv4Data()->getIPAddress()<<"\n";
				adv.push_back(ift->getInterface(i)->ipv4Data()->getIPAddress());
			}
			else if (ift->getInterface(i)->ipv6Data()!=NULL)
			{
				for (int32 j=0; j<ift->getInterface(i)->ipv6Data()->getNumAddresses(); j++)
				{
					sctpEV3<<"add address "<<ift->getInterface(i)->ipv6Data()->getAddress(j)<<"\n";
					adv.push_back(ift->getInterface(i)->ipv6Data()->getAddress(j));
				}
			}
		}
	}
	else
	{
		adv = localAddressList;
		sctpEV3<<"gebundene Adresse "<<localAddr<<" wird hinzugefuegt\n";
	}
	uint32 addrNum=0;
	bool friendly = false;
	if (remoteAddr.isIPv6())
	{
		for (AddressVector::iterator i=adv.begin(); i!=adv.end(); ++i)
		{
			if (!friendly)
			{
				initChunk->setAddressesArraySize(addrNum+1);
				initChunk->setAddresses(addrNum++,(*i));
				length+=20;
			}
			sctpMain->addLocalAddress(this, (*i));
			state->localAddresses.push_back((*i));
			if (localAddr.isUnspecified())
				localAddr=(*i);
		}
	}
	else
	{
		uint32 rlevel = getLevel(remoteAddr);
		sctpEV3<<"level of remote address="<<rlevel<<"\n";
		for (AddressVector::iterator i=adv.begin(); i!=adv.end(); ++i)
		{
			sctpEV3<<"level of address "<<(*i)<<" = "<<getLevel((*i))<<"\n";
			if (getLevel((*i))>=rlevel)
			{
				initChunk->setAddressesArraySize(addrNum+1);
				initChunk->setAddresses(addrNum++,(*i));
				length+=8;
				sctpMain->addLocalAddress(this, (*i));
				state->localAddresses.push_back((*i));
				if (localAddr.get4().getInt()==0)
					localAddr=(*i);
			}
			else if (rlevel==4 && getLevel((*i))==3 && friendly)
			{
				sctpMain->addLocalAddress(this, (*i));
				state->localAddresses.push_back((*i));
				if (localAddr.get4().getInt()==0)
					localAddr=(*i);
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
			SCTPPathVariables* path = new SCTPPathVariables((*it), this);
			sctpPathMap[(*it)] = path;
			qCounter.roomTransQ[(*it)] = 0;
			qCounter.roomRetransQ[(*it)] = 0;
			qCounter.bookedTransQ[(*it)] = 0;
		}
	}
	else
	{
		SCTPPathVariables* path = new SCTPPathVariables(remoteAddr, this);
		sctpPathMap[remoteAddr] = path;
		qCounter.roomTransQ[remoteAddr] = 0;
		qCounter.roomRetransQ[remoteAddr] = 0;
		qCounter.bookedTransQ[remoteAddr] = 0;
	}
	// send it
	state->initChunk=check_and_cast<SCTPInitChunk *>(initChunk->dup());
	state->initChunk->setName("StateInitChunk");
	printSctpPathMap();
	sctpEV3<<getFullPath()<<" sendInit: localVTag="<<localVTag<<" peerVTag="<<peerVTag<<"\n";
	sendToIP(sctpmsg);
}

void SCTPAssociation::retransmitInit()
{
	SCTPMessage *sctpmsg = new SCTPMessage();
	sctpmsg->setBitLength(SCTP_COMMON_HEADER*8);
	SCTPInitChunk *sctpinit;// = new SCTPInitChunk("INIT");

	sctpEV3<<"Retransmit InitChunk="<<&sctpinit<<"\n";

	sctpinit=check_and_cast<SCTPInitChunk *>(state->initChunk->dup());
	sctpinit->setChunkType(INIT);
	sctpmsg->addChunk(sctpinit);

	sendToIP(sctpmsg);

}



void SCTPAssociation::sendInitAck(SCTPInitChunk* initChunk)
{
	uint32 length = SCTP_INIT_CHUNK_LENGTH;

	state->primaryPathIndex = remoteAddr;
	// create segment
	SCTPMessage *sctpinitack = new SCTPMessage();
	sctpinitack->setBitLength(SCTP_COMMON_HEADER*8);

	sctpinitack->setSrcPort(localPort);
	sctpinitack->setDestPort(remotePort);
	SCTPInitAckChunk *initAckChunk = new SCTPInitAckChunk("INIT_ACK");
	initAckChunk->setChunkType(INIT_ACK);
	SCTPCookie *cookie = new SCTPCookie("CookieUtil");
	cookie->setCreationTime(simulation.getSimTime());
	cookie->setLocalTieTagArraySize(32);
	cookie->setPeerTieTagArraySize(32);
	if (fsm->getState()==SCTP_S_CLOSED)
	{
		do
		{
			peerVTag = uint32rand();
		} while (peerVTag==0);
		initAckChunk->setInitTag(peerVTag);
		initAckChunk->setInitTSN(2000);
		state->nextTSN=initAckChunk->getInitTSN();
		state->lastTSN = initAckChunk->getInitTSN() + state->numRequests - 1;
		cookie->setLocalTag(localVTag);
		cookie->setPeerTag(peerVTag);
		for (int32 i=0; i<32; i++)
		{
			cookie->setLocalTieTag(i,0);
			cookie->setPeerTieTag(i,0);
		}
		sctpinitack->setTag(localVTag);
		sctpEV3<<"state=closed: localVTag="<<localVTag<<" peerVTag="<<peerVTag<<"\n";
	}
	else if (fsm->getState()==SCTP_S_COOKIE_WAIT || fsm->getState()==SCTP_S_COOKIE_ECHOED)
	{
		initAckChunk->setInitTag(peerVTag);
		sctpEV3<<"different state:set InitTag in InitAck: "<<initAckChunk->getInitTag()<<"\n";
		initAckChunk->setInitTSN(state->nextTSN);
		initPeerTsn=initChunk->getInitTSN();
		state->cTsnAck = initPeerTsn - 1;
		cookie->setLocalTag(initChunk->getInitTag());
		cookie->setPeerTag(peerVTag);
		for (int32 i=0; i<32; i++)
		{
			cookie->setPeerTieTag(i,(uint8)intrand(256));
			state->peerTieTag[i] = cookie->getPeerTieTag(i);
			if (fsm->getState()==SCTP_S_COOKIE_ECHOED)
			{
				cookie->setLocalTieTag(i,(uint8)intrand(256));
				state->localTieTag[i] = cookie->getLocalTieTag(i);
			}
			else
				cookie->setLocalTieTag(i,0);
		}
		sctpinitack->setTag(initChunk->getInitTag());
		sctpEV3<<"VTag in InitAck: "<<sctpinitack->getTag()<<"\n";
	}
	else
	{
		sctpEV3<<"other state\n";
		uint32 tag;
		do
		{
				tag = uint32rand();
		} while (tag==0);
				initAckChunk->setInitTag(tag);
		initAckChunk->setInitTSN(state->nextTSN);
		cookie->setLocalTag(localVTag);
		cookie->setPeerTag(peerVTag);
		for (int32 i=0; i<32; i++)
		{
			cookie->setPeerTieTag(i,state->peerTieTag[i]);
			cookie->setLocalTieTag(i,state->localTieTag[i]);
		}
		sctpinitack->setTag(initChunk->getInitTag());
	}
	cookie->setBitLength(SCTP_COOKIE_LENGTH*8);
	initAckChunk->setStateCookie(cookie);
	initAckChunk->setCookieArraySize(0);
	initAckChunk->setA_rwnd(sctpMain->par("arwnd"));
	initAckChunk->setNoOutStreams((unsigned int)min(outboundStreams,initChunk->getNoInStreams()));
	initAckChunk->setNoInStreams(SCTP_DEFAULT_INBOUND_STREAMS);

	initTsn=initAckChunk->getInitTSN();
	uint32 addrNum=0;
	bool friendly = false;
	if (!friendly)
	for (AddressVector::iterator k=state->localAddresses.begin(); k!=state->localAddresses.end(); ++k)
	{
		initAckChunk->setAddressesArraySize(addrNum+1);
		initAckChunk->setAddresses(addrNum++,(*k));
		length+=8;
	}
	uint32 unknownLen = initChunk->getUnrecognizedParametersArraySize();
	if (unknownLen>0)
	{
		sctpEV3<<"Found unrecognized Parameters in INIT chunk with a length of "<<unknownLen<<" bytes.\n";
		initAckChunk->setUnrecognizedParametersArraySize(unknownLen);
		for (uint32 i=0; i<unknownLen; i++)
			initAckChunk->setUnrecognizedParameters(i,initChunk->getUnrecognizedParameters(i));
		length+=unknownLen;
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
	printSctpPathMap();
}

void SCTPAssociation::sendCookieEcho(SCTPInitAckChunk* initAckChunk)
{
	SCTPMessage *sctpcookieecho = new SCTPMessage();
	sctpcookieecho->setBitLength(SCTP_COMMON_HEADER*8);

	sctpEV3<<"SCTPAssociationUtil:sendCookieEcho\n";

	sctpcookieecho->setSrcPort(localPort);
	sctpcookieecho->setDestPort(remotePort);
	SCTPCookieEchoChunk* cookieEchoChunk=new SCTPCookieEchoChunk("COOKIE_ECHO");
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
			cookieEchoChunk->setUnrecognizedParameters(i,initAckChunk->getUnrecognizedParameters(i));
	}
	else
		cookieEchoChunk->setUnrecognizedParametersArraySize(0);
	state->cookieChunk=check_and_cast<SCTPCookieEchoChunk*>(cookieEchoChunk->dup());
	if (len==0)
	{
		state->cookieChunk->setStateCookie(initAckChunk->getStateCookie()->dup());
			}
	sctpcookieecho->addChunk(cookieEchoChunk);
		sendToIP(sctpcookieecho);
}



void SCTPAssociation::retransmitCookieEcho()
{
	SCTPMessage *sctpmsg = new SCTPMessage();
	sctpmsg->setBitLength(SCTP_COMMON_HEADER*8);
	SCTPCookieEchoChunk* cookieEchoChunk=check_and_cast<SCTPCookieEchoChunk*>(state->cookieChunk->dup());
	if (cookieEchoChunk->getCookieArraySize()==0)
	{
		cookieEchoChunk->setStateCookie(state->cookieChunk->getStateCookie()->dup());
	}
	sctpmsg->addChunk(cookieEchoChunk);

	sctpEV3<<"retransmitCookieEcho localAddr="<<localAddr<<"  remoteAddr"<<remoteAddr<<"\n";

	sendToIP(sctpmsg);
}

void SCTPAssociation::sendHeartbeat(SCTPPathVariables *path, bool local)
{
	SCTPMessage *sctpheartbeat = new SCTPMessage();
	sctpheartbeat->setBitLength(SCTP_COMMON_HEADER*8);

	sctpheartbeat->setSrcPort(localPort);
	sctpheartbeat->setDestPort(remotePort);
	SCTPHeartbeatChunk* heartbeatChunk=new SCTPHeartbeatChunk("HEARTBEAT");
	heartbeatChunk->setChunkType(HEARTBEAT);
	heartbeatChunk->setRemoteAddr(path->remoteAddress);
	heartbeatChunk->setTimeField(simulation.getSimTime());
	heartbeatChunk->setBitLength((SCTP_HEARTBEAT_CHUNK_LENGTH+12)*8);
	//heartbeatChunk->setBitLength((SCTP_HEARTBEAT_CHUNK_LENGTH+12+1000)*8);
	sctpheartbeat->addChunk(heartbeatChunk);
	if (local)
	{
		sctpEV3<<"sendHeartbeat: sendToIP from "<<localAddr<<" to "<<path->remoteAddress<<"\n";
		sendToIP(sctpheartbeat, localAddr, path->remoteAddress);
	}
	else
	{
		sctpEV3<<"sendHeartbeat: sendToIP  to "<<path->remoteAddress<<"\n";
		sendToIP(sctpheartbeat, path->remoteAddress);
	}
	path->hbWasAcked=false;
}

void SCTPAssociation::sendHeartbeatAck(SCTPHeartbeatChunk* heartbeatChunk, IPvXAddress src, IPvXAddress dest)
{
	SCTPMessage *sctpheartbeatack = new SCTPMessage();
	sctpheartbeatack->setBitLength(SCTP_COMMON_HEADER*8);

	sctpheartbeatack->setSrcPort(localPort);
	sctpheartbeatack->setDestPort(remotePort);
	SCTPHeartbeatAckChunk* heartbeatAckChunk=new SCTPHeartbeatAckChunk("HEARTBEAT_ACK");
	heartbeatAckChunk->setChunkType(HEARTBEAT_ACK);
	heartbeatAckChunk->setRemoteAddr(heartbeatChunk->getRemoteAddr());
	heartbeatAckChunk->setTimeField(heartbeatChunk->getTimeField());
	int32 len=heartbeatChunk->getInfoArraySize();
        if (len>0)
	{
		heartbeatAckChunk->setInfoArraySize(len);
		for (int32 i=0; i<len; i++)
			heartbeatAckChunk->setInfo(i,heartbeatChunk->getInfo(i));
	}

	heartbeatAckChunk->setBitLength(heartbeatChunk->getBitLength());
	sctpheartbeatack->addChunk(heartbeatAckChunk);
	sctpEV3<<"try to get path for "<<dest<<"\n";
	sctpEV3<<"send heartBeatAck from "<<src<<" to "<<dest<<"\n";
	sendToIP(sctpheartbeatack, src, dest);
}

void SCTPAssociation::sendCookieAck(IPvXAddress dest)
{
	SCTPMessage *sctpcookieack = new SCTPMessage();
	sctpcookieack->setBitLength(SCTP_COMMON_HEADER*8);

	sctpEV3<<"SCTPAssociationUtil:sendCookieACK\n";

	sctpcookieack->setSrcPort(localPort);
	sctpcookieack->setDestPort(remotePort);
	SCTPCookieAckChunk* cookieAckChunk=new SCTPCookieAckChunk("COOKIE_ACK");
	cookieAckChunk->setChunkType(COOKIE_ACK);
	cookieAckChunk->setBitLength(SCTP_COOKIE_ACK_LENGTH*8);
	sctpcookieack->addChunk(cookieAckChunk);
	sendToIP(sctpcookieack, dest);
}

void SCTPAssociation::sendShutdownAck(IPvXAddress dest)
{
	sendAll(dest);
	if (dest!=state->primaryPathIndex)
		sendAll(state->primaryPathIndex);
	if (getOutstandingBytes()==0)
	{
		performStateTransition(SCTP_E_NO_MORE_OUTSTANDING);
		SCTPMessage *sctpshutdownack = new SCTPMessage();
		sctpshutdownack->setBitLength(SCTP_COMMON_HEADER*8);

		sctpEV3<<"SCTPAssociationUtil:sendShutdownACK\n";

		sctpshutdownack->setSrcPort(localPort);
		sctpshutdownack->setDestPort(remotePort);
		SCTPShutdownAckChunk* shutdownAckChunk=new SCTPShutdownAckChunk("SHUTDOWN_ACK");
		shutdownAckChunk->setChunkType(SHUTDOWN_ACK);
		shutdownAckChunk->setBitLength(SCTP_COOKIE_ACK_LENGTH*8);
		sctpshutdownack->addChunk(shutdownAckChunk);
		state->initRexmitTimeout = SCTP_TIMEOUT_INIT_REXMIT;
		state->initRetransCounter = 0;
		stopTimer(T2_ShutdownTimer);
		startTimer(T2_ShutdownTimer,state->initRexmitTimeout);
		stopTimer(T5_ShutdownGuardTimer);
		startTimer(T5_ShutdownGuardTimer,SHUTDOWN_GUARD_TIMEOUT);
		state->shutdownAckChunk=check_and_cast<SCTPShutdownAckChunk*>(shutdownAckChunk->dup());
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
	SCTPShutdownCompleteChunk* shutdownCompleteChunk=new SCTPShutdownCompleteChunk("SHUTDOWN_COMPLETE");
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

	sctpEV3<<"SCTPAssociationUtil:sendABORT localPort="<<localPort<<"  remotePort="<<remotePort<<"\n";

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

	sctpEV3<<"SCTPAssociationUtil:sendShutdown localPort="<<localPort<<"  remotePort="<<remotePort<<"\n";

	msg->setSrcPort(localPort);
	msg->setDestPort(remotePort);
	SCTPShutdownChunk* shutdownChunk = new SCTPShutdownChunk("Shutdown");
	shutdownChunk->setChunkType(SHUTDOWN);
	//shutdownChunk->setCumTsnAck(state->lastTsnAck);
	shutdownChunk->setCumTsnAck(state->cTsnAck);
	shutdownChunk->setBitLength(SCTP_SHUTDOWN_CHUNK_LENGTH*8);
	state->initRexmitTimeout = SCTP_TIMEOUT_INIT_REXMIT;
	state->initRetransCounter = 0;
	stopTimer(T5_ShutdownGuardTimer);
	startTimer(T5_ShutdownGuardTimer,SHUTDOWN_GUARD_TIMEOUT);
	stopTimer(T2_ShutdownTimer);
	startTimer(T2_ShutdownTimer,state->initRexmitTimeout);
	state->shutdownChunk=check_and_cast<SCTPShutdownChunk*>(shutdownChunk->dup());
	msg->addChunk(shutdownChunk);
	sendToIP(msg, remoteAddr);
}


void SCTPAssociation::retransmitShutdown()
{
	SCTPMessage *sctpmsg = new SCTPMessage();
	sctpmsg->setBitLength(SCTP_COMMON_HEADER*8);
	SCTPShutdownChunk* shutdownChunk;
	shutdownChunk=check_and_cast<SCTPShutdownChunk*>(state->shutdownChunk->dup());

	sctpmsg->addChunk(shutdownChunk);

	sctpEV3<<"retransmitShutdown localAddr="<<localAddr<<"  remoteAddr"<<remoteAddr<<"\n";

	sendToIP(sctpmsg);
}

void SCTPAssociation::retransmitShutdownAck()
{
	SCTPMessage *sctpmsg = new SCTPMessage();
	sctpmsg->setBitLength(SCTP_COMMON_HEADER*8);
	SCTPShutdownAckChunk* shutdownAckChunk;
	shutdownAckChunk=check_and_cast<SCTPShutdownAckChunk*>(state->shutdownAckChunk->dup());
	sctpmsg->addChunk(shutdownAckChunk);

	sctpEV3<<"retransmitShutdownAck localAddr="<<localAddr<<"  remoteAddr"<<remoteAddr<<"\n";

	sendToIP(sctpmsg);
}


void SCTPAssociation::scheduleSack(void)
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


SCTPSackChunk* SCTPAssociation::createSack()
{
uint32 key=0, arwnd=0;

	sctpEV3<<"SCTPAssociationUtil:createSACK localPort="<<localPort<<"  remotePort="<<remotePort<<"\n";

	sctpEV3<<" localRwnd="<<state->localRwnd<<"  queuedBytes="<<state->queuedRcvBytes<<"\n";
	if ((int32)(state->localRwnd - state->queuedRcvBytes) <= 0)
	{
		arwnd = 0;
		if (state->swsLimit > 0)
			state->swsAvoidanceInvoked = true;
	}
	else if (state->localRwnd - state->queuedRcvBytes < state->swsLimit || state->swsAvoidanceInvoked == true)
	{
		arwnd = 1;
		if (state->swsLimit > 0)
			state->swsAvoidanceInvoked = true;
		// SWS Avoidance ACTIVE !!!
	}
	else
	{
		arwnd = state->localRwnd - state->queuedRcvBytes;
		sctpEV3<<simulation.getSimTime()<<" arwnd = "<<state->localRwnd<<" - "<<state->queuedRcvBytes<<" = "<<arwnd<<"\n";
	}
	advRwnd->record(arwnd);
	quBytes->record(state->queuedRcvBytes);
	SCTPSackChunk* sackChunk=new SCTPSackChunk("SACK");
	sackChunk->setChunkType(SACK);
	sackChunk->setCumTsnAck(state->cTsnAck);
	sackChunk->setA_rwnd(arwnd);
	uint32 numGaps=state->numGaps;
	uint32 numDups=state->dupList.size();
	uint16 sackLength=SCTP_SACK_CHUNK_LENGTH + numGaps*4 + numDups*4;
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
		sackLength=SCTP_SACK_CHUNK_LENGTH + numGaps*4 + numDups*4;
	}
	sackChunk->setNumGaps(numGaps);
	sackChunk->setNumDupTsns(numDups);
	sackChunk->setBitLength(sackLength*8);

	sctpEV3<<"Sack arwnd="<<sackChunk->getA_rwnd()<<" ctsnAck="<<state->cTsnAck<<" numGaps="<<numGaps<<" numDups="<<numDups<<"\n";

	if (numGaps > 0)
	{
		sackChunk->setGapStartArraySize(numGaps);
		sackChunk->setGapStopArraySize(numGaps);
		for (key=0; key<numGaps; key++)
		{
			sackChunk->setGapStart(key, state->gapStartList[key]);
			sackChunk->setGapStop(key, state->gapStopList[key]);
		}
	}
	if (numDups > 0)
	{
		sackChunk->setDupTsnsArraySize(numDups);
		key=0;
		for(std::list<uint32>::iterator iter=state->dupList.begin(); iter!=state->dupList.end(); iter++)
		{
			sackChunk->setDupTsns(key, (*iter));
			key++;
			if (key == numDups)
				break;
		}
		state->dupList.clear();
	}
	sctpEV3<<endl;
	for (uint32 i=0; i<numGaps; i++)
		sctpEV3<<sackChunk->getGapStart(i)<<" - "<<sackChunk->getGapStop(i)<<"\n";

	sctpEV3<<"send "<<sackChunk->getName()<<" from "<<localAddr<<" to "<<state->lastDataSourceAddress<<"\n";
	return sackChunk;
}

void SCTPAssociation::sendSack(void)
{
	sctpEV3<<"sendSack\n";
	SCTPSackChunk* sackChunk;
	IPvXAddress dpi; 		/* destination path index that will actually be used/was used on last send */
	/* sack timer has expired, reset flag, and send SACK */
	stopTimer(SackTimer);
	state->ackState = 0;
	sackChunk = createSack();

	/* return the SACK to the address where we last got a data chunk from */
	dpi = state->lastDataSourceAddress;
	SCTPMessage* sctpmsg = new SCTPMessage();
	sctpmsg->setBitLength(SCTP_COMMON_HEADER*8);
	sctpmsg->addChunk(sackChunk);
	sendToIP(sctpmsg, dpi);
}



void SCTPAssociation::sendAll(IPvXAddress pathId)
{
	SCTPSackChunk* sackChunk=NULL;
	uint32 chunksAdded=0,totalChunksSent=0,totalPacketsSent=0;
	uint32 nextSsn=0, osb=0;
	bool headerCreated=false, forwPresent = false, firstTime=false;
	IPvXAddress dpi, newDpi, pd;
	int32 dataChunksAdded=0;
	SCTPPathVariables* pathVar;
	SCTPDataChunk* chunkPtr=NULL;
	SCTPMessage* sctpmsg=NULL;
	SCTPDataMsg* datMsg;
	SCTPDataVariables *chunk=NULL, *datVar=NULL;
	bool authAdded = false;
	bool sendBeforeRtx = false;
	bool rtxActive = false;
	bool packetFull = false;
	bool probing = false;
	bool sendOneMorePacket = false;
	int32 tcount = 0, rtcount = 0, scount = 0, bcount = 0, bytesToSend = 0;
	sctpEV3<<"sendAll: pathId="<<pathId<<"\n";
	printSctpPathMap();
	if (pathId == IPvXAddress("0.0.0.0"))
		pd = state->primaryPathIndex;
	else
		pd = pathId;
	newDpi = pd;
	sctpEV3<<"newDpi set to "<<newDpi<<" dpi="<<dpi<<"\n";
	CounterMap::iterator tq=qCounter.roomTransQ.find(pd);
	if (tq != qCounter.roomTransQ.end())
		tcount = tq->second;
	else
		tcount = 0;
	CounterMap::iterator rtq=qCounter.roomRetransQ.find(pd);
	if (rtq != qCounter.roomRetransQ.end())
		rtcount = rtq->second;
	else
		rtcount = 0;
	scount = qCounter.roomSumSendStreams;
	bcount = qCounter.bookedSumSendStreams;
	sctpEV3<<"\nsend all on "<<pd<<": tcount="<<tcount<<"  scount="<<scount<<"  rtcount="<<rtcount<<" nextTsn="<<state->nextTSN<</*" numChunksAllowed="<<state->numChunksAllowed<<*/"\n";
	if (!state->stopSending)  //for M3UA
	{
		if (tcount>0)
		{
			rtxActive = true;
			dpi = pd;
			sctpEV3<<"dpi=pd="<<dpi<<"\n";
		}

		if (tcount == 0 && scount == 0)
		{

			sctpEV3<<"No chunk available!\n";
			bytesToSend = 0;
			bytes.bytesToSend = 0;
			bytes.packet = false;
			bytes.chunk = false;
			dpi = pathId;
			sctpEV3<<"dpi=pathId="<<dpi<<"\n";
		}
		else
		{
				if (tcount==0)
				{
					dpi = state->primaryPathIndex;
					sctpEV3<<"dpi=primary="<<dpi<<"\n";
					firstTime=true;
				}
				if (!(state->resetPending && firstTime))
					bytesAllowedToSend(dpi);
			newDpi = dpi;
			sctpEV3<<"newDpi set to "<<newDpi<<" dpi="<<dpi<<"\n";
		}

		if (newDpi==IPvXAddress("0.0.0.0"))
			opp_error("newDpi unspecified");
		if (bytesToSend < 0)  //24.11.2006
			bytesToSend=0;

		bytesToSend = bytes.bytesToSend;

		pathVar=getPath(dpi);

		sctpEV3<<"after bytesAllowedToSend: bytesToSend="<<bytesToSend<<", packet="<<bytes.packet<<", chunk="<<bytes.chunk<<"\n";
		if (bytes.chunk && (rtcount==0 || tcount>0))
			probing = true;
		/* schedule sending of SACKs at once, when we have fragments to report */
		if ((state->numGaps > 0 || state->dupList.size() > 0) && state->sackAllowed)
		{
			sctpEV3<<"Gaps present ==> ackState = "<<sackFrequency<<"\n";
			state->ackState = sackFrequency;
		}
		if (state->ackState < sackFrequency && bytesToSend == 0 && bytes.chunk==false && bytes.packet==false && forwPresent == false)
		{
			sctpEV3<<"sendAll: nothing to send...chunk pointer is NULL...BYE...\n";
			return;
		}


		sctpEV3<<"sendAll::bytesToSend="<<bytesToSend<<" osb="<<pathVar->outstandingBytes<<" cwnd="<<pathVar->cwnd<<" arwnd="<<state->peerRwnd<<"\n";
		osb = pathVar->outstandingBytes;

		if ((int32)pathVar->outstandingBytes<0)
			opp_error("util %d ,tsna=%d",__LINE__, state->cTsnAck);

	}
	else
		dpi = pd;

	sctpEV3<<"ackState="<<state->ackState<<" Sacktimer="<<SackTimer->isScheduled()<<"\n";
	if (state->ackState >= sackFrequency || (SackTimer->isScheduled() &&  (state->alwaysBundleSack  == true || pathId == dpi)) )
	{
		sctpmsg = new SCTPMessage();
		sctpmsg->setBitLength(SCTP_COMMON_HEADER*8);

		sctpEV3<<"SCTPAssociationUtil:sendAll localPort="<<localPort<<"  remotePort="<<remotePort<<"\n";

		/*build a SACK chunk....*/
		headerCreated = true;
		sackChunk = createSack();

		if (dpi!=remoteAddr)
			dpi = remoteAddr;

		sctpEV3<<"SACK: dpi="<<dpi<<"\n";

		/* ...and add a SACK to the packet... */

		chunksAdded++;
		totalChunksSent++;
		sctpmsg->addChunk(sackChunk);

		if (tcount==0 && bytesToSend>0 && state->nagleEnabled && state->peerRwnd < pathVar->pmtu && pathVar->outstandingBytes > 0)
		{
		// Test whether a full packet can be filled
		sctpEV3<<" peerRwnd="<<state->peerRwnd<<" bytesToSend="<<bytesToSend<<" nextTSN="<<state->nextTSN<<"\n";
			SCTPDataMsg *datMsg = peekOutboundDataMsg();
			uint32 ums = datMsg->getBooksize();
				uint32 num = (uint32)((pathVar->pmtu-sctpmsg->getByteLength()-20)/(ums+SCTP_DATA_CHUNK_LENGTH));
				if (num*ums>state->peerRwnd)
					bytesToSend = 0;
				else
					bytesToSend = num * ums;
		}
		sctpEV3<<"sendAll: appended "<<sackChunk->getBitLength()/8<<" bytes SACK chunk, msg length now "<<sctpmsg->getBitLength()/8<<"\n";

		state->ackState = 0;
			/* stop SACK timer if it is running...				 */
		stopTimer(SackTimer);
		sctpAlgorithm->sackSent();
		state->sackAllowed = false;
	}
	else

	{
		if (tcount==0 && bytesToSend>0 && state->nagleEnabled && state->peerRwnd < pathVar->pmtu && pathVar->outstandingBytes > 0)
		{
		// Test whether a full packet can be filled
		sctpEV3<<" peerRwnd="<<state->peerRwnd<<" bytesToSend="<<bytesToSend<<" nextTSN="<<state->nextTSN<<"\n";
			SCTPDataMsg *datMsg = peekOutboundDataMsg();
			uint32 ums = datMsg->getBooksize();
				uint32 num = (uint32)((pathVar->pmtu-32)/(ums+SCTP_DATA_CHUNK_LENGTH));
				if (num*ums>state->peerRwnd)
					bytesToSend = 0;
				else
					bytesToSend = num * ums;
		}
		if (bytesToSend > 0 || bytes.chunk || bytes.packet || (bytes.chunk && probing))
		{
			if (tcount>0 || scount>0 && (!(state->nagleEnabled && osb>0)||(uint32)scount>=pathVar->pmtu-32 ))
			{
				sctpmsg = new SCTPMessage();
				headerCreated=true;
				sctpmsg->setBitLength(SCTP_COMMON_HEADER*8);
			}
		}
		else
		{
			sctpEV3<<"Logic Error in sendAll: no header created....???\n";

		}
	}

	while ((bytesToSend > 0 || bytes.chunk || bytes.packet ) && !(state->resetPending && firstTime) && headerCreated && !state->stopSending)
	{
		datVar = NULL;
		datMsg = NULL;
		if (tq->second > 0) //take the chunks out of the transmissionQ first
		{
			sctpEV3<<tq->second<<" bytes are in the transQ\n";
			datVar=getOutboundDataChunk(dpi, pathVar->pmtu-sctpmsg->getBitLength()/8 -20);
			sctpEV3<<"sendAll::sctpmsg->length="<<sctpmsg->getBitLength()/8<<" length datVar="<<pathVar->pmtu-sctpmsg->getBitLength()/8 -20<<"\n";
			if (datVar != NULL)
			{
				datVar->numberOfRetransmissions++;
				if (datVar->hasBeenAcked==false)
				{
					datVar->countsAsOutstanding = true;
					datVar->hasBeenRemoved = false;
					getPath(datVar->nextDestination)->outstandingBytes += (datVar->booksize);
					sctpEV3<<"osb="<<getPath(datVar->nextDestination)->outstandingBytes<<"\n";
					CounterMap::iterator i=qCounter.roomRetransQ.find(getPath(datVar->lastDestination)->remoteAddress);
						i->second += ADD_PADDING(datVar->booksize+SCTP_DATA_CHUNK_LENGTH);
				}
			}
			else
			{
				sctpEV3<<"datVar=NULL -> packetFull\n";
				packetFull = true;
			}
		}
		else if (scount>0 && dpi==state->primaryPathIndex)
		{
			sctpEV3<<"scount="<<scount<<", nagle="<<state->nagleEnabled<<", osb="<<osb<<", mtu="<<pathVar->pmtu-32<<"\n";
			if (state->nagleEnabled && osb>0 && (uint32)scount<pathVar->pmtu-32)
			{
				scount = 0;
				break;
			}
			datMsg=dequeueOutboundDataMsg(pathVar->pmtu-sctpmsg->getBitLength()/8 -20);
			sctpEV3<<"sendAll::sctpmsg->length="<<sctpmsg->getBitLength()/8<<" length datMsg="<<pathVar->pmtu-sctpmsg->getBitLength()/8 -20<<"\n";
			if (datMsg)
			{
				state->queuedMessages--;
				if (state->queueLimit>0 && state->queuedMessages < state->queueLimit && state->queueUpdate==false)
				{
					sendIndicationToApp(SCTP_I_SEND_MSG);
					state->queueUpdate = true;
					sctpEV3 << "\nQueue has to be filled! Queued Messages = "<<state->queuedMessages<<"\n";
				}
				datVar=new SCTPDataVariables();
				datVar->enqueuingTime = datMsg->getEnqueuingTime();
				datVar->expiryTime = datMsg->getExpiryTime();
				datVar->ppid = datMsg->getPpid();
				datMsg->setInitialDestination(state->primaryPathIndex);//I.R.always send on primaryPath
				datVar->initialDestination = datMsg->getInitialDestination();
				dpi = datVar->initialDestination;  //I.R. 23.10.07
				sctpEV3<<"initialDestination="<<datVar->initialDestination<<": data will be sent to "<<dpi<<"\n";
				// datVar->len just data
				datVar->len = datMsg->getBitLength();
				datVar->sid = datMsg->getSid();
				datVar->allowedNoRetransmissions = datMsg->getRtx();
				datVar->booksize = datMsg->getBooksize();
				SCTPSendStreamMap::iterator iter=this->sendStreams.find(datMsg->getSid());
				SCTPSendStream* stream=iter->second;
				nextSsn = stream->getNextStreamSeqNum();
				datVar->userData = datMsg->decapsulate();
				if (datMsg->getOrdered())
				{
					datVar->ssn=nextSsn++;
					datVar->ordered = true;
					if (nextSsn > 65535)
						stream->setNextStreamSeqNum(0);
					else
						stream->setNextStreamSeqNum(nextSsn);
				}
				else
				{
					datVar->ssn=0;
					datVar->ordered = false;
				}
				firstTime=true;
				delete datMsg;

				sctpEV3<<"chunk "<<datVar<<" dequeued from StreamQ "<<datVar->sid<<" tsn="<<datVar->tsn<<" bytes now "<<qCounter.roomSumSendStreams<<"\n";
			}
			else
			{
				if (headerCreated==true)
				{
					if (chunksAdded==0)
					{
						delete sctpmsg;
						return;
					}
					else
					{
						packetFull = true;
						sctpEV3<<"packetFull: msg length = "<<sctpmsg->getBitLength()/8+20<<"\n";
						datVar=NULL;
					}
				}
			}
		}
		chunk=datVar;
		if (chunk!=NULL)
		{
			if (chunk->tsn == 0)
			{
				chunk->tsn = state->nextTSN;
				sctpEV3<<"Set TSN="<<chunk->tsn<<" sid="<<chunk->sid<<", ssn="<<chunk->ssn<<"\n";
				state->nextTSN++;
			}
			pathVar->pathTSN->record(chunk->tsn);
			SCTP::AssocStatMap::iterator iter=sctpMain->assocStatMap.find(assocId);
			iter->second.transmittedBytes+=chunk->len/8;
			/* only if it is a first time send event, store the chunk also in the retransmission queue */
			chunk->countsAsOutstanding = true;
			chunk->hasBeenRemoved = false;
			chunk->sendTime = simulation.getSimTime(); //I.R. to send Fast RTX just once a RTT
			if (chunk->numberOfTransmissions == 0)
			{
				chunk->lastDestination = state->primaryPathIndex;
				sctpEV3<<simulation.getSimTime()<<" firstTime, TSN "<<chunk->tsn<<": lastDestination set to "<<chunk->lastDestination<<"\n";
				if (chunk->lastDestination==IPvXAddress("0.0.0.0"))
					opp_error("primaryPathIndex unspecified");
				if (!state->firstDataSent)
					state->firstDataSent=true;
				sctpEV3<<"insert in retransmissionQ tsn="<<chunk->tsn<<"\n";
				if(!retransmissionQ->checkAndInsertVar(chunk->tsn, chunk))
				{
					sctpEV3<<"Cannot add message/chunk retransmission Q buffer (tsn="<<chunk->tsn<<")...terminating!\n";
				}
				else
				{
					sctpEV3<<"size of retransmissionQ="<<retransmissionQ->getQueueSize()<<"\n";
					chunk->hasBeenAcked = false;
					pathVar->outstandingBytes+=chunk->booksize;
					sctpEV3<<"osb="<<pathVar->outstandingBytes<<"\n";
					CounterMap::iterator q=qCounter.roomRetransQ.find(dpi);
					q->second+= ADD_PADDING(chunk->len/8 + SCTP_DATA_CHUNK_LENGTH);
				}
			}
			else
				sctpEV3<<"numberOfTransmissions="<<chunk->numberOfTransmissions<<"\n";

				/* chunk is already in the retransmissionQ */
			chunk->numberOfTransmissions++;

			chunk->gapReports = 0;
			chunk->hasBeenFastRetransmitted = false;
			sctpEV3<<"sendAll(): adding new outbound data chunk to packet (tsn="<<chunk->tsn<<")...!!!\n";

			chunkPtr = transformDataChunk(chunk);

			/* update counters */
			totalChunksSent++;
			chunksAdded++;
			dataChunksAdded++;
			sctpmsg->addChunk(chunkPtr);

			if (chunk->booksize > state->peerRwnd)
			{
				state->peerRwnd = 0;
				bytesToSend = 0;
				bytes.packet = false;
				packetFull = true;
			}
			else
			{

				state->peerRwnd-=chunk->booksize;
				if (bytes.chunk==false && bytes.packet==false)
					bytesToSend -= chunk->booksize;
				else if (bytes.chunk)
					bytes.chunk = false;
				else if (bytes.packet && packetFull)
					bytes.packet = false;
			}

			if (bytesToSend <= 0)
			{
				if (!packetFull && qCounter.roomSumSendStreams>pathVar->pmtu-32)
				{
					sendOneMorePacket = true;
					bytes.packet = true;
					sctpEV3<<"one more packet allowed\n";
				}
				else
				{
					bytesToSend = 0;
					packetFull = true;
				}
			}
			else if (qCounter.roomSumSendStreams==0 && tq->second==0)
			{
				packetFull = true;
				sctpEV3<<"no data in send and transQ: packet full\n";
			}
			sctpEV3<<"bytesToSend after reduction: "<<bytesToSend<<"\n";

			chunk->lastDestination = newDpi;
			sctpEV3<<simulation.getSimTime()<<" TSN "<<chunk->tsn<<": lastDestination set to "<<chunk->lastDestination<<"\n";
			if (chunk->lastDestination==IPvXAddress("0.0.0.0"))
					opp_error("newDpi unspecified");
		}
		else if (headerCreated)
		{
			if (chunksAdded==0)
			{
				delete sctpmsg;
				return;
			}
			else
			{
				packetFull = true;
				sctpEV3<<"packetFull: msg length = "<<sctpmsg->getBitLength()/8+20<<"\n";
				datVar=NULL;
			}
		}


		if (packetFull || (dpi != newDpi) || sendBeforeRtx)
		{

			sctpEV3<<simulation.getSimTime()<<"  packet full: totalLength="<<sctpmsg->getBitLength()/8 + 20<<", dpi="<<dpi<<", newDpi="<<newDpi<<" "<<dataChunksAdded<<" chunks added, osb now "<<getPath(dpi)->outstandingBytes<<"\n";

			if (headerCreated == true && chunksAdded > 0)
			{
				/* new chunks would exceed MTU, so we send old packet and build a new one */
				/* this implies that at least one data chunk is send here */
				if (dataChunksAdded > 0)
				{
					if (!pathVar->T3_RtxTimer->isScheduled())
					{
						startTimer(pathVar->T3_RtxTimer, pathVar->pathRto);
					}
					else
						sctpEV3<<"RTX Timer already scheduled\n";
				}
				if (sendOneMorePacket)
				{
					sendOneMorePacket = false;
					bytesToSend = 0;
					bytes.packet = false;
				}
				sendToIP(sctpmsg, dpi);
				pmDataIsSentOn(dpi);
				totalPacketsSent++;
				headerCreated = false;
				authAdded = false;
				chunksAdded = 0;
				dataChunksAdded = 0;
				firstTime = false;
				sendBeforeRtx = false;
				packetFull = false;

				sctpEV3<<"sendAll: Sending Packet to path "<<dpi<<" scount="<<scount<<"  tcount="<<tcount<<" bytesToSend="<<bytesToSend<<"\n";
				if (tcount==0 && bytesToSend>0 && state->nagleEnabled && state->peerRwnd < pathVar->pmtu && pathVar->outstandingBytes > 0)
				{
				// Test whether a full packet can be filled
				sctpEV3<<" peerRwnd="<<state->peerRwnd<<" bytesToSend="<<bytesToSend<<" nextTSN="<<state->nextTSN<<"\n";
					SCTPDataMsg *datMsg = peekOutboundDataMsg();
					if (datMsg)
					{
						uint32 ums = datMsg->getBooksize();
						uint32 num =(uint32)((pathVar->pmtu-32)/(ums+SCTP_DATA_CHUNK_LENGTH));
						if (num*ums>state->peerRwnd)
							bytesToSend = 0;
						else
							bytesToSend = num * ums;
					}
					else
						bytesToSend = 0;
					sctpEV3<<"bytesToSend set to "<<bytesToSend<<"\n";
				}
			}
			else
			{

				sctpEV3<<"Logic Error: Packet Size + new chunk len exceed path mtu, but chunksAdded==0\n";

			}

		}


		sctpEV3<<"still "<<bytesToSend<<" bytes to send, headerCreated="<<headerCreated<<"\n";
			/* else { */ /* place left in the MTU, so add chunk to packet...and go on */
		if (!headerCreated && bytesToSend > 0)
		{
			sctpEV3<<"headerCreated="<<headerCreated<<"  bytesToSend="<<bytesToSend<<"  sumSendStreams="<<qCounter.roomSumSendStreams<<"\n";
			if ((state->nagleEnabled==false && (qCounter.roomSumSendStreams>0 || tq->second>0)) || (state->nagleEnabled==true && (qCounter.roomSumSendStreams>= (pathVar->pmtu - 32) || tq->second>0)))
			{
				sctpmsg = new SCTPMessage();
				headerCreated=true;
				sctpmsg->setBitLength(SCTP_COMMON_HEADER*8);
				tcount = tq->second;
				rtcount = rtq->second;
				scount = qCounter.roomSumSendStreams;
				bcount = qCounter.bookedSumSendStreams;
			}
			else
				bytesToSend=0;
		}

	}

		//sctpEV3<<"end of while\n";
		/* =============================================================  */


	if (chunksAdded > 0 && headerCreated == true)
	{
		sctpEV3<<"out of while: headerCreated="<<headerCreated<<"  chunksAdded="<<chunksAdded<<" dataChunksAdded="<<dataChunksAdded<<"\n";
		if (state->nagleEnabled==false || osb==0 || firstTime == false || (state->nagleEnabled && qCounter.roomSumSendStreams>= (pathVar->pmtu - 32)) || rtxActive == true)
		{
			/* this destination may have to be changed (e.g. to last fromAddress) */
			if (dpi == IPvXAddress("0.0.0.0"))
				dpi = state->primaryPathIndex;
			/* start T3 timer, if it is not already running, and turn off probing */

			if (dataChunksAdded > 0)
			{
				if (!getPath(dpi)->T3_RtxTimer->isScheduled())
				{

					startTimer(getPath(dpi)->T3_RtxTimer, getPath(dpi)->pathRto);
				}
			}
		}
		sctpEV3<<"send all queued chunks to "<<dpi<<" and get out. osb now "<<getPath(dpi)->outstandingBytes<<"\n";
		sendToIP(sctpmsg, dpi);
		authAdded = false;
		pmDataIsSentOn(dpi);
		totalPacketsSent++;
		rtxActive=false;
		probing = false;
	}
	else
	{

		sctpEV3<<"sendAll: nothing more to send...BYE !\n";

	}

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


void SCTPAssociation::putInDeliveryQ(uint16 sid)
{
	SCTPReceiveStream* rStream;
	SCTPDataVariables* chunk;
	SCTPReceiveStreamMap::iterator iter=receiveStreams.find(sid);
	rStream = iter->second;
	sctpEV3<<"putInDeliveryQ: first look for SSN "<<rStream->getExpectedStreamSeqNum()<<" of SID "<<sid<<" in orderedQ of size "<<rStream->getOrderedQ()->getQueueSize()<<"\n";
	while (rStream->getOrderedQ()->getQueueSize()>0)
	{
		/* dequeue first from reassembly Q */
		chunk = rStream->getOrderedQ()-> dequeueVarBySsn(rStream->getExpectedStreamSeqNum());
		if (chunk)
		{
			sctpEV3<<"putInDeliveryQ::chunk "<<chunk->tsn<<" , sid "<<chunk->sid<<" and ssn "<<chunk->ssn<<" dequeued from ordered queue. queuedRcvBytes="<<state->queuedRcvBytes<<" will be reduced by "<<chunk->len/8<<"\n";
			state->queuedRcvBytes-=chunk->len/8;


			qCounter.roomSumRcvStreams -= ADD_PADDING(chunk->len/8 + SCTP_DATA_CHUNK_LENGTH);
			if (rStream->getDeliveryQ()->checkAndInsertVar(chunk->tsn, chunk))
			{

				state->queuedRcvBytes+=chunk->len/8;

				sctpEV3<<"data put in deliveryQ; queuedBytes now "<<state->queuedRcvBytes<<"\n";
				qCounter.roomSumRcvStreams += ADD_PADDING(chunk->len/8 + SCTP_DATA_CHUNK_LENGTH);
				int32 seqnum=rStream->getExpectedStreamSeqNum();
				rStream->setExpectedStreamSeqNum(++seqnum);
				if (rStream->getExpectedStreamSeqNum() > 65535)
					rStream->setExpectedStreamSeqNum(0);
				sendDataArrivedNotification(sid);
			}
		}
		else
		{
			break;
		}
	}
}

void SCTPAssociation::pushUlp()
{
	int32 count=0;
	SCTPReceiveStream* rStream;
	SCTPDataVariables* chunk;
	for (unsigned i=0; i<inboundStreams; i++)	//12.06.08
		putInDeliveryQ(i);
	if (state->pushMessagesLeft <=0)
		state->pushMessagesLeft = state->messagesToPush;
	bool restrict = false;

	if (state->pushMessagesLeft > 0)
		restrict = true;

	quBytes->record(state->queuedRcvBytes);

	sctpEV3<<simulation.getSimTime()<<" Calling PUSH_ULP ("<<state->queuedRcvBytes<<" bytes queued)...\n";
	uint32 i=state->nextRSid;
	do
	{
		SCTPReceiveStreamMap::iterator iter=receiveStreams.find(i);
		rStream=iter->second;
		sctpEV3<<"Size of stream "<<iter->first<<": "<<rStream->getDeliveryQ()->getQueueSize()<<"\n";

		while (!rStream->getDeliveryQ()->payloadQueue.empty() && (!restrict || (restrict && state->pushMessagesLeft>0)))
		{
			chunk = rStream->getDeliveryQ()->extractMessage();
			qCounter.roomSumRcvStreams -= ADD_PADDING(chunk->len/8 + SCTP_DATA_CHUNK_LENGTH);

			if (state->pushMessagesLeft >0)
				state->pushMessagesLeft--;

			state->queuedRcvBytes-=chunk->len/8;

			if (state->swsAvoidanceInvoked)
			{
				quBytes->record(state->queuedRcvBytes);
				if ((int32)(state->localRwnd - state->queuedRcvBytes) >= (int32)(state->swsLimit) && (int32)(state->localRwnd - state->queuedRcvBytes) <= (int32)(state->swsLimit+state->assocPmtu))

				{
					/* only if the window has opened up more than one MTU we will send a SACK */
					state->swsAvoidanceInvoked = false;
					sctpEV3<<"pushUlp: Window opens up to "<<(int32)state->localRwnd-state->queuedRcvBytes<<" bytes: sending a SACK. SWS Avoidance INACTIVE\n";

					sendSack();
				}
			}
			sctpEV3<<"Push TSN "<<chunk->tsn<<": sid="<<chunk->sid<<" ssn="<<chunk->ssn<<"\n";
			cPacket* msg= (cPacket *)chunk->userData;
			msg->setKind(SCTP_I_DATA);
			SCTPCommand *cmd = new SCTPCommand("push");
			cmd->setAssocId(assocId);
			cmd->setGate(appGateIndex);
			cmd->setSid(chunk->sid);
			cmd->setLocalAddr(localAddr);
			cmd->setRemoteAddr(remoteAddr);
			msg->setControlInfo(cmd);
			state->numMsgsReq[count]--;
			delete chunk;
			sendToApp(msg);
		}

		i = (i+1)%inboundStreams;

		count++;
	} while (i!=state->nextRSid);

	state->nextRSid = (state->nextRSid+1)%inboundStreams;

	if (state->queuedRcvBytes==0 && fsm->getState()==SCTP_S_SHUTDOWN_ACK_SENT)
{
		sctpEV3<<"SCTP_E_CLOSE\n";
		performStateTransition(SCTP_E_CLOSE);
}
}

SCTPDataChunk* SCTPAssociation::transformDataChunk(SCTPDataVariables* datVar)
{

	SCTPDataChunk* dataChunk=new SCTPDataChunk("DATA");
	SCTPSimpleMessage* msg=check_and_cast<SCTPSimpleMessage*>(datVar->userData->dup());
	dataChunk->setChunkType(DATA);
	// no fragmentation implemented yet
	dataChunk->setBBit(1);
	dataChunk->setEBit(1);
	if (datVar->ordered)
		dataChunk->setUBit(0);
	else
		dataChunk->setUBit(1);
	dataChunk->setTsn(datVar->tsn);
	dataChunk->setSid(datVar->sid);
	dataChunk->setSsn(datVar->ssn);
	dataChunk->setPpid(datVar->ppid);
	dataChunk->setEnqueuingTime(datVar->enqueuingTime);
	dataChunk->setBitLength(SCTP_DATA_CHUNK_LENGTH*8);
	msg->setBitLength(datVar->len);
	dataChunk->encapsulate(msg);
	return dataChunk;
}


void SCTPAssociation::addPath(IPvXAddress addr)
{

	sctpEV3<<"Add Path remote address: "<<addr<<"\n";

	SCTPPathMap::iterator i = sctpPathMap.find(addr);
	if (i==sctpPathMap.end())
	{
		SCTPPathVariables* path = new SCTPPathVariables(addr, this);
		sctpPathMap[addr] = path;
		qCounter.roomTransQ[addr] = 0;
		qCounter.roomRetransQ[addr] = 0;
		qCounter.bookedTransQ[addr] = 0;
	}
}

void SCTPAssociation::removePath(IPvXAddress addr)
{
	SCTPPathMap::iterator j = sctpPathMap.find(addr);
	if (j!=sctpPathMap.end())
	{
		stopTimer(j->second->HeartbeatTimer);
		delete j->second->HeartbeatTimer;
		stopTimer(j->second->HeartbeatIntervalTimer);
		delete j->second->HeartbeatIntervalTimer;
		stopTimer(j->second->T3_RtxTimer);
		delete j->second->T3_RtxTimer;
		stopTimer(j->second->CwndTimer);
		delete j->second->CwndTimer;
		sctpPathMap.erase(j);
		delete j->second;
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

void SCTPAssociation::removeLastPath(IPvXAddress addr)
{
	SCTPPathVariables* path = getPath(addr);
	stopTimer(path->HeartbeatTimer);
	delete path->HeartbeatTimer;
	stopTimer(path->HeartbeatIntervalTimer);
	delete path->HeartbeatIntervalTimer;
	stopTimer(path->T3_RtxTimer);
	delete path->T3_RtxTimer;
	stopTimer(path->CwndTimer);
	delete path->CwndTimer;
}

bool SCTPAssociation::makeRoomForTsn(uint32 tsn, uint32 length, bool uBit)
{
SCTPDataVariables* datVar=NULL;
SCTPQueue* stream, dStream;
uint32 sum=0, comp=0;
bool delQ = false;
uint32 high = state->highestTsnStored;
sctpEV3<<simulation.getSimTime()<<": makeRoomForTsn: tsn="<<tsn<<", length="<<length<<" high="<<high<<"\n";
	while (sum<length && state->highestTsnReceived>state->lastTsnAck)
	{
		comp = sum;
		for (SCTPReceiveStreamMap::iterator iter=receiveStreams.begin();iter!=receiveStreams.end(); iter++)
		{
			if (tsn > high)
				return false;
			if (uBit)
				stream = iter->second->getUnorderedQ();
			else
				stream = iter->second->getOrderedQ();
			datVar = stream->getVar(high);
			if (datVar==NULL)	//12.06.08
			{
				sctpEV3<<high<<" not found in orderedQ. Try deliveryQ\n";
				stream = iter->second->getDeliveryQ();
				datVar = stream->getVar(high);
				delQ = true;
			}
			if (datVar!=NULL)
			{

				sum+=datVar->len;
				if (stream->deleteMsg(high))
				{
				sctpEV3<<high<<" found and deleted\n";

				state->queuedRcvBytes-=datVar->len/8; //12.06.08
				if (ssnGt(iter->second->getExpectedStreamSeqNum(),datVar->ssn))
					iter->second->setExpectedStreamSeqNum(datVar->ssn);
				}
				qCounter.roomSumRcvStreams -= ADD_PADDING(datVar->len/8 + SCTP_DATA_CHUNK_LENGTH);
				if (high == state->highestTsnReceived)
					state->highestTsnReceived--;
				removeFromGapList(high);

				if (tsn > state->highestTsnReceived)
					state->highestTsnReceived=tsn;
				high--;
				break;
			}
			else
			{

				sctpEV3<<"TSN "<<high<<" not found in stream "<<iter->second->getStreamId()<<"\n";

			}
		}

		if (comp == sum)
		{
			//removeFromGapList(high);
			sctpEV3<<high<<" not found in any stream\n";
			high--;
		}
		state->highestTsnStored = high;

		if (tsn > state->highestTsnReceived)
			return false;
	}
	return true;
}

bool SCTPAssociation::tsnIsDuplicate(uint32 tsn)
{
	for (std::list<uint32>::iterator it=state->dupList.begin(); it!=state->dupList.end(); it++)
	{
		if ((*it)==tsn)
			return true;
	}
	for (uint32 i=0; i<state->numGaps; i++)
		if (tsnBetween(state->gapStartList[i], tsn, state->gapStopList[i]))
			return true;
	return false;
}

void SCTPAssociation::removeFromGapList(uint32 removedTsn)
{
int32 gapsize, numgaps;

	numgaps = state->numGaps;
	sctpEV3<<"remove TSN "<<removedTsn<<" from GapList. "<<numgaps<<" gaps present, cumTsnAck="<<state->cTsnAck<<"\n";
	for (int32 j=0; j<numgaps; j++)
		sctpEV3<<state->gapStartList[j]<<" - "<<state->gapStopList[j]<<"\n";
	for (int32 i=numgaps-1; i>=0; i--)
	{
		sctpEV3<<"gapStartList["<<i<<"]="<<state->gapStartList[i]<<", state->gapStopList["<<i<<"]="<<state->gapStopList[i]<<"\n";
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
					state->numGaps++;
				}
			}
			else
			{

				for (int32 j=i; j<=numgaps-1; j++)
				{
					state->gapStopList[j] = state->gapStopList[j+1];
					state->gapStartList[j] = state->gapStartList[j+1];
				}
				state->gapStartList[numgaps-1]=0;
				state->gapStopList[numgaps-1]=0;
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

bool SCTPAssociation::updateGapList(uint32 receivedTsn)
{
	uint32 lo, hi, gapsize;


	sctpEV3<<"Entering updateGapList(tsn=="<< receivedTsn<<" cTsnAck="<<state->cTsnAck<<" Number of Gaps="<<state->numGaps<<"\n";

	lo = state->cTsnAck + 1;
	if ((int32)(state->localRwnd-state->queuedRcvBytes)<=0)
	{
		sctpEV3<<"window full\n";
		//only check if cumTsnAck can be advanced
		if (receivedTsn == lo)
		{
			sctpEV3<<"Window full, but cumTsnAck can be advanced:"<<lo<<"\n";

		}
		else
			return false;
	}

	if (tsnGt(receivedTsn, state->highestTsnStored))	//17.06.08
		state->highestTsnStored = receivedTsn;

	for (uint32 i=0; i<state->numGaps; i++)
	{
		if (state->gapStartList[i]>0)
		{
			hi = state->gapStartList[i] - 1;
			if (tsnBetween(lo, receivedTsn, hi))
			{
				gapsize = hi - lo + 1;
				if (gapsize > 1)
				{
					/**
					* TSN either sits at the end of one gap, and thus changes gap
					* boundaries, or it is in between two gaps, and becomes a new gap
					*/
					if (receivedTsn == hi)
					{

						sctpEV3<<"receivedTsn==hi....";

						state->gapStartList[i] = receivedTsn;
						state->newChunkReceived = true;
						return true;
					}
					else if (receivedTsn == lo)
					{

						sctpEV3<<"receivedTsn==lo=="<<lo<<"....."<< lo<<"\n";

						if (receivedTsn == (state->cTsnAck + 1))
						{
							state->cTsnAck++;
							state->newChunkReceived = true;
							return true;
						}
						/* some gap must increase its upper bound */
						state->gapStopList[i-1] = receivedTsn;
						state->newChunkReceived = true;
						return true;
					}
					else
					{    /* a gap in between */
						state->numGaps++;

						sctpEV3<<"Inserting new gap (start==stop=="<<receivedTsn<<"\n";

						for (uint32 j=state->numGaps; j>i; j--)
						{
							state->gapStartList[j] = state->gapStartList[j-1];
							state->gapStopList[j] = state->gapStopList[j-1];
						}
						state->gapStartList[i]=receivedTsn;
						state->gapStopList[i]=receivedTsn;
						state->newChunkReceived = true;
						for (uint32 k=0; k<state->numGaps; k++)

							sctpEV3<<"Gap "<<k<<": "<<state->gapStartList[k]<<" - "<<state->gapStopList[k]<<"\n";

						return true;
					}
				}
				else
				{ /* alright: gapsize is 1: our received tsn may close gap between fragments */

					sctpEV3<<"gapsize="<<gapsize<<"\n";

					if (lo == state->cTsnAck + 1)
					{
						state->cTsnAck = state->gapStopList[i];
						if (i==state->numGaps-1)
						{
							state->gapStartList[i] = 0;
							state->gapStopList[i] = 0;
						}
						else
							for (uint32 j=i; j<state->numGaps; j++)
							{
								state->gapStartList[j] = state->gapStartList[j+1];
								state->gapStopList[j] = state->gapStopList[j+1];
							}
						state->numGaps--;

						sctpEV3<<"state->numGaps="<<state->numGaps<<"\n";

						state->newChunkReceived = true;
						return true;
					}
					else
					{
						state->gapStopList[i-1] = state->gapStopList[i];
						if (i==state->numGaps-1)
						{
							state->gapStartList[i] = 0;
							state->gapStopList[i] = 0;
						}
						else
							for (uint32 j=i; j<=state->numGaps; j++)
							{
								state->gapStartList[j] = state->gapStartList[j+1];
								state->gapStopList[j] = state->gapStopList[j+1];
							}
						state->numGaps--;
						state->newChunkReceived = true;
						return true;
					}

				}

			}
			else
			{  /* receivedTsn is not in the gap between these fragments... */
				lo = state->gapStopList[i] + 1;
			}
		} /* end: for */
	}/* end: for */

	/* (NULL LIST)  OR  (End of Gap List passed) */
	if (receivedTsn == lo)
	{
		/* just increase ctsna, handle further update of ctsna later */
		if (receivedTsn == state->cTsnAck + 1)
		{

			sctpEV3<<"Updating cTsnAck....now cTsnAck=="<<receivedTsn<<"\n";

			state->cTsnAck = receivedTsn;
			state->newChunkReceived = true;
			return true;
		}
		/* Update last fragment....increase stop_tsn by one */
		state->gapStopList[state->numGaps-1]++;

		state->newChunkReceived = true;
		return true;

	}
	else
	{                    /* a new fragment altogether, past the end of the list */

		sctpEV3<<"Inserting new fragment after the end of the list (start==stop=="<<receivedTsn<<"\n";

		state->gapStartList[state->numGaps] = receivedTsn;
		state->gapStopList[state->numGaps] = receivedTsn;
		state->numGaps++;
		state->newChunkReceived = true;
		for (uint32 k=0; k<state->numGaps; k++)

			sctpEV3<<"Gap "<<k<<": "<<state->gapStartList[k]<<" - "<<state->gapStopList[k]<<"\n";

		return true;
	}

	return false;

}

bool SCTPAssociation::advanceCtsna(void)
{
int32 listLength, counter;

	ev<<"Entering advanceCtsna(ctsna now =="<< state->cTsnAck<<"\n";;

	listLength = state->numGaps;

	/* if there are no fragments, we cannot advance the ctsna */
	if (listLength == 0) return false;
	counter = 0;

	while(counter < listLength)
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
			ev<<"Entering advanceCtsna(when leaving: ctsna=="<<state->cTsnAck<<"\n";
			return false;
		}

	}	/* end while */

	ev<<"Entering advanceCtsna(when leaving: ctsna=="<< state->cTsnAck<<"\n";
	return true;
}

SCTPDataVariables* SCTPAssociation::makeVarFromMsg(SCTPDataChunk* dataChunk)
{
SCTPDataVariables* datVar=new SCTPDataVariables();

	datVar->sid = dataChunk->getSid();
	datVar->ssn = dataChunk->getSsn();
	datVar->ppid = dataChunk->getPpid();
	datVar->tsn = dataChunk->getTsn();
	if (!dataChunk->getUBit())
		datVar->ordered = true;
	else
		datVar->ordered = false;
	SCTPSimpleMessage* smsg=check_and_cast<SCTPSimpleMessage*>(dataChunk->decapsulate());

	datVar->userData = smsg;
		datVar->len = smsg->getDataLen()*8;
	sctpEV3<<"Datenlaenge="<<datVar->len/8<<"\n";


	sctpEV3<<"makeVarFromMsg::queuedBytes have been increased to "<<state->queuedRcvBytes<<"\n";
	return datVar;
}




SCTPDataVariables* SCTPAssociation::getOutboundDataChunk(IPvXAddress pid, int32 bytes)
{
int32 len;
	/* are there chunks in the transmission queue ? If Yes -> dequeue and return it */
	sctpEV3<<"getOutboundDataChunk: pid="<<pid<<" bytes="<<bytes<<"\n";
	sctpEV3<<"chunks in transmissionQ="<<transmissionQ->getQueueSize()<<"\n";
	if (!transmissionQ->payloadQueue.empty())
	{
		sctpEV3<<"transmissionQ not empty\n";
		for(SCTPQueue::PayloadQueue::iterator it=transmissionQ->payloadQueue.begin(); it!=transmissionQ->payloadQueue.end(); it++)
		{
			SCTPDataVariables* chunk=it->second;
			sctpEV3<<"chunk->nextDestination="<<chunk->nextDestination<<"\n";
			if (chunk->nextDestination == pid)
			{
				len = ADD_PADDING(chunk->len/8+SCTP_DATA_CHUNK_LENGTH);
				sctpEV3<<"getOutboundDataChunk() found chunk "<<chunk->tsn<<" in the transmission queue, length="<<len<<"\n";
				if (len<=bytes)
				{
					transmissionQ->payloadQueue.erase(it);
					CounterMap::iterator i=qCounter.roomTransQ.find(pid);
					i->second-= ADD_PADDING(chunk->len/8+SCTP_DATA_CHUNK_LENGTH);
					CounterMap::iterator ib=qCounter.bookedTransQ.find(pid);
					ib->second-= chunk->booksize;
					if (chunk->hasBeenAcked==false)
						return chunk;
				}
			}
		}
	}
	else
		sctpEV3<<"transmissionQ empty!\n";
	return NULL;
}

SCTPDataVariables* SCTPAssociation::peekOutboundDataChunk(IPvXAddress pid)
{
	SCTPDataVariables* chunk = NULL;

	/* are there chunks in the transmission queue ? If Yes -> dequeue and return it */
	if (!transmissionQ->payloadQueue.empty())
	{
		for(SCTPQueue::PayloadQueue::iterator it=transmissionQ->payloadQueue.begin(); it!=transmissionQ->payloadQueue.end(); it++)
		{
			chunk = it->second;
			if (chunk->nextDestination == pid)
			{
				sctpEV3<<"peekOutboundDataChunk() found chunk "<<chunk->tsn<<" in the transmission queue\n";
				if (chunk->hasBeenAcked)
				{
					transmissionQ->payloadQueue.erase(it);
					CounterMap::iterator i=qCounter.roomTransQ.find(pid);
					i->second-= ADD_PADDING(chunk->len/8+SCTP_DATA_CHUNK_LENGTH);
					CounterMap::iterator ib=qCounter.bookedTransQ.find(pid);
					ib->second-= chunk->booksize;
				}
				else
				{
						return chunk;
				}
			}
		}
	}

	return NULL;
}

SCTPDataVariables* SCTPAssociation::peekAbandonedChunk(IPvXAddress pid)
{
	SCTPDataVariables* chunk = NULL;

	/* are there chunks in the retransmission queue ? If Yes -> dequeue and return it */
	if (!retransmissionQ->payloadQueue.empty())
	{
		for(SCTPQueue::PayloadQueue::iterator it=retransmissionQ->payloadQueue.begin(); it!=retransmissionQ->payloadQueue.end(); it++)
		{
			chunk=it->second;
			sctpEV3<<"peek Chunk "<<chunk->tsn<<"\n";
			if (chunk->lastDestination == pid && chunk->hasBeenAbandoned)
			{
				sctpEV3<<"peekAbandonedChunk() found chunk in the retransmission queue\n";
				return chunk;
			}
		}
	}

	return NULL;
}


SCTPDataMsg* SCTPAssociation::peekOutboundDataMsg(void)
{
	SCTPDataMsg* datMsg=NULL;
	int32 nextStream = -1;
	nextStream = (this->*ssFunctions.ssGetNextSid)(true);

	if (nextStream == -1)
	{

		sctpEV3<<"peekOutboundDataMsg(): no valid stream found -> returning NULL !\n";

		return NULL;
	}


	for (SCTPSendStreamMap::iterator iter=sendStreams.begin(); iter!=sendStreams.end(); ++iter)
	{
		if ((int32)iter->first==nextStream)
		{
			SCTPSendStream* stream=iter->second;
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

SCTPDataMsg* SCTPAssociation::dequeueOutboundDataMsg(int32 bytes)
{
	SCTPDataMsg* datMsg=NULL;
	int32 nextStream = -1;

	sctpEV3<<"dequeueOutboundDataMsg: "<<bytes<<" bytes left to be sent\n";
	nextStream = (this->*ssFunctions.ssGetNextSid)(false);

	if (nextStream == -1)
		return NULL;

	for (SCTPSendStreamMap::iterator iter=sendStreams.begin(); iter!=sendStreams.end(); ++iter)
	{
		if ((int32)iter->first==nextStream)
		{
			SCTPSendStream* stream=iter->second;
			if (!stream->getUnorderedStreamQ()->empty())
			{
				datMsg = (SCTPDataMsg*)stream->getUnorderedStreamQ()->pop();

				sctpEV3<<"DequeueOutboundDataMsg() found chunk ("<<&datMsg<<")  in the unordered stream queue "<<&iter->first<<"("<<stream->getUnorderedStreamQ()<<") queue size="<<stream->getUnorderedStreamQ()->getLength()<<"\n";
			}
			else if (!stream->getStreamQ()->empty())
			{

				int32 b=ADD_PADDING( (check_and_cast<SCTPSimpleMessage*>(((SCTPDataMsg*)stream->getStreamQ()->back())->getEncapsulatedMsg())->getBitLength()/8+SCTP_DATA_CHUNK_LENGTH));
				 if (b<=bytes)
				 {
				 	datMsg = (SCTPDataMsg*)stream->getStreamQ()->pop();
					if (!state->appSendAllowed && stream->getStreamQ()->getLength()<=state->sendQueueLimit)
					{
						state->appSendAllowed = true;
						sendIndicationToApp(SCTP_I_SENDQUEUE_ABATED);
					}
					sendQueue->record(stream->getStreamQ()->getLength());
				sctpEV3<<"DequeueOutboundDataMsg() found chunk ("<<&datMsg<<")  in the stream queue "<<&iter->first<<"("<<stream->getStreamQ()<<") queue size="<<stream->getStreamQ()->getLength()<<"\n";
				}
				else
					sctpEV3<<"chunk too long\n";
			}
			break;
		}
	}
	if (datMsg != NULL)
	{
		qCounter.roomSumSendStreams -= ADD_PADDING( (check_and_cast<SCTPSimpleMessage*>(datMsg->getEncapsulatedMsg())->getBitLength()/8+SCTP_DATA_CHUNK_LENGTH));
		qCounter.bookedSumSendStreams -= datMsg->getBooksize();
	}
	return (datMsg);
}



IPvXAddress SCTPAssociation::getNextAddress(IPvXAddress pid)
{

	int32 hit=0;

	if (sctpPathMap.size()>1)
	{
		for (SCTPPathMap::iterator iter=sctpPathMap.begin(); iter!=sctpPathMap.end(); iter++)
		{
			if (iter->first==pid)
			{
				if (++hit==1)
					continue;
				else
					break;
			}
			if (iter->second->activePath)
				return iter->first;
		}
	}
	return IPvXAddress("0.0.0.0");
}

IPvXAddress SCTPAssociation::getNextDestination(SCTPDataVariables* chk)
{
	IPvXAddress dpi, last;


	sctpEV3<<"getNextDestination\n";

	if (chk->numberOfTransmissions == 0)
	{
		if (chk->initialDestination == IPvXAddress("0.0.0.0"))
		{
			dpi = state->primaryPathIndex;
		}
		else
		{
			dpi = chk->initialDestination;
		}
	}
	else
	{
		if (chk->hasBeenFastRetransmitted)
		{

			sctpEV3<<"Chunk is scheduled for FastRetransmission. Next destination = "<<chk->lastDestination<<"\n";

			//chk->hasBeenFastRetransmitted=false;
			return chk->lastDestination;
		}
		dpi = last = chk->lastDestination;
		/* if this is a retransmission, we should choose another, active path */
		SCTPPathMap::iterator iter=sctpPathMap.find(dpi);
		do
		{

			sctpEV3<<"old dpi="<<iter->first<<"\n";

			iter++;
			if (iter==sctpPathMap.end())
				iter=sctpPathMap.begin();
			dpi=iter->first;

		} while (iter->first != last && iter->second->activePath == false || iter->second->confirmed==false);
	}


	sctpEV3<<"sctp_get_next_destination: chunk was last sent to "<<last<<", will next be sent to path "<<dpi<<"\n";

	sctpEV3<<"new dpi="<<dpi<<"\n";
	return (dpi);
}

void SCTPAssociation::bytesAllowedToSend(IPvXAddress dpi)
{
	uint32 osb = 0;
	int32 diff = 0;
	SCTPPathMap::iterator iter;
	bytes.chunk = false;
	bytes.packet = false;
	bytes.bytesToSend = 0;

	iter=sctpPathMap.find(dpi);
	osb = getPath(dpi)->outstandingBytes;
	sctpEV3<<"bytesAllowedToSend: osb="<<osb<<"  cwnd="<<iter->second->cwnd<<"\n";
	if (!state->firstDataSent)
	{
		bytes.chunk = true;
		return;
	}
	if (state->peerWindowFull)
	{
		if (osb==0)
		{
			sctpEV3<<"probingIsAllowed\n";
			state->zeroWindowProbing=true;
			bytes.chunk = true;
		}
		return;
	}
	else
	{


		sctpEV3<<"bytesAllowedToSend: osb="<<osb<<"  cwnd="<<iter->second->cwnd<<"\n";
		CounterMap::iterator it = qCounter.roomTransQ.find(dpi);
		sctpEV3<<"bytes in transQ="<<it->second<<"\n";
		if (it->second>0)
		{
			diff = iter->second->cwnd - osb;
			sctpEV3<<"cwnd-osb="<<diff<<"\n";
			if (state->peerRwnd<iter->second->pmtu)
			{
				bytes.bytesToSend = state->peerRwnd;
				return;
			}
			if (diff > 0)
			{
				CounterMap::iterator bit = qCounter.bookedTransQ.find(dpi);
				if (bit->second > (uint32)diff)
				{
					bytes.bytesToSend = diff;
					sctpEV3<<"cwnd does not allow all RTX\n";
					return;
				}
				else
				{
					bytes.bytesToSend = bit->second;
					sctpEV3<<"cwnd allows more than those "<<bytes.bytesToSend<<" bytes for retransmission\n";
				}
			}
			else // You may transmit one packet
			{
				bytes.packet = true;
				sctpEV3<<"diff<=0: retransmit one packet\n";
				return;
			}
		}

		if (!bytes.chunk && !bytes.packet)
		{
			if (osb < iter->second->cwnd && !state->peerWindowFull )
			{
				sctpEV3<<"bookedSumSendStreams="<<qCounter.bookedSumSendStreams<<" bytes.bytesToSend="<<bytes.bytesToSend<<"\n";
				diff = iter->second->cwnd - osb - bytes.bytesToSend;
				if (diff>0)
				if (qCounter.bookedSumSendStreams > (uint32)diff)
				{
					bytes.bytesToSend =  iter->second->cwnd - osb ;
					sctpEV3<<"bytesToSend are limited by cwnd: "<<bytes.bytesToSend<<"\n";
				}
				else
				{
					bytes.bytesToSend += qCounter.bookedSumSendStreams;
					sctpEV3<<"send all stored bytes: "<<bytes.bytesToSend<<"\n";
				}
			}
		}
	}

}

void SCTPAssociation::printOutstandingTsns()
{
	uint32 first=0, second = 0;

	if (retransmissionQ->payloadQueue.empty())
	{
		return;
	}
	for(SCTPQueue::PayloadQueue::iterator pl=retransmissionQ->payloadQueue.begin();pl!=retransmissionQ->payloadQueue.end();pl++)
	{
		if (pl->second->hasBeenAcked == false && pl->second->countsAsOutstanding == true)
		{
			if (pl->second->tsn > second+1)
			{
				if (second != 0)
				{
					sctpEV3<<" - "<<second<<"\t";;

				}
				first = second = pl->second->tsn;
				sctpEV3<<pl->second->tsn;
			}
			else if (pl->second->tsn == second+1)
			{
				second = pl->second->tsn;
			}
			first = pl->second->tsn;
		}
	}
	sctpEV3<<" - "<<second<<"\n";
}

uint32 SCTPAssociation::getOutstandingBytesOnPath(IPvXAddress pathId)
{
	uint32 osb = 0, queueLength = 0, first=0, second = 0;


	if (retransmissionQ->payloadQueue.empty()) return 0;

	queueLength = retransmissionQ->getQueueSize();
	sctpEV3<<"queueLength of retransmissionQ="<<queueLength<<"\n";
	sctpEV3<<"outstanding TSNs\n";
	for(SCTPQueue::PayloadQueue::iterator pl=retransmissionQ->payloadQueue.begin();pl!=retransmissionQ->payloadQueue.end();pl++)
	{
		if (pl->second->lastDestination == pathId)
		{
			if (pl->second->hasBeenAcked == false && pl->second->countsAsOutstanding == true)
			{
				//osb += pl->second->len/8;
				if (pl->second->tsn > second+1)
				{
					if (second != 0)
					{
						sctpEV3<<" - "<<second<<"\t";;

					}
					first = second = pl->second->tsn;
					sctpEV3<<pl->second->tsn;
				}
				else if (pl->second->tsn == second+1)
				{
					second = pl->second->tsn;
				}
				first = pl->second->tsn;
				osb += pl->second->booksize;
			}
		}
	}
	sctpEV3<<" - "<<second<<"\n";

	sctpEV3<<"getOutstandingBytesOnPath("<<(pathId)<<") : currently "<<osb<<" bytes outstanding\n";


	return (osb);
}

SCTPPathVariables* SCTPAssociation::getPath(IPvXAddress pid)
{
	SCTPPathMap::iterator i=sctpPathMap.find(pid);
	if (i!=sctpPathMap.end())
	{
		return i->second;
	}
	else
	{
		return NULL;
	}
}

void  SCTPAssociation::pmDataIsSentOn(IPvXAddress pathId)
{
	SCTPPathVariables* path;

	/* restart hb_timer on this path */
	path=getPath(pathId);
	path->heartbeatTimeout = path->pathRto+ (double)sctpMain->par("hbInterval");
	stopTimer(path->HeartbeatTimer);
	startTimer(path->HeartbeatTimer, path->heartbeatTimeout);
	path->cwndTimeout = path->pathRto;
	stopTimer(path->CwndTimer);
	startTimer(path->CwndTimer, path->cwndTimeout);


	sctpEV3<<"Restarting HB timer on path "<<(pathId)<<" to expire at time "<<path->heartbeatTimeout<<"\n";
	sctpEV3<<"Restarting CWND timer on path "<<(pathId)<<" to expire at time "<<path->cwndTimeout<<"\n";


	/* AJ - 02-04-2004 - added for fullfilling section 8.2 */
	state->lastUsedDataPath = pathId;
}

void SCTPAssociation::pmStartPathManagement(void)
{
	RoutingTableAccess routingTableAccess;
	SCTPPathVariables* path;
	int32 i=0;
	/* populate path structures !!! */
	/* set a high start value...this is appropriately decreased later (below) */
	state->assocPmtu = state->localRwnd;
	for(SCTPPathMap::iterator piter=sctpPathMap.begin(); piter!=sctpPathMap.end(); piter++)
	{
		path=piter->second;
		path->pathErrorCount = 0;
		InterfaceEntry *rtie = routingTableAccess.get()->getInterfaceForDestAddr(path->remoteAddress.get4());
		path->pmtu = rtie->getMTU();
		if (path->pmtu < state->assocPmtu)
		{
			state->assocPmtu = path->pmtu;
		}
		initCCParameters(path);
		path->pathRto = (double)sctpMain->par("rtoInitial");
		path->srtt = path->pathRto;
		path->rttvar = 0.0;
		/* from now on we may have one update per RTO/SRTT */
		path->updateTime = 0.0;


		path->partialBytesAcked = 0;
		path->outstandingBytes = 0;
		path->activePath = true;
		path->hbWasAcked = true;
		// Timer probably not running, but stop it anyway I.R.
		stopTimer(path->T3_RtxTimer);

		path->heartbeatTimeout= (double)sctpMain->par("hbInterval")+i*path->pathRto;
		stopTimer(path->HeartbeatTimer);
		if (path->remoteAddress == state->initialPrimaryPath && !path->confirmed)
		{
			path->confirmed = true;

		}
		sctpEV3<<getFullPath()<<" numberOfLocalAddresses="<<state->localAddresses.size()<<"\n";
		if (state->localAddresses.size()>1)
			sendHeartbeat(path, false);
		else
			sendHeartbeat(path, true);
		startTimer(path->HeartbeatTimer, path->heartbeatTimeout);
		startTimer(path->HeartbeatIntervalTimer, path->heartbeatIntervalTimeout);
		path->pathRTO->record(path->pathRto);
		i++;
	}
}


int32 SCTPAssociation::getQueuedBytes(void)
{
	int32 qb = 0;
	SCTPReceiveStream* rStream;
	SCTPQueue* sq;

    for (SCTPReceiveStreamMap::iterator iter=receiveStreams.begin(); iter!=receiveStreams.end(); iter++)
	{
		rStream = iter->second;
		sq = rStream->getOrderedQ();
		if (sq)
		{
			qb+=sq->getNumBytes();
		}
		sq = rStream->getUnorderedQ();
		if (sq)
		{
			qb+=sq->getNumBytes();
		}
		sq = rStream->getDeliveryQ();
		if (sq)
		{
			qb+=sq->getNumBytes();
		}
	}


	sctpEV3<<"getQueuedBytes : currently "<<qb<<"  bytes  buffered\n";

	return (qb);
}

int32 SCTPAssociation::getOutstandingBytes(void)
{
	int32 osb = 0;
	for (SCTPPathMap::iterator pm=sctpPathMap.begin(); pm != sctpPathMap.end(); pm++)
		osb += pm->second->outstandingBytes;
	return osb;
}

int32 SCTPAssociation::dequeueAckedChunks(uint32 tsna, IPvXAddress pathId, simtime_t* rttEstimation)
{
	int32 newlyAckedBytes = 0;
	SCTPDataVariables* chunk = NULL;
	simtime_t timeDiff = 0.0;

	/* set it ridiculously high */
	*rttEstimation = -1.0;

	/* are there chunks in the retransmission queue ? If Yes -> check for dequeue */
	while (!retransmissionQ->payloadQueue.empty())
	{

		SCTPQueue::PayloadQueue::iterator iter=retransmissionQ->payloadQueue.begin();
		chunk = iter->second;

		sctpEV3<<"dequeueAckedChunks() found chunk tsn "<<chunk->tsn<<" in the retransmissionQ\n";


		if (tsnGe(tsna,chunk->tsn))
		{
			/* dequeue chunk, cause it has been acked */
			if (transmissionQ->getVar(chunk->tsn))
			{
				sctpEV3<<"Chunk was found in transmissionQ. Remove it\n";
				transmissionQ->removeMsg(chunk->tsn);
				CounterMap::iterator q = qCounter.roomTransQ.find(chunk->nextDestination);
				q->second -= ADD_PADDING(chunk->len/8+SCTP_DATA_CHUNK_LENGTH);
				CounterMap::iterator qb = qCounter.bookedTransQ.find(chunk->nextDestination);
				qb->second -= chunk->booksize;
			}
			chunk = retransmissionQ->getAndExtractMessage(chunk->tsn);
			sctpEV3<<"dequeueAckedChunks(): dequeue chunk tsn="<<chunk->tsn<<", tsna="<<tsna<<", "<<retransmissionQ->getQueueSize()<<" chunks still in queue \n";

			if (!chunk->hasBeenAcked)
				newlyAckedBytes += (chunk->booksize);
			SCTP::AssocStatMap::iterator iter=sctpMain->assocStatMap.find(assocId);
			iter->second.ackedBytes+=chunk->len/8;

			if (chunk->hasBeenAcked == false && chunk->countsAsOutstanding)
			{
				chunk->hasBeenAcked = true;
				chunk->countsAsOutstanding = false;
				if (chunk->numberOfTransmissions == 1 && chunk->lastDestination == pathId)
				{
					timeDiff = simulation.getSimTime() - chunk->sendTime;

					if (timeDiff < *rttEstimation || *rttEstimation == -1.0)
						*rttEstimation = timeDiff;

					sctpEV3<<"dequeueAckedChunks(): computed rtt time diff == "<<timeDiff<<" for TSN"<<chunk->tsn<<"\n";
				}
				getPath(chunk->lastDestination)->outstandingBytes -= chunk->booksize;
				sctpEV3<<"osb="<<getPath(chunk->lastDestination)->outstandingBytes<<"\n";
				CounterMap::iterator i=qCounter.roomRetransQ.find(getPath(chunk->lastDestination)->remoteAddress);
						i->second -= ADD_PADDING(chunk->booksize+SCTP_DATA_CHUNK_LENGTH);
			}
			pmClearPathCounter(chunk->lastDestination);
			if (chunk->userData != NULL)
			{
				delete chunk->userData;
			}
			delete chunk;
		}
		else
			break;
	}


	sctpEV3<<"dequeueAckedChunks(): newly_acked_bytes== "<<newlyAckedBytes<<", rtt estimation == "<<*rttEstimation<<"\n";


	return (newlyAckedBytes);
}

void SCTPAssociation::pmClearPathCounter(IPvXAddress pathId)
{
	SCTPPathMap::iterator pm=sctpPathMap.find(pathId);
	if (pm!=sctpPathMap.end())
	{
		pm->second->pathErrorCount = 0;

		if (pm->second->activePath==false)
		{
			/* notify the application */
			pathStatusIndication(pathId, true);

			sctpEV3<<"Path "<<pathId<<" state changes from INACTIVE to ACTIVE !!!!\n";

		}
	}
}

void SCTPAssociation::pathStatusIndication(IPvXAddress pid, bool status)
{
	cPacket* msg=new cPacket("StatusInfo");
	msg->setKind(SCTP_I_STATUS);
	SCTPStatusInfo *cmd = new SCTPStatusInfo();
	cmd->setPathId(pid);
	cmd->setAssocId(assocId);
	cmd->setActive(status);
	msg->setControlInfo(cmd);
	if (!status)
	{
		SCTP::AssocStatMap::iterator iter=sctpMain->assocStatMap.find(assocId);
		iter->second.numPathFailures++;
	}
	sendToApp(msg);
}

void SCTPAssociation::pmRttMeasurement(IPvXAddress pathId, simtime_t rttEstimate, int32 acknowledgedBytes)
{

	if (rttEstimate != -1.0)
	{
		/* we have a valid estimation */
		SCTPPathVariables* path=getPath(pathId);

		if (simulation.getSimTime() > path->updateTime)
		{

			if (path->updateTime == 0.0)
			{
				path->rttvar = rttEstimate / 2;
				path->srtt = rttEstimate;
				path->pathRto = 3 * rttEstimate;
				path->pathRto = max(min(path->pathRto.dbl(), sctpMain->par("rtoMax")), (double)sctpMain->par("rtoMin"));

			}
			else
			{

				path->rttvar = (1.0 - (double)sctpMain->par("rtoBeta")) * path->rttvar +	(double)sctpMain->par("rtoBeta") * fabs(path->srtt - rttEstimate);

				path->srtt = (1.0 - (double)sctpMain->par("rtoAlpha")) * path->srtt + (double)sctpMain->par("rtoAlpha") * rttEstimate;

				path->pathRto = path->srtt + 4 * path->rttvar;

				path->pathRto = max(min(path->pathRto.dbl(), sctpMain->par("rtoMax")), (double)sctpMain->par("rtoMin"));
			}

			sctpEV3<<simulation.getSimTime()<<": Updating timer values for path "<<pathId<<"...RTO == "<<path->pathRto<<"\n";
			sctpEV3<<" rttEstimat="<<rttEstimate<<"  SRTT == "<<path->srtt<<" ------>  RTTVAR == "<<path->rttvar<<"\n";

			/* RFC2960, sect.6.3.1: new RTT measurements SHOULD be made no more than once per round-trip */
			path->updateTime = simulation.getSimTime() + path->srtt;
		}
		path->pathRTO->record(path->pathRto);
		path->pathRTT->record(rttEstimate);
    }

	if (acknowledgedBytes > 0)
	{
		/* we received a (new) SACK/HB ACK from our peer -- reset the error counters for this path */
		state->errorCount = 0;
		pmClearPathCounter(pathId);
	}

}

void SCTPAssociation::fcAdjustCounters(uint32 ackedBytes, uint32 osb, bool ctsnaAdvanced, IPvXAddress pathId, uint32 pathOsb, uint32 newOsb)
{
	SCTPPathVariables* path=getPath(pathId);


	sctpEV3<<"fcAdjustCounters: cwnd="<<path->cwnd<<" ssthresh="<<path->ssthresh<<" pathOsbBefore="<<pathOsb<<" osbBefore="<<osb<<" ackedBytes="<<ackedBytes<<"\n";


	if (path->cwnd <=path->ssthresh)
	{
		for (SCTPPathMap::iterator iter=sctpPathMap.begin(); iter!=sctpPathMap.end(); iter++)
		{
			iter->second->partialBytesAcked = 0;
		}

		/**
		 * From the implementers guide: increase cwnd only if
		 * - the current congestion window is being fully utilized,
		 * - an incoming SACK advances the Cumulative TSN Ack Point,
		 * - and the data sender (sack receiver) is not in Fast Recovery.
		 */

		sctpEV3<<"ctsnaAdvanced="<<ctsnaAdvanced<<"  fastRecoveryActive="<<state->fastRecoveryActive<<"\n";

		if (ctsnaAdvanced == true && pathOsb >= path->cwnd )
		{
			path->cwndAdjustmentTime = simulation.getSimTime();

			path->cwnd += (int32)min(path->pmtu, ackedBytes);
			path->pathCwnd->record(path->cwnd);

			sctpEV3<<"SLOW START: Setting CWND of path "<<pathId<<" to "<<path->cwnd<<" bytes\n";

		}

	}
	else
	{
		path->partialBytesAcked += ackedBytes;
	sctpEV3<<"partialBytesAcked="<<path->partialBytesAcked<<"  cwnd="<<path->cwnd<<"  osb="<<osb<<" advanced="<<ctsnaAdvanced<<"\n";
		if ((path->partialBytesAcked  >= path->cwnd) &&
			(osb >= path->cwnd) && (ctsnaAdvanced == true))
		{
			path->cwnd += path->pmtu;
			path->pathCwnd->record(path->cwnd);
			// I.R. make the term easier
			path->partialBytesAcked =
				((path->cwnd < path->partialBytesAcked) ?
					(path->partialBytesAcked - path->cwnd) : 0);

			/* get and store the time when CWND was adjusted here */
			path->cwndAdjustmentTime = simulation.getSimTime();


			sctpEV3<<"CONG.AVOID.: Setting CWND of path "<<pathId<<" to "<<path->cwnd<<" bytes\n";

		}
	}


	if (newOsb == 0) path->partialBytesAcked = 0;
}

bool SCTPAssociation::allPathsInactive(void)
{
	for(SCTPPathMap::iterator i=sctpPathMap.begin(); i!=sctpPathMap.end(); i++)
	{
		if (i->second->activePath)
			return false;
	}

	return true;
}

uint32 SCTPAssociation::getLevel(IPvXAddress addr)
{
	if (addr.isIPv6())
	{
		if (addr.get6().getScope()==IPv6Address::UNSPECIFIED || addr.get6().getScope()==IPv6Address::MULTICAST)
			return 0;
		if (addr.get6().getScope()==IPv6Address::LOOPBACK)
			return 1;
		if (addr.get6().getScope()==IPv6Address::LINK)
			return 2;
		if (addr.get6().getScope()==IPv6Address::SITE)
			return 3;
	}
	else
	{
		//Addresses usable with SCTP, but not as destination or source address
		if (addr.get4().maskedAddrAreEqual(addr.get4(), IPAddress("0.0.0.0"), IPAddress("255.0.0.0")) ||
			addr.get4().maskedAddrAreEqual(addr.get4(), IPAddress("224.0.0.0"), IPAddress("240.0.0.0")) ||
			addr.get4().maskedAddrAreEqual(addr.get4(), IPAddress("198.18.0.0"), IPAddress("255.255.255.0")) ||
			addr.get4().maskedAddrAreEqual(addr.get4(), IPAddress("192.88.99.0"), IPAddress("255.255.255.0")))
			return 0;

		//Loopback
		if (addr.get4().maskedAddrAreEqual(addr.get4(), IPAddress("127.0.0.0"), IPAddress("255.0.0.0")))
		return 1;

		//Link-local
		if (addr.get4().maskedAddrAreEqual(addr.get4(), IPAddress("169.254.0.0"), IPAddress("255.255.0.0")))
			return 2;

		//Private
		if (addr.get4().maskedAddrAreEqual(addr.get4(), IPAddress("10.0.0.0"), IPAddress("255.0.0.0")) ||
			addr.get4().maskedAddrAreEqual(addr.get4(), IPAddress("172.16.0.0"), IPAddress("255.240.0.0")) ||
			addr.get4().maskedAddrAreEqual(addr.get4(), IPAddress("192.168.0.0"), IPAddress("255.255.0.0")))
			return 3;
	 }
	//Global
	return 4;
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


