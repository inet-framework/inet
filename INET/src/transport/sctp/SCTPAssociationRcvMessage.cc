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
#include "SCTPMessage_m.h"
#include "IPControlInfo_m.h"
#include "SCTPQueue.h"
#include "SCTPAlgorithm.h"
#include "IPv4InterfaceData.h"
#include "IPv6InterfaceData.h"

bool SCTPAssociation::process_RCV_Message(SCTPMessage *sctpmsg, IPvXAddress src, IPvXAddress dest)
{
	uint8 type;
	bool trans=true, sendAllowed=false, dupReceived=false, dataChunkReceived=false, dataChunkDelivered = false;
	bool shutdownCalled=false;
	SCTPPathVariables* path;
	SCTPChunk* header;
	SCTPEventCode event;
	int32 srcPort, destPort;
	uint32 dataChunkCount = 0;
	
	 
	sctpEV3<<getFullPath()<<" SCTPAssociationRcvMessage:process_RCV_Message\n";
	sctpEV3<<"localAddr="<<localAddr<<"  remoteAddr="<<remoteAddr<<"\n";
	uint32 numberOfChunks = sctpmsg->getChunksArraySize();
	sctpEV3<<"numberOfChunks="<<numberOfChunks<<"\n";


	if ((sctpmsg->hasBitError() || !sctpmsg->getChecksumOk()))
	{
		if (((SCTPChunk*)(sctpmsg->getChunks(0)))->getChunkType()==INIT_ACK)
		{
			stopTimer(T1_InitTimer);
			sctpEV3<<"InitAck with bit-error. Retransmit Init\n";
			retransmitInit();
			startTimer(T1_InitTimer,state->initRexmitTimeout);
		}
		if (((SCTPChunk*)(sctpmsg->getChunks(0)))->getChunkType()==COOKIE_ACK)
		{
			stopTimer(T1_InitTimer);
			sctpEV3<<"CookieAck with bit-error. Retransmit CookieEcho\n";
			retransmitCookieEcho();
			startTimer(T1_InitTimer,state->initRexmitTimeout);
		}
	}

	srcPort = sctpmsg->getDestPort();
	destPort = sctpmsg->getSrcPort();
	
	SCTPPathMap::iterator it=sctpPathMap.find(src);
	if (it!=sctpPathMap.end())
		path=it->second;
	else
		path=NULL;
	state->sctpmsg = (SCTPMessage*)sctpmsg->dup();
	for (uint32 i=0; i<numberOfChunks; i++)
	{
		header=(SCTPChunk*)(sctpmsg->removeChunk());
		type = header->getChunkType();

		if (type!=INIT && type!=ABORT && sctpmsg->getTag()!= peerVTag)
		{
			
			ev<<" VTag "<<sctpmsg->getTag()<<" incorrect. Should be "<<peerVTag<<" localVTag="<<localVTag<<"\n";
			
			return true;
		}

		switch (type)
		{
		case INIT: 
			
			sctpEV3<<"SCTPAssociationRcvMessage: INIT received\n";
			
			SCTPInitChunk* initChunk;
			initChunk = check_and_cast<SCTPInitChunk *>(header);
			if (initChunk->getNoInStreams()!=0 && initChunk->getNoOutStreams()!=0 && initChunk->getInitTag()!=0)
				trans = processInitArrived(initChunk, srcPort, destPort);
			i = numberOfChunks-1;
			delete initChunk;
			break;
		case INIT_ACK:
			sctpEV3<<"SCTPAssociationRcvMessage: INIT_ACK received\n"; 
			if (fsm->getState()==SCTP_S_COOKIE_WAIT)
			{
				SCTPInitAckChunk* initAckChunk;
				initAckChunk = check_and_cast<SCTPInitAckChunk *>(header);
				if (initAckChunk->getNoInStreams()!=0 && initAckChunk->getNoOutStreams()!=0 && initAckChunk->getInitTag()!=0)
					trans = processInitAckArrived(initAckChunk);
				else if (initAckChunk->getInitTag()==0)
				{
					sendAbort();
					sctpMain->removeAssociation(this);	
				}
				i = numberOfChunks-1;
				delete initAckChunk;
			}
			else
				sctpEV3<<"INIT_ACK will be ignored\n";
			break;
		case COOKIE_ECHO:
			sctpEV3<<"SCTPAssociationRcvMessage: COOKIE_ECHO received\n";
			SCTPCookieEchoChunk* cookieEchoChunk;
			cookieEchoChunk = check_and_cast<SCTPCookieEchoChunk *>(header);
			trans = processCookieEchoArrived(cookieEchoChunk,src);
			delete cookieEchoChunk;
			break;
		case COOKIE_ACK: 
			
			sctpEV3<<"SCTPAssociationRcvMessage: COOKIE_ACK received\n";
			if (fsm->getState()==SCTP_S_COOKIE_ECHOED)
			{
				SCTPCookieAckChunk* cookieAckChunk;
				cookieAckChunk = check_and_cast<SCTPCookieAckChunk *>(header);
				trans = processCookieAckArrived();
				delete cookieAckChunk;
			}
			break;
		case DATA: 
			
			sctpEV3<<"\nSCTPAssociationRcvMessage: DATA received\n";
			if (fsm->getState()==SCTP_S_COOKIE_ECHOED)
				trans=performStateTransition(SCTP_E_RCV_COOKIE_ACK);
			
			if (!(fsm->getState()==SCTP_S_SHUTDOWN_RECEIVED || fsm->getState()==SCTP_S_SHUTDOWN_ACK_SENT))
			{
				SCTPDataChunk* dataChunk;
				dataChunk = check_and_cast<SCTPDataChunk *>(header);
				if (dataChunk->getByteLength()-16>0)
				{
					dataChunkCount++;
					event=processDataArrived(dataChunk, dataChunkCount);
					if (event == SCTP_E_DELIVERED)
					{
						dataChunkReceived=true;
						dataChunkDelivered = true;
						state->sackAllowed = true;				
					}
					else if (event==SCTP_E_SEND || event==SCTP_E_IGNORE)
					{
						dataChunkReceived=true;
						state->sackAllowed = true;
					}
					else if (event==SCTP_E_DUP_RECEIVED)
					{
						dupReceived=true;
					}
				}
				else
				{
					sendAbort();
					sctpMain->removeAssociation(this);
				}
				
				delete dataChunk;
			}
			trans = true;
			break;	
		case SACK: 
		{
			CounterMap::iterator rtq=qCounter.roomRetransQ.find(remoteAddr);
			int32 scount;
			scount = qCounter.roomSumSendStreams;
			sctpEV3<<"SCTPAssociationRcvMessage: SACK received\n";
			
			SCTPSackChunk* sackChunk;
			sackChunk = check_and_cast<SCTPSackChunk *>(header);
			processSackArrived(sackChunk);
			trans=true;
			sendAllowed=true;
			delete sackChunk;
			sctpEV3<<"state->lastTsnAck="<<state->lastTsnAck<<" state->lastTSN="<<state->lastTSN<<" getOutstandingBytes()="<<getOutstandingBytes()<<" transmissionQ->getQueueSize()="<<transmissionQ->getQueueSize()<<" scount="<<scount<<" fsm->getState()="<<fsm->getState()<<"\n";
			if ((state->lastTsnAck == state->lastTSN || (getOutstandingBytes()==0 && transmissionQ->getQueueSize()==0 && scount==0)) && fsm->getState() == SCTP_S_SHUTDOWN_PENDING)
			{
				sctpEV3<<"no more packets: send Shutdown\n";
				sendShutdown();
				trans=performStateTransition(SCTP_E_NO_MORE_OUTSTANDING);
				shutdownCalled=true;	
			}
			else if (fsm->getState()==SCTP_S_SHUTDOWN_RECEIVED && getOutstandingBytes()==0)
			{
				sctpEV3<<"no more outstanding\n";
				sendShutdownAck(remoteAddr);
				trans=performStateTransition(SCTP_E_NO_MORE_OUTSTANDING);
			}
			break;
		}
		case ABORT:
			SCTPAbortChunk* abortChunk;
			abortChunk = check_and_cast<SCTPAbortChunk *>(header);
			sctpEV3<<"SCTPAssociationRcvMessage: ABORT with T-Bit "<<abortChunk->getT_Bit() << " received\n";
			if (sctpmsg->getTag() == localVTag || sctpmsg->getTag() == peerVTag)
			{
				sendIndicationToApp(SCTP_I_ABORT);
				trans=performStateTransition(SCTP_E_ABORT);
			}
			delete abortChunk;
			break;	
		case HEARTBEAT: 
			
			sctpEV3<<"SCTPAssociationRcvMessage: HEARTBEAT received\n";	
			
			SCTPHeartbeatChunk* heartbeatChunk;
			heartbeatChunk = check_and_cast<SCTPHeartbeatChunk *>(header);	
			if (!(fsm->getState()==SCTP_S_CLOSED ))
			{
				sendHeartbeatAck(heartbeatChunk, dest,src);	
			}
			trans=true;
			delete heartbeatChunk;
			break;
		case HEARTBEAT_ACK:
			
			sctpEV3<<"SCTPAssociationRcvMessage: HEARTBEAT_ACK received\n";
			if (fsm->getState()==SCTP_S_COOKIE_ECHOED)
				trans=performStateTransition(SCTP_E_RCV_COOKIE_ACK);
			
			SCTPHeartbeatAckChunk* heartbeatAckChunk;
			heartbeatAckChunk = check_and_cast<SCTPHeartbeatAckChunk *>(header);	
			if (path)
				processHeartbeatAckArrived(heartbeatAckChunk, path);	
			trans=true;
			delete heartbeatAckChunk;
			break;
		case SHUTDOWN:
			sctpEV3<<"Shutdown received\n";
			SCTPShutdownChunk* shutdownChunk;
			shutdownChunk = check_and_cast<SCTPShutdownChunk *>(header);
			if (shutdownChunk->getCumTsnAck()>state->lastTsnAck)
			{
				simtime_t rttEst=-1.0;
				dequeueAckedChunks(shutdownChunk->getCumTsnAck(), remoteAddr, &rttEst); 
				state->lastTsnAck = shutdownChunk->getCumTsnAck();
			}
			trans=performStateTransition(SCTP_E_RCV_SHUTDOWN);
			sendIndicationToApp(SCTP_I_SHUTDOWN_RECEIVED);	
			trans=true;
			delete shutdownChunk;
			break;
		case SHUTDOWN_ACK:
			sctpEV3<<"ShutdownAck received\n";
			if (fsm->getState()!=SCTP_S_ESTABLISHED)
			{
				SCTPShutdownAckChunk* shutdownAckChunk;
				shutdownAckChunk = check_and_cast<SCTPShutdownAckChunk *>(header);
				sendShutdownComplete();
				stopTimers();
				stopTimer(T2_ShutdownTimer);
				stopTimer(T5_ShutdownGuardTimer);
				
				if (fsm->getState()==SCTP_S_SHUTDOWN_SENT || fsm->getState()==SCTP_S_SHUTDOWN_ACK_SENT)
				{
					trans=performStateTransition(SCTP_E_RCV_SHUTDOWN_ACK);	
					sendIndicationToApp(SCTP_I_CLOSED);
					delete state->shutdownChunk;
				}
				
				delete shutdownAckChunk;
			}
			break;	
		case SHUTDOWN_COMPLETE:
			sctpEV3<<"Shutdown Complete arrived\n";
			SCTPShutdownCompleteChunk* shutdownCompleteChunk;
			shutdownCompleteChunk = check_and_cast<SCTPShutdownCompleteChunk *>(header);
			trans=performStateTransition(SCTP_E_RCV_SHUTDOWN_COMPLETE);
			sendIndicationToApp(SCTP_I_PEER_CLOSED);	// necessary for NAT-Rendezvous
			if (trans==true)
				stopTimers();
			stopTimer(T2_ShutdownTimer);
			stopTimer(T5_ShutdownGuardTimer);	
			delete state->shutdownAckChunk;
			delete shutdownCompleteChunk;
			break;	
		default: sctpEV3<<"different type\n"; break;
		}
		
		if (i==numberOfChunks-1 && (dataChunkReceived || dupReceived))
		{
			sendAllowed=true;
			sctpEV3<<"i="<<i<<" sendAllowed=true; scheduleSack\n";
			scheduleSack();	
			if (fsm->getState()==SCTP_S_SHUTDOWN_SENT && state->ackState>=sackFrequency)
				sendSack();
		}

						
		/* send any new DATA chunks, SACK chunks, HB chunks etc. */
		if ((fsm->getState()==SCTP_S_ESTABLISHED || fsm->getState()==SCTP_S_SHUTDOWN_PENDING) && sendAllowed && !shutdownCalled)
		{
			sctpEV3<<"SCTPRcvMessage 449:sendAll to primaryPath:"<<state->primaryPathIndex<<"\n";
			sendAll(state->primaryPathIndex);	
			if (remoteAddr!=state->primaryPathIndex)
			{
				sctpEV3<<"SCTPRcvMessage 453:sendAll to remoteAddr: "<<remoteAddr<<"\n";
				sendAll(remoteAddr);
			}
				
		}
	}
	disposeOf(state->sctpmsg);
	sctpEV3<<"trans="<<trans<<"\n";
	return trans;
}

bool SCTPAssociation::processInitArrived(SCTPInitChunk* initchunk, int32 srcPort, int32 destPort)
{
	SCTPAssociation* assoc;
	char timername[70];
	bool trans = false;
	InterfaceTableAccess interfaceTableAccess;
	AddressVector adv;
	sctpEV3<<"processInitArrived\n";
	if (fsm->getState()==SCTP_S_CLOSED)
	{
		sctpEV3<<"fork="<<state->fork<<" initReceived="<<state->initReceived<<"\n";
		if (state->fork && !state->initReceived)
		{
			sctpEV3<<"cloneAssociation\n";
			assoc = cloneAssociation(); 
			sctpEV3<<"addForkedAssociation\n";
			sctpMain->addForkedAssociation(this, assoc, localAddr, remoteAddr, srcPort, destPort);

			sctpEV3 << "Connection forked: this connection got new assocId=" << assocId << ", "
				"spinoff keeps LISTENing with assocId=" << assoc->assocId << "\n";
			sprintf(timername, "T2_SHUTDOWN of assoc %d", assocId);
			T2_ShutdownTimer->setName(timername);
			sprintf(timername, "SACK_TIMER of assoc %d", assocId);
			SackTimer->setName(timername);
			sprintf(timername, "T1_INIT of assoc %d", assocId);
			T1_InitTimer->setName(timername);
		}
		else
		{
			sctpMain->updateSockPair(this, localAddr, remoteAddr, srcPort, destPort);

		} 
		if (!state->initReceived)
		{
			state->initReceived = true;
			state->initialPrimaryPath = remoteAddr;
			state->primaryPathIndex = remoteAddr;
			if (initchunk->getAddressesArraySize()==0)
			{
				SCTPPathVariables* rPath = new SCTPPathVariables(remoteAddr, this);
				sctpPathMap[rPath->remoteAddress] = rPath;
				qCounter.roomTransQ[rPath->remoteAddress] = 0;
				qCounter.roomRetransQ[rPath->remoteAddress] = 0;
				qCounter.bookedTransQ[rPath->remoteAddress] = 0;
			}
			initPeerTsn=initchunk->getInitTSN();
			state->cTsnAck = initPeerTsn - 1;
			state->initialPeerRwnd = initchunk->getA_rwnd();
			state->peerRwnd = state->initialPeerRwnd;
			localVTag= initchunk->getInitTag();
			numberOfRemoteAddresses =initchunk->getAddressesArraySize(); 
			IInterfaceTable *ift = interfaceTableAccess.get();
			state->localAddresses.clear();
			if (localAddressList.front() == IPvXAddress("0.0.0.0"))
			{
				for (int32 i=0; i<ift->getNumInterfaces(); ++i)
				{
					//if (ift->getInterface(i)->ipv4()!=NULL)
					if (ift->getInterface(i)->ipv4Data()!=NULL)
					{ 
						//adv.push_back(ift->getInterface(i)->ipv4()->getIPAddress());
						adv.push_back(ift->getInterface(i)->ipv4Data()->getIPAddress());
					}
					else if (ift->getInterface(i)->ipv6Data()!=NULL)
					{ 
						adv.push_back(ift->getInterface(i)->ipv6Data()->getAddress(0));
					}
				}
			}
			else
			{	
				adv = localAddressList;
			}
			uint32 rlevel = getLevel(remoteAddr);
			if (rlevel>0)
				for (AddressVector::iterator i=adv.begin(); i!=adv.end(); ++i)
				{
					if (getLevel((*i))>=rlevel)
					{
						sctpMain->addLocalAddress(this, (*i));
						state->localAddresses.push_back((*i));
					}
				}
			for (uint32 j=0; j<initchunk->getAddressesArraySize(); j++)
			{
				// skip IPv6 because we can't send to them yet
				if (initchunk->getAddresses(j).isIPv6())
					continue;
				// set path variables for this pathlocalAddresses
				SCTPPathVariables* path = new SCTPPathVariables(initchunk->getAddresses(j), this);
				for (AddressVector::iterator k=state->localAddresses.begin(); k!=state->localAddresses.end(); ++k)
				{
					sctpMain->addRemoteAddress(this,(*k), initchunk->getAddresses(j));
					this->remoteAddressList.push_back(initchunk->getAddresses(j));
				}			
				sctpPathMap[path->remoteAddress] = path;
				qCounter.roomTransQ[path->remoteAddress] = 0;
				qCounter.roomRetransQ[path->remoteAddress] = 0;
				qCounter.bookedTransQ[path->remoteAddress] = 0;
			}
			SCTPPathMap::iterator ite=sctpPathMap.find(remoteAddr);
			if (ite==sctpPathMap.end())
			{
				SCTPPathVariables* path = new SCTPPathVariables(remoteAddr, this);
				sctpPathMap[remoteAddr] = path;
				qCounter.roomTransQ[remoteAddr] = 0;
				qCounter.roomRetransQ[remoteAddr] = 0;
				qCounter.bookedTransQ[remoteAddr] = 0;
			}
			trans=performStateTransition(SCTP_E_RCV_INIT);
			if (trans)
			{
				sendInitAck(initchunk);
			}
		}
		else if (fsm->getState()==SCTP_S_CLOSED)
		{
			trans=performStateTransition(SCTP_E_RCV_INIT);
			if (trans)
				sendInitAck(initchunk);
		}
		else 
			trans = true;
	}
	else if (fsm->getState()==SCTP_S_COOKIE_WAIT) //INIT-Collision
	{
		sctpEV3<<"INIT collision: send Init-Ack\n";	
		sendInitAck(initchunk);  
		trans=true;
	}
	else if (fsm->getState()==SCTP_S_COOKIE_ECHOED || fsm->getState()==SCTP_S_ESTABLISHED)
	{
		// check, whether a new address has been added
		bool addressPresent = false;
		for (uint32 j=0; j<initchunk->getAddressesArraySize(); j++)
		{
			if (initchunk->getAddresses(j).isIPv6())
				continue;
			for (AddressVector::iterator k=remoteAddressList.begin(); k!=remoteAddressList.end(); ++k)
			{
				if ((*k)==(initchunk->getAddresses(j)))
				{
					addressPresent = true;
					break;
				}
			}
			if (!addressPresent)
			{
				sendAbort();
				return true;
			}
		}
		sendInitAck(initchunk);
		trans=true;
	}
	else if (fsm->getState()==SCTP_S_SHUTDOWN_ACK_SENT)
		trans = true;
	printSctpPathMap();
	return trans;
}


bool SCTPAssociation::processInitAckArrived(SCTPInitAckChunk* initAckChunk)
{
	bool trans = false;
	if (fsm->getState()==SCTP_S_COOKIE_WAIT)
	{
		sctpEV3<<"State is COOKIE_WAIT, Cookie_Echo has to be sent\n";
		stopTimer(T1_InitTimer);
		state->initRexmitTimeout = SCTP_TIMEOUT_INIT_REXMIT;
		trans=performStateTransition(SCTP_E_RCV_INIT_ACK);
		//delete state->initChunk; will be deleted when state ESTABLISHED is entered
		if (trans)
		{
			state->initialPrimaryPath = remoteAddr;
			state->primaryPathIndex = remoteAddr;
			initPeerTsn=initAckChunk->getInitTSN();
			localVTag= initAckChunk->getInitTag();
			state->cTsnAck = initPeerTsn - 1;
			state->initialPeerRwnd = initAckChunk->getA_rwnd();
			state->peerRwnd = state->initialPeerRwnd;
			remoteAddressList.clear();
			numberOfRemoteAddresses =initAckChunk->getAddressesArraySize(); 
			sctpEV3<<"number of remote addresses in initAck="<<numberOfRemoteAddresses<<"\n";
			for (uint32 j=0; j<numberOfRemoteAddresses; j++)
			{
				if (initAckChunk->getAddresses(j).isIPv6())
					continue;
				for (AddressVector::iterator k=state->localAddresses.begin(); k!=state->localAddresses.end(); ++k)
				{
					if (!((*k).isUnspecified()))
					{
						sctpEV3<<"addPath "<<initAckChunk->getAddresses(j)<<"\n";
						sctpMain->addRemoteAddress(this,(*k), initAckChunk->getAddresses(j));
						this->remoteAddressList.push_back(initAckChunk->getAddresses(j));
						addPath(initAckChunk->getAddresses(j));
					}
				}
			}
			SCTPPathMap::iterator ite=sctpPathMap.find(remoteAddr);
			if (ite==sctpPathMap.end())
			{
				SCTPPathVariables* path = new SCTPPathVariables(remoteAddr, this);
				sctpPathMap[remoteAddr] = path;
				qCounter.roomTransQ[remoteAddr] = 0;
				qCounter.roomRetransQ[remoteAddr] = 0;
				qCounter.bookedTransQ[remoteAddr] = 0;
			}
		
			inboundStreams = ((initAckChunk->getNoOutStreams()<inboundStreams)?initAckChunk->getNoOutStreams():inboundStreams);
			outboundStreams = ((initAckChunk->getNoInStreams()<outboundStreams)?initAckChunk->getNoInStreams():outboundStreams);
			(this->*ssFunctions.ssInitStreams)(inboundStreams, outboundStreams);
			sendCookieEcho(initAckChunk);
		}
		startTimer(T1_InitTimer, state->initRexmitTimeout);
		
	}
	else
		sctpEV3<<"State="<<fsm->getState()<<"\n";
	printSctpPathMap();
	return trans;
}



bool SCTPAssociation::processCookieEchoArrived(SCTPCookieEchoChunk* cookieEcho, IPvXAddress addr)
{
	bool trans = false;
	SCTPCookie* cookie = check_and_cast<SCTPCookie*>(cookieEcho->getStateCookie());
	if (cookie->getCreationTime()+(int32)sctpMain->par("validCookieLifetime")<simulation.getSimTime())
	{
		sctpEV3<<"stale Cookie: sendAbort\n";
		sendAbort();
		delete cookie;
		return trans;
	}
	if (fsm->getState()==SCTP_S_CLOSED)
	{
		if (cookie->getLocalTag()!=localVTag || cookie->getPeerTag() != peerVTag) 
		{
			bool same=true;
			for (int32 i=0; i<32; i++)
			{
				if (cookie->getLocalTieTag(i)!=state->localTieTag[i])
				{
					same = false;
					break;
				}
				if (cookie->getPeerTieTag(i)!=state->peerTieTag[i])
				{
					same = false;
					break;
				}
			}
			if (!same)
			{
				sendAbort();
				delete cookie;
				return trans;
			}
		}
		
		sctpEV3<<"State is CLOSED, Cookie_Ack has to be sent\n";
		trans=performStateTransition(SCTP_E_RCV_VALID_COOKIE_ECHO);
		if (trans)
			sendCookieAck(addr);//send to address
	}
	else if (fsm->getState()==SCTP_S_ESTABLISHED || fsm->getState()==SCTP_S_COOKIE_WAIT || fsm->getState()==SCTP_S_COOKIE_ECHOED)
	{
		sctpEV3<<"State is not CLOSED, but COOKIE_ECHO received. Compare the Tags\n";
		// case A: Peer restarted, retrieve information from cookie
		if (cookie->getLocalTag()!=localVTag && cookie->getPeerTag() != peerVTag )
		{
			bool same=true;
			for (int32 i=0; i<32; i++)
			{
				if (cookie->getLocalTieTag(i)!=state->localTieTag[i])
				{
					same = false;
					break;
				}
				if (cookie->getPeerTieTag(i)!=state->peerTieTag[i])
				{
					same = false;
					break;
				}
			}
			if (same)
			{
				localVTag = cookie->getLocalTag();
				peerVTag = cookie->getPeerTag();
				sendCookieAck(addr);
			}
		}
		// case B: Setup collision, retrieve information from cookie
		else if (cookie->getPeerTag()==peerVTag && (cookie->getLocalTag()!=localVTag || cookie->getLocalTag()==0))
		{
			localVTag = cookie->getLocalTag();
			peerVTag = cookie->getPeerTag();
			performStateTransition(SCTP_E_RCV_VALID_COOKIE_ECHO);
			sendCookieAck(addr);
		}
		else if (cookie->getPeerTag()==peerVTag && cookie->getLocalTag()==localVTag)
		{
			sendCookieAck(addr); //send to address src
		}
		trans=true;
	}
	else 
	{
		sctpEV3<<"State="<<fsm->getState()<<"\n";
		trans = true;
	}
	delete cookie;
	return trans;
}

bool SCTPAssociation::processCookieAckArrived()
{
bool trans=false;

	if (fsm->getState()==SCTP_S_COOKIE_ECHOED)
	{
		stopTimer(T1_InitTimer);
		trans=performStateTransition(SCTP_E_RCV_COOKIE_ACK);
		if (state->cookieChunk->getCookieArraySize()==0)
		{
				delete state->cookieChunk->getStateCookie();
		}
		delete state->cookieChunk;
		return trans;			
	}
	else
		sctpEV3<<"State="<<fsm->getState()<<"\n";

	return trans;
}



SCTPEventCode SCTPAssociation::processSackArrived(SCTPSackChunk* sackChunk)
{
	uint32 tsna, ackedBytes=0, osb=0, bufferPosition, osbPathBefore, osbBefore, lo, hi, hiAcked;
	uint64 arwnd;
	uint16 numGaps, numDups;
	bool ctsnaAdvanced = false, rtxNecessary=false, lowestTsnRetransmitted = false;
	SCTPDataVariables *datVar;
	simtime_t  timeDiff, rttEst = -1.0;
	SCTPPathVariables* path;
	IPvXAddress pathId, pid;

	tsna=sackChunk->getCumTsnAck();
	arwnd = sackChunk->getA_rwnd();
	numGaps=sackChunk->getNumGaps();
	numDups=sackChunk->getNumDupTsns();	
	hiAcked = tsna;
	pathId = remoteAddr;	
	path=getPath(pathId);

	path->pathRcvdTSN->record(tsna);
	sctpEV3<<simulation.getSimTime()<<" processSackArrived at "<<localAddr<<": tsna="<<tsna<<" arwnd="<<arwnd<<"  numGaps="<<numGaps<<"  numDups="<<numDups<<" osb on path "<<pathId<<"="<<path->outstandingBytes<<" highestTsnReceived="<<state->highestTsnReceived<<" highestTsnAcked="<<state->highestTsnAcked<<" lastTsnAcked="<<state->lastTsnAck<<"\n";
	if ((int32)path->outstandingBytes<0)
		opp_error("rcv %d ,tsna=%d",__LINE__, tsna);
	for (int32 j=0; j<numGaps; j++)
	{
		sctpEV3<<sackChunk->getGapStart(j)<<" - "<<sackChunk->getGapStop(j)<<"\n";
	}
	for(SCTPPathMap::iterator it=sctpPathMap.begin(); it!=sctpPathMap.end(); it++) 
	{
		it->second->requiresRtx = false;
		if (it->second->remoteAddress == pathId)
			it->second->lastAckTime = simulation.getSimTime();
	}

	if (state->zeroWindowProbing && arwnd>0)
		state->zeroWindowProbing = false;

	osbPathBefore = path->outstandingBytes;
	osbBefore = getOutstandingBytes();
	sctpEV3<<"osb="<<osbPathBefore<<", osbPathBefore="<<osbPathBefore<<"\n";	

	if (tsnGt(tsna, state->lastTsnAck)) 
	{
		/* we have got new chunks acked, and our cum ack point is advanced... */
		/* depending on the parameter osbWithHeader ackedBytes are with or without the header bytes*/
		ackedBytes = dequeueAckedChunks(tsna, pathId, &rttEst); // chunks with tsn between lastTsnAck and tsna are removed from the transmissionQ and the retransmissionQ; outstandingBytes are decreased

		state->lastTsnAck = tsna;
		ctsnaAdvanced = true;
	} 
	else if (tsnLt(tsna, state->lastTsnAck)) 
	{
		 
		sctpEV3<<"processSackArrived : stale CTSNA....returning";
		
		return SCTP_E_IGNORE;
	}
	
	if (state->fastRecoveryActive) 
	{
		if (tsnGt(state->lastTsnAck, state->fastRecoveryExitPoint) || (state->lastTsnAck == state->fastRecoveryExitPoint)) 
		{
			 
			sctpEV3<<"===> Leaving FAST RECOVERY !!! CTSNA == "<<state->lastTsnAck<<" <==\n";
			
			state->fastRecoveryActive = false;
			state->fastRecoveryExitPoint = 0;
		}
	}
	
	if (numGaps == 0 && tsnLt(tsna, state->highestTsnAcked)) 
	{
		sctpEV3<<"numGaps=0 && tsna "<<tsna<<" < highestTsnAcked "<<state->highestTsnAcked<<"\n";
		SCTPQueue::PayloadQueue::iterator pq;
		uint32 i=state->highestTsnAcked;
		pq=retransmissionQ->payloadQueue.find(i); 
		while (i>=tsna+1)
		{
			if (pq!=retransmissionQ->payloadQueue.end())
			{
				if (pq->second->hasBeenAcked)
				{
					pq->second->hasBeenAcked=false;
					if (!pq->second->countsAsOutstanding)	//12.06.08
					{
						if (i>tsna+1 && retransmissionQ->payloadQueue.find(i-1)->second->hasBeenAcked)
							state->highestTsnAcked = i-1;  //I.R.
						else
							state->highestTsnAcked = tsna;
						#ifdef PKT
						tsnWasReneged(pq->second);
						#else
						sctpEV3<<"tsn "<<pq->second->tsn<<" has been removed\n";
						pq->second->hasBeenRemoved = true;
						pq->second->gapReports = 1;
						if (!getPath(pq->second->lastDestination)->T3_RtxTimer->isScheduled())
							startTimer(getPath(pq->second->lastDestination)->T3_RtxTimer, getPath(pq->second->lastDestination)->pathRto);
						sctpEV3<<"highestTsnAcked now "<<state->highestTsnAcked<<"\n";
						#endif
					}
				}	
			}
			i--;
			pq=retransmissionQ->payloadQueue.find(i);
		}
	}

		
	if (numGaps > 0 && !retransmissionQ->payloadQueue.empty())
	{
		 
		sctpEV3<<"We got "<<numGaps<<" GAP reports\n";
		sctpEV3<<"tsna="<<tsna<<"  nextTSN="<<state->nextTSN<<"\n";
		
		/* we got fragment reports...check for newly acked chunks */
		uint32 queuedChunks=retransmissionQ->payloadQueue.size();
		sctpEV3<<"number of chunks in retransmissionQ: "<<queuedChunks<<" highestGapStop: "<<sackChunk->getGapStop(numGaps-1)<<" highestTsnAcked: "<<state->highestTsnAcked<<"\n";

		SCTPQueue::PayloadQueue::iterator pq;
			
		//highest gapStop smaller than highestTsnAcked: there might have been reneging
		if (tsnLt(sackChunk->getGapStop(numGaps-1), state->highestTsnAcked))
		{
			uint32 i=state->highestTsnAcked;
			pq=retransmissionQ->payloadQueue.find(i); 
			while (i>=sackChunk->getGapStop(numGaps-1)+1)
			{
				sctpEV3<<"looking for TSN "<<i<<" in retransmissionQ\n";
				if (pq != retransmissionQ->payloadQueue.end())
				{
					if (pq->second->hasBeenAcked)
					{
						pq->second->hasBeenAcked=false;
						sctpEV3<<i<<" was found. It has been set to hasBeenAcked=false.\n";
						if (pq->second->countsAsOutstanding)
						{
							pq->second->countsAsOutstanding = false; //I.R. FIXME:
							getPath(pq->second->lastDestination)->outstandingBytes -= (pq->second->booksize);
							sctpEV3<<"osb="<<getPath(pq->second->lastDestination)->outstandingBytes<<"\n";
							CounterMap::iterator i=qCounter.roomRetransQ.find(getPath(pq->second->lastDestination)->remoteAddress);
								i->second -= ADD_PADDING(pq->second->booksize+SCTP_DATA_CHUNK_LENGTH);
						}

						if (retransmissionQ->payloadQueue.find(i-1)!=retransmissionQ->payloadQueue.end() &&  retransmissionQ->payloadQueue.find(i-1)->second->hasBeenAcked)
							state->highestTsnAcked = i-1;  //I.R. 12.06.08
						#ifdef PKT
						tsnWasReneged(pq->second);
						#else
						sctpEV3<<"tsn "<<pq->second->tsn<<" was removed\n";
						pq->second->hasBeenRemoved = true;
						pq->second->hasBeenAcked = false;  //12.06.08
						if (!getPath(pq->second->lastDestination)->T3_RtxTimer->isScheduled())
							startTimer(getPath(pq->second->lastDestination)->T3_RtxTimer, getPath(pq->second->lastDestination)->pathRto);
						pq->second->gapReports = 1;
						sctpEV3<<"highestTsnAcked now "<<state->highestTsnAcked<<"\n";
						#endif
					}
				}
				else
					sctpEV3<<"TSN "<<i<<" not found in retransmissionQ\n";
				i--;
				pq=retransmissionQ->payloadQueue.find(i); 
			}
		}
		
		//looking for changes in the gap reports
		for (int32 key=0; key<numGaps; key++)
		{
			lo = sackChunk->getGapStart(key);
			hi = sackChunk->getGapStop(key);
			sctpEV3<<"examine TSNs between "<<lo<<" and "<<hi<<"\n";
			for (uint32 pos=lo; pos<=hi; pos++)
			{
				datVar = retransmissionQ->getVar(pos);
				if (datVar)
				{		
					/*sctpEV3<<"TSN "<<datVar->tsn<<" found in retransmissionQ\n";
					sctpEV3<<"acked="<<datVar->hasBeenAcked<<", abandoned="<<datVar->hasBeenAbandoned<<", removed="<<datVar->hasBeenRemoved<<"\n";*/
					pmClearPathCounter(datVar->lastDestination);
					if (datVar->numberOfTransmissions == 1 && datVar->lastDestination == pathId && datVar->hasBeenAcked == false && datVar->hasBeenRemoved == false) 
					{
						timeDiff = simulation.getSimTime() - datVar->sendTime;
						if (timeDiff < rttEst || rttEst == -1.0) 
						{
							rttEst = timeDiff;
						} 
							
						sctpEV3<<simulation.getSimTime()<<" processSackArrived: computed rtt time diff == "<<timeDiff<<" for TSN "<<datVar->tsn<<"\n";
						
					}
					if (datVar->hasBeenAcked == false && datVar->hasBeenAbandoned == false && datVar->hasBeenRemoved == false) 
					{
						ackedBytes += (datVar->booksize);
						sctpEV3<<simulation.getSimTime()<<": "<<datVar->tsn<<" added to newly acked bytes:  "<<ackedBytes<<"\n";
						if (datVar->tsn > hiAcked)
							hiAcked = datVar->tsn;
						datVar->hasBeenAcked = true;
						if (datVar->countsAsOutstanding)
						{
							
							sctpEV3<<"outstanding\n";
							sctpEV3<<"lastDestination="<<datVar->lastDestination<<"\n";
							sctpEV3<<"booksize="<<datVar->booksize<<"\n";
							sctpEV3<<"outstanding bytes on path="<<getPath(datVar->lastDestination)->outstandingBytes<<"\n";

							getPath(datVar->lastDestination)->outstandingBytes -= datVar->booksize;
							datVar->countsAsOutstanding = false;
							sctpEV3<<"osb="<<getPath(datVar->lastDestination)->outstandingBytes<<"\n";
							CounterMap::iterator i=qCounter.roomRetransQ.find(getPath(datVar->lastDestination)->remoteAddress);
								i->second -= ADD_PADDING(datVar->booksize+SCTP_DATA_CHUNK_LENGTH);
						}
						if (transmissionQ->getVar(datVar->tsn))  //2.1.07
						{
							sctpEV3<<"found tsn "<<datVar->tsn<<" in transmissionQ. Remove Message.\n";
							transmissionQ->removeMsg(datVar->tsn);
							CounterMap::iterator q = qCounter.roomTransQ.find(datVar->nextDestination);
							q->second-=ADD_PADDING(datVar->len/8+SCTP_DATA_CHUNK_LENGTH);
							CounterMap::iterator qb=qCounter.bookedTransQ.find(datVar->nextDestination);
							qb->second-=datVar->booksize;
							
							
						}
						sctpEV3<<"not outstanding\n";
						
						
						datVar->gapReports = 0;		
					} 
				}
				
			}			 
		}
		lo = tsna;
		//examine chunks between the gap reports; they might have to be retransmitted or they could have been removed
		for (int32 key=0; key<numGaps; key++)
		{
			hi = sackChunk->getGapStart(key);

			for (uint32 i = lo+1; i<=hi-1; i++)
			{
				pq=retransmissionQ->payloadQueue.find(i); 
				if (pq != retransmissionQ->payloadQueue.end())
				{
					sctpEV3<<"TSN "<<pq->second->tsn<<"\t";
					if (pq->second->hasBeenAcked)
					{
						sctpEV3<<" has been acked\n";
						pq->second->hasBeenAcked=false;
						if (pq->second->countsAsOutstanding)
						{
							sctpEV3<<"counts as outstanding\n";
							pq->second->countsAsOutstanding = false;
							getPath(pq->second->lastDestination)->outstandingBytes -= (pq->second->booksize);
							sctpEV3<<"osb="<<getPath(pq->second->lastDestination)->outstandingBytes<<"\n";
							CounterMap::iterator i=qCounter.roomRetransQ.find(getPath(pq->second->lastDestination)->remoteAddress);
								i->second -= ADD_PADDING(pq->second->booksize+SCTP_DATA_CHUNK_LENGTH);
						}
						#ifdef PKT
						tsnWasReneged(pq->second);
						#else
						sctpEV3<<"not PKT: TSN "<<pq->second->tsn<<"hasBeenRemoved\n"; 
						pq->second->hasBeenRemoved = true;
						pq->second->gapReports = 1;
						if (!getPath(pq->second->lastDestination)->T3_RtxTimer->isScheduled())
							startTimer(getPath(pq->second->lastDestination)->T3_RtxTimer, getPath(pq->second->lastDestination)->pathRto);
						#endif
					}
					else
					{
						sctpEV3<<" has not been acked, hiAcked="<<hiAcked<<" countsAsOutstanding="<<pq->second->countsAsOutstanding<<"\n";
						if (hiAcked > pq->second->tsn)
						{
							pq->second->gapReports++;
							sctpEV3<<"increase gap reports of TSN "<<pq->second->tsn<<" to "<<pq->second->gapReports<<"\n";
							if (pq->second->gapReports >= (uint32) sctpMain->par("numGapReports")) 
							{
								/* chunks are only fast retransmitted once */
bool fastRtx = false;
	fastRtx = ((pq->second->hasBeenFastRetransmitted == false) && (pq->second->numberOfRetransmissions==0));
								if (fastRtx)
								{
									sctpEV3<<simulation.getSimTime()<<" Got "<<pq->second->gapReports<<" gap_reports, scheduling "<<pq->second->tsn<<" for RTX\n";
									sctpEV3<<"last sendTime for TSN "<<pq->second->tsn<<" was "<<pq->second->sendTime<<". RTT for path "<<pq->second->lastDestination<<" is "<<getPath(pq->second->lastDestination)->srtt<<"\n";
									

									SCTPQueue::PayloadQueue::iterator it=transmissionQ->payloadQueue.find(pq->second->tsn);
									if (it==transmissionQ->payloadQueue.end()) 
									{
										
										SCTP::AssocStatMap::iterator iter=sctpMain->assocStatMap.find(assocId);
										iter->second.numFastRtx++;	
										sctpEV3<<"insert in transmissionQ tsn="<<pq->second->tsn<<"\n";
										pq->second->hasBeenFastRetransmitted = true;
										IPvXAddress oldPid=pq->second->lastDestination;
										pid = getNextDestination(pq->second);
										pq->second->nextDestination = pid;
										if (pq->second->countsAsOutstanding) 
										{
										getPath(pq->second->lastDestination)->outstandingBytes -= pq->second->booksize;
										pq->second->countsAsOutstanding = false;
										CounterMap::iterator i=qCounter.roomRetransQ.find(getPath(pq->second->lastDestination)->remoteAddress);
											i->second -= ADD_PADDING(pq->second->booksize+SCTP_DATA_CHUNK_LENGTH);
										}
										if (!transmissionQ->checkAndInsertVar(pq->second->tsn, pq->second))  
										{
											
											ev<<"Fast RTX: cannot add message/chunk (TSN="<<pq->second->tsn<<") to the transmission Q\n";
											
										}
										else
										{
											CounterMap::iterator q = qCounter.roomTransQ.find(pq->second->nextDestination);
											q->second+=ADD_PADDING(pq->second->len/8+SCTP_DATA_CHUNK_LENGTH);
											CounterMap::iterator qb=qCounter.bookedTransQ.find(pq->second->nextDestination);
											qb->second+=pq->second->booksize;
											sctpEV3<<"Fast RTX: "<<transmissionQ->getQueueSize()<<" chunks = "<<q->second<<"bytes. OldPid="<<oldPid<<", newPid="<<pid<<"\n";
										}
										path->requiresRtx = true;
										rtxNecessary = true;
										if(bufferPosition == 0)
											lowestTsnRetransmitted = true;
									}
								}
							}
						}
						else 
						{
							pq->second->hasBeenFastRetransmitted = false;
							sctpEV3<<"TSN "<<pq->second->tsn<<" countsAsOutstanding="<<pq->second->countsAsOutstanding<<"\n";
							if (hiAcked > pq->second->tsn)
								pq->second->gapReports++;
						}
					}
				}
				else
					sctpEV3<<"TSN "<<i<<" not found in retransmissionQ\n";
			}
			lo = sackChunk->getGapStop(key);
		}

		state->highestTsnAcked = sackChunk->getGapStop(numGaps-1);
		if (state->highestTsnAcked < state->nextTSN-1)
		{
			SCTPQueue::PayloadQueue::iterator iter;

			for (uint32 i=state->highestTsnAcked+1; i< state->nextTSN; i++)
			{
				iter=retransmissionQ->payloadQueue.find(i); 
				if (iter!=retransmissionQ->payloadQueue.end())
				{
					iter->second->hasBeenAcked=false;
				}
			}
		}
	}
		/* update RTT measurement for newly acked data chunks */
	sctpEV3<<simulation.getSimTime()<<": SACK: rtt="<<rttEst<<", ackedBytes="<<ackedBytes<<", pathId="<<pathId<<"\n";
	pmRttMeasurement(pathId, rttEst, ackedBytes);
	
	osb = getOutstandingBytes();
	
	/* compute current receiver window */
	state->peerRwnd = arwnd - osb;

	// position of statement changed 20.07.05 I.R.
	if ((int32)(state->peerRwnd)< 0) state->peerRwnd= 0;

	if (state->peerRwnd > state->initialPeerRwnd)
		state->peerRwnd = state->initialPeerRwnd;

	sctpEV3<<"rwnd set to "<<state->peerRwnd<<" ("<<arwnd<<"-"<<osb<<")\n";
	sctpEV3<<"pathFlow="<<state->peerRwnd/rttEst<<"="<<state->peerRwnd<<"/"<<rttEst<<"\n";	 
	sctpEV3<<"state->peerRwnd now "<<state->peerRwnd<<"\n";
	if (arwnd == 1 || state->peerRwnd < state->swsLimit || arwnd == 0)
	{
		sctpEV3<<"processSackArrived: arwnd="<<arwnd<<" state->peerRwnd="<<state->peerRwnd<<" set peerWindowFull\n";
		state->peerWindowFull = true;
	}
	else
	{
		state->peerWindowFull = false;
		state->zeroWindowProbing = false;
	}

	
	if (ackedBytes > 0) 
	{		
		/* adjust congestion control variables for the corresponding path */
		 
		sctpEV3<<"processSackArrived: "<<ackedBytes<<" bytes were acked\n";
		(this->*ccFunctions.ccUpdateBytesAcked)(ackedBytes, osbBefore,  ctsnaAdvanced, pathId, osbPathBefore, osb);
	} 	

	if (osb == 0) 
	{	
		for (SCTPPathMap::iterator iter=sctpPathMap.begin(); iter!=sctpPathMap.end(); iter++)
		{
			stopTimer(iter->second->T3_RtxTimer);
		}
		if (arwnd == 0)
			state->zeroWindowProbing = true;

	} 
	else 
	{
		sctpEV3<<"outstandingBytes on path "<<pathId<<" = "<<getPath(pathId)->outstandingBytes<<"\n";
		/* there is still something outstanding */
		if (getPath(pathId)->outstandingBytes == 0)
		{
			/* if all data is acked on **this path** stop T3 timer */
			stopTimer(getPath(pathId)->T3_RtxTimer);
		} 
		else if (ctsnaAdvanced) 
		{
			/* if new data is ACKED, restart T3 timer */
			 
			sctpEV3<<"CTSNA was advanced to "<<state->lastTsnAck<<" by incoming SACK chunk, now restart T3 timer for path id "<<(pathId)<<"\n";
			stopTimer(getPath(pathId)->T3_RtxTimer);
			startTimer(getPath(pathId)->T3_RtxTimer,getPath(pathId)->pathRto);
		} 
		else
		/* AJ - added next clause - 07.04.2004 - also restart T3 timer, when lowest TSN is rtx'ed */
			if (lowestTsnRetransmitted == true) 
			{
				 
				sctpEV3<<"Lowest tsn "<<state->lastTsnAck<<" was retransmitted, now restart T3 timer for path id "<<(pathId)<<"\n";
				stopTimer(getPath(pathId)->T3_RtxTimer);
				startTimer(getPath(pathId)->T3_RtxTimer,getPath(pathId)->pathRto);			
			}
	}

	
	(this->*ccFunctions.ccUpdateAfterSack)(rtxNecessary, path);


	return SCTP_E_IGNORE;
}



SCTPEventCode SCTPAssociation::processDataArrived(SCTPDataChunk* dataChunk, uint32 chunkCount)
{
	SCTPEventCode event;
	uint32 tsn;
	bool checkCtsnaChange=false;
	
	//recordScalar("delay", simulation.getSimTime()-dataChunk->getEnqueuingTime());
	state->newChunkReceived = false;
	state->lastDataSourceAddress = remoteAddr;
	SCTPPathVariables* path=getPath(remoteAddr);
	tsn=dataChunk->getTsn();

	sctpEV3<<simulation.getSimTime()<<"  SCTPAssociation::processDataArrived TSN="<<tsn<<", SID="<<dataChunk->getSid()<<", SSN="<<dataChunk->getSsn()<<"\n";
	sctpEV3<<"localRwnd="<<state->localRwnd<<", queuedRcvBytes="<<state->queuedRcvBytes<<"\n";
	sctpEV3<<"Laenge="<<dataChunk->getBitLength()<<"\n";
	sctpEV3<<simulation.getSimTime()<<"  SCTPAssociation::processDataArrived TSN="<<tsn<<"\n";
	path->pathRcvdTSN->record(tsn);
	state->lastTsnReceived=tsn;
	SCTPSimpleMessage* smsg = check_and_cast <SCTPSimpleMessage*>(dataChunk->decapsulate());
	dataChunk->setBitLength(SCTP_DATA_CHUNK_LENGTH*8);
	dataChunk->encapsulate(smsg);
	uint32 payloadLength = dataChunk->getBitLength()/8-SCTP_DATA_CHUNK_LENGTH;
	sctpEV3<<"state->bytesRcvd="<<state->bytesRcvd<<"\n";
	state->bytesRcvd+=payloadLength;
	sctpEV3<<"state->bytesRcvd now="<<state->bytesRcvd<<"\n";
	SCTP::AssocStatMap::iterator iter=sctpMain->assocStatMap.find(assocId);
	iter->second.rcvdBytes+=dataChunk->getBitLength()/8-SCTP_DATA_CHUNK_LENGTH;
	
	if (state->numGaps == 0)
		state->highestTsnReceived = state->cTsnAck;
	else
		state->highestTsnReceived = state->gapStopList[state->numGaps-1];
	
	if (state->stopReceiving)
	{
		return SCTP_E_IGNORE;
	}
	
	if (tsnLe(tsn, state->cTsnAck))
	{

			sctpEV3<<"sctp_handle_incoming_data_chunk: old TSN value...inserted to duplist - returning\n tsn="<<tsn<<"  lastAcked="<<state->cTsnAck<<"\n";
			
			state->dupList.push_back(tsn);
			state->dupList.unique();
			delete check_and_cast <SCTPSimpleMessage*>(dataChunk->decapsulate());
			return SCTP_E_DUP_RECEIVED;
	}


	if ((int32)(state->localRwnd-state->queuedRcvBytes)<=0)
	{
		if (tsnGt(tsn, state->highestTsnReceived))
		{
			sctpEV3<<"sctp_handle_incoming_data_chunk: dropping new chunk due to full receive buffer -- wait for rwnd to open up\n";
			
			return SCTP_E_IGNORE;
		}
		// changed 06.07.05 I.R.
		else if (!tsnIsDuplicate(tsn) && tsn<state->highestTsnStored)
		{ 
			if (!makeRoomForTsn(tsn, dataChunk->getBitLength()-SCTP_DATA_CHUNK_LENGTH*8, dataChunk->getUBit()))
			{
				delete check_and_cast <SCTPSimpleMessage*>(dataChunk->decapsulate());
				return SCTP_E_IGNORE;
			}
			quBytes->record(state->queuedRcvBytes);
		}
	}
	if (tsnGt(tsn, state->highestTsnReceived)) 
	{
		sctpEV3<<"highestTsnReceived="<<state->highestTsnReceived<<"; tsn="<<tsn<<"\n";
		state->highestTsnReceived = state->highestTsnStored = tsn;
		if (tsn==initPeerTsn)
		{
			state->cTsnAck = tsn;
		}
		else
		{
		/* UPDATE fragment list   */
			sctpEV3<<"update fragment list\n";
			checkCtsnaChange = updateGapList(tsn);
		}
		state->newChunkReceived = true;		
	} 
	else if (tsnIsDuplicate(tsn)) 
	{	
		state->dupList.push_back(tsn);
		state->dupList.unique();
		 
		sctpEV3<<"sctp_handle_incoming_data_chunk: TSN value is duplicate within a fragment -- returning";
		
		return SCTP_E_IGNORE;		
	} 
	else 
	{
		sctpEV3<<"else: updateGapList\n";
		checkCtsnaChange = updateGapList(tsn);
	}
	
	if (state->swsAvoidanceInvoked) 
	{
		/* schedule a SACK to be sent at once in this case */
		sctpEV3<<"swsAvoidanceInvoked\n";
		state->ackState = sackFrequency;
	}		
		
	if (checkCtsnaChange) 
	{
		advanceCtsna();
	}
	event=SCTP_E_SEND; 
	
	sctpEV3<<"\ncTsnAck="<<state->cTsnAck<<" highestTsnReceived="<<state->highestTsnReceived<<"\n";
	
	if (state->newChunkReceived)
	{
		SCTPReceiveStreamMap::iterator iter=receiveStreams.find(dataChunk->getSid());

		if ((iter->second->enqueueNewDataChunk(makeVarFromMsg(dataChunk)))>0)
		{

	state->queuedRcvBytes+=payloadLength;

			event = SCTP_E_DELIVERED;
			sendDataArrivedNotification(dataChunk->getSid());
			putInDeliveryQ(dataChunk->getSid());
		}
		state->newChunkReceived=false;
	}
	

	return event;
}


SCTPEventCode  SCTPAssociation::processHeartbeatAckArrived(SCTPHeartbeatAckChunk* hback, SCTPPathVariables* path)
{
	simtime_t rttEstimate = 0.0;
	IPvXAddress addr;
	simtime_t hbTimeField = 0.0;

	/* hb-ack goes to pathmanagement, reset error counters, stop timeout timer */
	addr = hback->getRemoteAddr(); 
	hbTimeField = hback->getTimeField();
	stopTimer(path->HeartbeatTimer);
	/* assume a valid RTT measurement on this path */
	rttEstimate = simulation.getSimTime() - hbTimeField;
	pmRttMeasurement(addr, rttEstimate, 1);
	path->hbWasAcked = true;
	path->confirmed = true;
	path->lastAckTime = simulation.getSimTime();
	if (path->primaryPathCandidate == true)
	{
		state->primaryPathIndex = addr;
		path->primaryPathCandidate = false;
		if (path->pmtu < state->assocPmtu) 
			state->assocPmtu = path->pmtu;
		path->cwndAdjustmentTime = simulation.getSimTime();
		path->ssthresh = state->peerRwnd;
		path->pathSsthresh->record(path->ssthresh);
		path->heartbeatTimeout= (double)sctpMain->par("hbInterval")+path->pathRto;
	}
		
	if (path->activePath == false)
	{
		sctpEV3<<"HB-Ack arrived activePath=false. remoteAddress="<<path->remoteAddress<<" initialPP="<<state->initialPrimaryPath<<"\n";
		path->activePath = true;
		if (state->reactivatePrimaryPath && path->remoteAddress == state->initialPrimaryPath)
			state->primaryPathIndex = path->remoteAddress;
		sctpEV3<<"primaryPath now "<<state->primaryPathIndex<<"\n";
	}
	path->pathErrorCount = 0;
	return SCTP_E_IGNORE;
}



void SCTPAssociation::process_TIMEOUT_INIT_REXMIT(SCTPEventCode& event)
{

	if (++state->initRetransCounter > (int32)sctpMain->par("maxInitRetrans"))
	{
		sctpEV3 << "Retransmission count during connection setup exceeds " << (int32)sctpMain->par("maxInitRetrans") << ", giving up\n";
		sendIndicationToApp(SCTP_I_CLOSED);
		sendAbort();
		sctpMain->removeAssociation(this);	
		return;
	}
	
	sctpEV3<< "Performing retransmission #" << state->initRetransCounter << "\n";
	
	switch(fsm->getState())
	{
		case SCTP_S_COOKIE_WAIT: retransmitInit(); break;
		case SCTP_S_COOKIE_ECHOED: retransmitCookieEcho(); break;
		default:  opp_error("Internal error: INIT-REXMIT timer expired while in state %s", stateName(fsm->getState()));
	}
	
	state->initRexmitTimeout *= 2;
	
	if (state->initRexmitTimeout > SCTP_TIMEOUT_INIT_REXMIT_MAX)
		state->initRexmitTimeout = SCTP_TIMEOUT_INIT_REXMIT_MAX;
		
	startTimer(T1_InitTimer,state->initRexmitTimeout);
}

void SCTPAssociation::process_TIMEOUT_SHUTDOWN(SCTPEventCode& event)
{

	if (++state->errorCount > (uint32)sctpMain->par("assocMaxRetrans"))
	{
		sendIndicationToApp(SCTP_I_CONN_LOST);
		sendAbort();
		sctpMain->removeAssociation(this);
		return;
		
	} 
	
	sctpEV3 << "Performing shutdown retransmission. Assoc error count now "<<state->errorCount<<" \n";
	
	if(fsm->getState()==SCTP_S_SHUTDOWN_SENT)
	{
		retransmitShutdown(); 
	}
	else if (fsm->getState()==SCTP_S_SHUTDOWN_ACK_SENT)
		retransmitShutdownAck(); 
	
	state->initRexmitTimeout *= 2;
	
	if (state->initRexmitTimeout > SCTP_TIMEOUT_INIT_REXMIT_MAX)
		state->initRexmitTimeout = SCTP_TIMEOUT_INIT_REXMIT_MAX;
		
	startTimer(T2_ShutdownTimer,state->initRexmitTimeout);
}



void SCTPAssociation::process_TIMEOUT_CWND(SCTPPathVariables* path)
{
	/* 
	 *  When the association does not transmit data on a given transport address
	 *  within an RTO, the cwnd of the transport address SHOULD be adjusted to 2*MTU.
	 */

	path->cwnd = (int32)min(4 * path->pmtu, max(2 * path->pmtu, 4380));
	path->pathCwnd->record(path->cwnd);
	 
	sctpEV3<<"CWND timer run off: readjusting CWND(path=="<<path->remoteAddress<<") = "<<path->cwnd<<"\n";
	
	path->cwndAdjustmentTime = simulation.getSimTime();
}

void SCTPAssociation::process_TIMEOUT_HEARTBEAT_INTERVAL(SCTPPathVariables* path, bool force)
{

	 
	sctpEV3<<"HB Interval timer expired -- sending new HB REQ on path "<<path->remoteAddress<<"\n";
	
	/* restart hb_send_timer on this path */
	
	stopTimer(path->HeartbeatIntervalTimer);
	stopTimer(path->HeartbeatTimer);
	
	path->heartbeatIntervalTimeout = (double)sctpMain->par("hbInterval") +  path->pathRto;	
	path->heartbeatTimeout = path->pathRto;
	
	startTimer(path->HeartbeatIntervalTimer, path->heartbeatIntervalTimeout);
	
	if (simulation.getSimTime() - path->lastAckTime > path->heartbeatIntervalTimeout/2 || path->forceHb)
	{
		sendHeartbeat(path, false);
		startTimer(path->HeartbeatTimer, path->heartbeatTimeout);

		path->forceHb = false;
	}
	
}


void SCTPAssociation::process_TIMEOUT_HEARTBEAT(SCTPPathVariables* path)
{
	bool oldState;

	/* check if error counters must be increased */
	if (path->hbWasAcked)
	{
		sctpEV3<<"ERROR : HB Timeout timer expired for a path --> The Timer should already have been stopped !!!\n";

	} 
	else 
	{
		if (path->activePath)
		{
			state->errorCount++;
			path->pathErrorCount++;
			
			sctpEV3<<"HB timeout timer expired for path "<<path->remoteAddress<<" --> Increase Error Counters (Assoc: "<<state->errorCount<<", Path: "<<path->pathErrorCount<<")\n";
		}
	}
	
	/* RTO must be doubled for this path ! */
	path->pathRto = (simtime_t)min(2 * path->pathRto.dbl(), sctpMain->par("rtoMax"));
	path->pathRTO->record(path->pathRto);
	/* check if any thresholds are exceeded, and if so, check if ULP must be notified */
	if (state->errorCount > (uint32)sctpMain->par("assocMaxRetrans"))
	{
		sendIndicationToApp(SCTP_I_CONN_LOST);
		sendAbort();
		sctpMain->removeAssociation(this);
		return;
		
	} 
	else 
	{
		/* set path state to INACTIVE, if the path error counter is exceeded */
		if (path->pathErrorCount >  (uint32)sctpMain->par("pathMaxRetrans"))
		{		
			oldState = path->activePath;		
			path->activePath = false;
			if (path->remoteAddress == state->primaryPathIndex)
				state->primaryPathIndex = getNextAddress(path->remoteAddress);
			sctpEV3<<"pathErrorCount now "<<path->pathErrorCount<<"  PP now "<<state->primaryPathIndex<<"\n";
		}
		/* then: we can check, if all paths are INACTIVE ! */	
		if (allPathsInactive())
		{
			 
			sctpEV3<<"sctp_do_hb_to_timer() : ALL PATHS INACTIVE --> closing ASSOC\n";
			
			sendIndicationToApp(SCTP_I_CONN_LOST);
			return;

		} else if (path->activePath == false && oldState == true) 
		{
			/* notify the application, in case the PATH STATE has changed from ACTIVE to INACTIVE */
			pathStatusIndication(path->remoteAddress, false);
		}

	}		
}

void SCTPAssociation::stopTimers()
{
	for (SCTPPathMap::iterator j = sctpPathMap.begin(); j!=sctpPathMap.end(); j++)
	{	
		stopTimer(j->second->HeartbeatTimer);
		stopTimer(j->second->HeartbeatIntervalTimer);
	}
}

void SCTPAssociation::stopTimer(cMessage* timer)
{	
	 
	ev<<"stopTimer "<<timer->getName()<<"\n";
	
	if (timer->isScheduled())
	{
		cancelEvent(timer);
	}
}

void SCTPAssociation::startTimer(cMessage* timer, simtime_t timeout)
{
	 
	sctpEV3<<"startTimer "<<timer->getName()<<" with timeout "<<timeout<<" to expire at "<<simulation.getSimTime()+timeout<<"\n";
	
	scheduleTimeout(timer, timeout);
}



int32 SCTPAssociation::updateCounters(SCTPPathVariables* path)
{
	bool notifyUlp = false;
	
	if (++state->errorCount >=  (uint32)sctpMain->par("assocMaxRetrans"))
	{
		sctpEV3 << "Retransmission count during connection setup exceeds " << (int32)sctpMain->par("assocMaxRetrans") << ", giving up\n";
		sendIndicationToApp(SCTP_I_CLOSED);
		sendAbort();
		sctpMain->removeAssociation(this);	
		return 0;
	}
	else if (++path->pathErrorCount >=  (uint32)sctpMain->par("pathMaxRetrans")) 
	{
		if (path->activePath)
		{
			/* tell the source */
			notifyUlp = true;
		}

		path->activePath = false;
		if (path->remoteAddress == state->primaryPathIndex)
			state->primaryPathIndex = getNextAddress(path->remoteAddress); 
		sctpEV3<<"process_TIMEOUT_RESET("<<(path->remoteAddress)<<") : PATH ERROR COUNTER EXCEEDED, path status is INACTIVE\n";
		
		if (allPathsInactive())
		{
				
			sctpEV3<<"process_TIMEOUT_RESET : ALL PATHS INACTIVE --> closing ASSOC\n";
			
			sendIndicationToApp(SCTP_I_CONN_LOST);
			sendAbort();
			sctpMain->removeAssociation(this);
			return 0;
			
		} 
		else if (notifyUlp)
		{
			/* notify the application */
			pathStatusIndication(path->remoteAddress, false);	
		} 
		sctpEV3<<"process_TIMEOUT_RESET("<<(path->remoteAddress)<<") : PATH ERROR COUNTER now "<<path->pathErrorCount<<"\n";
		return 2;
	}
	return 1;
}




int32 SCTPAssociation::process_TIMEOUT_RTX(SCTPPathVariables* path)
{
	SCTPDataVariables* chunk = NULL;
	bool notifyUlp = false;

	/* increase the RTO (by doubling it) */
	path->pathRto = min(2*path->pathRto.dbl(),sctpMain->par("rtoMax"));
	path->pathRTO->record(path->pathRto);
	 
	sctpEV3<<"Schedule T3 based retransmission for path "<<(path->remoteAddress)<<"\n";
	

	(this->*ccFunctions.ccUpdateAfterRtxTimeout)(path);
	
	if (!state->zeroWindowProbing)
	{
		state->errorCount++;
		path->pathErrorCount++;
		sctpEV3<<"RTX-Timeout: errorCount increased to "<<path->pathErrorCount<<"  state->errorCount="<<state->errorCount<<"\n";
	}
	if (state->errorCount >=  (uint32)sctpMain->par("assocMaxRetrans"))
	{
		/* error counter exceeded terminate the association -- create an SCTPC_EV_CLOSE event and send it to myself */ 
		 
		sctpEV3<<"process_TIMEOUT_RTX : ASSOC ERROR COUNTER EXCEEDED, closing ASSOC\n";
		
		sendIndicationToApp(SCTP_I_CONN_LOST);
		sendAbort();
		sctpMain->removeAssociation(this);
		return 0;

	} 
	else 
	{
		if (path->pathErrorCount >=  (uint32)sctpMain->par("pathMaxRetrans")) 
		{
			sctpEV3<<"pathErrorCount exceeded\n";
			if (path->activePath)
			{
				/* tell the source */
				notifyUlp = true;
			}
			path->activePath = false;
			if (path->remoteAddress == state->primaryPathIndex)
			{
				IPvXAddress adr = getNextAddress(path->remoteAddress); 
				if (adr!=IPvXAddress("0.0.0.0"))
					state->primaryPathIndex = adr; 
			}
			sctpEV3<<"process_TIMEOUT_RTX("<<(path->remoteAddress)<<") : PATH ERROR COUNTER EXCEEDED, path status is INACTIVE\n";
			
			if (allPathsInactive())
			{
				 
				sctpEV3<<"process_TIMEOUT_RTX : ALL PATHS INACTIVE --> closing ASSOC\n";
				
				sendIndicationToApp(SCTP_I_CONN_LOST);
				sendAbort();
				sctpMain->removeAssociation(this);
				return 0;
				
			} 
			else if (notifyUlp)
			{
				/* notify the application */
				pathStatusIndication(path->remoteAddress, false);				
			}
		}
		 
		sctpEV3<<"process_TIMEOUT_RTX("<<(path->remoteAddress)<<") : PATH ERROR COUNTER now "<<path->pathErrorCount<<"\n";
		
	}

	/*======================================================================*/
	/* 						do retransmission 								*/
	/*======================================================================*/
	
	/* dequeue all chunks not acked so far and put them in the TransmissionQ */
	
	if (retransmissionQ->payloadQueue.empty()) return 1;		
	sctpEV3<<"Still "<<retransmissionQ->payloadQueue.size()<<" chunks in retransmissionQ\n";
	for (SCTPQueue::PayloadQueue::iterator iter=retransmissionQ->payloadQueue.begin(); iter!=retransmissionQ->payloadQueue.end(); iter++)
	{
		chunk = iter->second;
		if (chunk == NULL) 
		{ 
			sctpEV3<<"sctp assoc: chunk pointer is NULL in sctp_handle_t3_timeout()\n";
		}

		/*========================================================================*/
		/* only insert chunks that were sent to the path that has timed out 	  */
		/*========================================================================*/
		
		if ((chunk->hasBeenAcked == false && chunk->countsAsOutstanding || chunk->hasBeenRemoved )&& chunk->lastDestination == path->remoteAddress ) 
		{
			iter->second->hasBeenFastRetransmitted = false;
			iter->second->gapReports = 0;
			IPvXAddress oldpid = chunk->lastDestination;
			iter->second->nextDestination = getNextDestination(chunk);
			sctpEV3<<simulation.getSimTime()<<" RTX for TSN "<<iter->second->tsn<<": lastDestination="<<iter->second->lastDestination<<" nextDestination="<<iter->second->nextDestination<<"\n";
			/* check, if chunk_ptr->tsn is already in transmission queue */
			/* this can happen in case multiple timeouts occur in succession    */
			if (!transmissionQ->checkAndInsertVar(chunk->tsn, chunk))
			{
				sctpEV3<<"TSN "<<chunk->tsn<<" already in transmissionQ\n";
				continue;
			}
			else
			{
				sctpEV3<<"insert "<<chunk->tsn<<" in transmissionQ. hasBeenAcked="<<iter->second->hasBeenAcked<<" countsAsOutstanding="<<iter->second->countsAsOutstanding<<"\n";
				CounterMap::iterator q = qCounter.roomTransQ.find(iter->second->nextDestination);
				q->second += ADD_PADDING(chunk->len/8+SCTP_DATA_CHUNK_LENGTH);
				CounterMap::iterator qb = qCounter.bookedTransQ.find(iter->second->nextDestination);
				qb->second += chunk->booksize;
				sctpEV3<<"RTX-Timeout: "<<transmissionQ->getQueueSize()<<" chunks = "<<q->second<<" bytes. Osb="<<getPath(iter->second->lastDestination)->outstandingBytes<<"\n";

				if (iter->second->countsAsOutstanding)
				{
					getPath(iter->second->lastDestination)->outstandingBytes -= iter->second->booksize;
					sctpEV3<<"osb after substraction="<<getPath(iter->second->lastDestination)->outstandingBytes<<"\n";
					iter->second->countsAsOutstanding = false; 
				}
				CounterMap::iterator i=qCounter.roomRetransQ.find(getPath(iter->second->lastDestination)->remoteAddress);
				qb->second += chunk->booksize;
				sctpEV3<<"RTX-Timeout: "<<transmissionQ->getQueueSize()<<" chunks = "<<q->second<<" bytes. Osb="<<getPath(iter->second->lastDestination)->outstandingBytes<<"\n";

				if (iter->second->countsAsOutstanding)
				{
					getPath(iter->second->lastDestination)->outstandingBytes -= iter->second->booksize;
					sctpEV3<<"osb after substraction="<<getPath(iter->second->lastDestination)->outstandingBytes<<"\n";
					iter->second->countsAsOutstanding = false; 
				}
				i->second -= ADD_PADDING(iter->second->booksize+SCTP_DATA_CHUNK_LENGTH);
				
				state->peerRwnd += (chunk->booksize); 
				SCTP::AssocStatMap::iterator iter=sctpMain->assocStatMap.find(assocId);
				iter->second.numT3Rtx++;
			}
			if (state->peerRwnd > state->initialPeerRwnd)
				state->peerRwnd = state->initialPeerRwnd;
			sctpEV3<<"T3 Timeout: Chunk (TSN="<<chunk->tsn<<") has been requeued in transmission Q, rwnd was set to "<<state->peerRwnd<<"\n";
			
		}
	}

	IPvXAddress addr=getNextAddress(path->remoteAddress);
	sctpEV3<<"TimeoutRTX sendAll to addr: "<<addr<<"\n";
	sendAll(addr);
	if (addr != state->primaryPathIndex)
	{
		sctpEV3<<"TimeoutRTX sendAll to pp: "<<state->primaryPathIndex<<"\n";
		sendAll(state->primaryPathIndex);
	}
	else
	if (addr != path->remoteAddress)
	{
		sctpEV3<<"TimeoutRTX sendAll to remoteAddr: "<<remoteAddr<<"\n";
		sendAll(path->remoteAddress);
	}
	
	
	 
	sctpEV3<<"process_TIMEOUT_RTX sendAll returned\n";
	
	return 0;
}



