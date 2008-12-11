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
#include "IPControlInfo_m.h"
#include "SCTPAlgorithm.h"

//
// Event processing code
//

void SCTPAssociation::process_ASSOCIATE(SCTPEventCode& event, SCTPCommand *sctpCommand, cPacket *msg)
{
IPvXAddress lAddr, rAddr;

	SCTPOpenCommand *openCmd = check_and_cast<SCTPOpenCommand *>(sctpCommand);
	 
	ev<<"SCTPAssociationEventProc:process_ASSOCIATE\n";
	
	switch(fsm->getState())
	{
		case SCTP_S_CLOSED:
		initAssociation(openCmd);
		state->active = true;
		localAddressList = openCmd->getLocalAddresses();
		lAddr = openCmd->getLocalAddresses().front();
		if (!(openCmd->getRemoteAddresses().empty()))
		{
			remoteAddressList = openCmd->getRemoteAddresses();
			rAddr = openCmd->getRemoteAddresses().front();
		}
		else
			rAddr = openCmd->getRemoteAddr();
		localPort = openCmd->getLocalPort();
		remotePort = openCmd->getRemotePort();
		state->numRequests = openCmd->getNumRequests();
		
		if (rAddr.isUnspecified() || remotePort==0)
		opp_error("Error processing command OPEN_ACTIVE: remote address and port must be specified");

		if (localPort==0)
		{
		localPort = sctpMain->getEphemeralPort();
		}	
		ev << "OPEN: " << lAddr << ":" << localPort << " --> " << rAddr << ":" << remotePort << "\n";

		sctpMain->updateSockPair(this, lAddr, rAddr, localPort, remotePort);
		state->localRwnd = (long)sctpMain->par("arwnd");
		sendInit();
		startTimer(T1_InitTimer,state->initRexmitTimeout); 
		break;

	default:
		opp_error("Error processing command OPEN_ACTIVE: connection already exists");
}

}

void SCTPAssociation::process_OPEN_PASSIVE(SCTPEventCode& event, SCTPCommand *sctpCommand, cPacket *msg)
{
	IPvXAddress lAddr;
	int16 localPort;
    
	SCTPOpenCommand *openCmd = check_and_cast<SCTPOpenCommand *>(sctpCommand);

	sctpEV3<<"SCTPAssociationEventProc:process_OPEN_PASSIVE\n";
	
	switch(fsm->getState())
	{
		case SCTP_S_CLOSED:
			initAssociation(openCmd);
			state->fork = openCmd->getFork();
			localAddressList = openCmd->getLocalAddresses();
			sctpEV3<<"process_OPEN_PASSIVE: number of local addresses="<<localAddressList.size()<<"\n";
			lAddr = openCmd->getLocalAddresses().front();
			localPort = openCmd->getLocalPort();
			state->localRwnd = (long)sctpMain->par("arwnd");
			state->numRequests = openCmd->getNumRequests();
			state->messagesToPush = openCmd->getMessagesToPush();
			
			if (localPort==0)
				opp_error("Error processing command OPEN_PASSIVE: local port must be specified");
			sctpEV3 << "Assoc "<<assocId<<"::Starting to listen on: " << lAddr << ":" << localPort << "\n"; 
		
			sctpMain->updateSockPair(this, lAddr, IPvXAddress(), localPort, 0);
			break;
		default:
		opp_error("Error processing command OPEN_PASSIVE: connection already exists");
	}
}

void SCTPAssociation::process_SEND(SCTPEventCode& event, SCTPCommand *sctpCommand, cPacket *msg)
{
uint32 pkSize=0, streamId, sendUnordered, ppid=0;
SCTPSendStream* stream;

	SCTPSendCommand *sendCommand = check_and_cast<SCTPSendCommand *>(sctpCommand);
	switch(fsm->getState())
	{
		case SCTP_S_ESTABLISHED:  
			 
			sctpEV3<<"SCTPAssociationEventProc: process_SEND  localAddr="<<localAddr<<"  remoteAddr="<<remoteAddr<<"  appGateIndex="<<appGateIndex<<"  assocId="<<assocId<<"\n";	
			
			SCTPSimpleMessage* smsg = check_and_cast<SCTPSimpleMessage*>((msg->decapsulate()));
			SCTP::AssocStatMap::iterator iter=sctpMain->assocStatMap.find(assocId);
			iter->second.sentBytes+=smsg->getBitLength()/8;	
			pkSize = smsg->getBitLength()+SCTP_DATA_CHUNK_LENGTH*8;
			
				/* check that message is shorter than the MSS, else use segmentation */
			if (pkSize <= SCTP_MAX_PAYLOAD * 8) 
			{
				streamId = sendCommand->getSid();
				sendUnordered = sendCommand->getSendUnordered();
				ppid = sendCommand->getPpid();
				SCTPDataMsg* datMsg = new SCTPDataMsg();
				SCTPSendStreamMap::iterator iter=sendStreams.find(streamId);
				if (iter!=sendStreams.end())
					stream=iter->second;
				else
					opp_error("stream with id %d not found",streamId);
				char stri[20];
				sprintf(stri, "SDATA-%d-%d",streamId,state->msgNum);
				smsg->setName(stri);
				datMsg->encapsulate(smsg);
				datMsg->setSid(streamId);
				datMsg->setPpid(ppid);
				if (sendCommand->getPrimary())
				{
					if (sendCommand->getRemoteAddr()==IPvXAddress("0.0.0.0"))
						datMsg->setInitialDestination(remoteAddr);
					else
						datMsg->setInitialDestination(sendCommand->getRemoteAddr());
				}
				else
					datMsg->setInitialDestination(state->primaryPathIndex);
				datMsg->setEnqueuingTime(simulation.getSimTime());
					datMsg->setBooksize(smsg->getBitLength()/8 + state->header);
				qCounter.roomSumSendStreams += ADD_PADDING(smsg->getBitLength()/8 + SCTP_DATA_CHUNK_LENGTH);

				qCounter.bookedSumSendStreams += datMsg->getBooksize();
				datMsg->setMsgNum(++state->msgNum);

				if (sendUnordered==1)
				{
					datMsg->setOrdered(false);
					stream->getUnorderedStreamQ()->insert(datMsg);
				}
				else
				{ 
					datMsg->setOrdered(true);
					stream->getStreamQ()->insert(datMsg);

					if (stream->getStreamQ()->getLength()==state->sendQueueLimit)
					{
						sendIndicationToApp(SCTP_I_SENDQUEUE_FULL);
						state->appSendAllowed = false;
					}
					sendQueue->record(stream->getStreamQ()->getLength());
				}
				state->queuedMessages++;	
				if (state->queueLimit>0 && state->queuedMessages>state->queueLimit)
				{
					state->queueUpdate = false;
				}
				sctpEV3<<"\nlast="<<sendCommand->getLast()<<", queueLimit="<<state->queueLimit<<"\n";
				if (sendCommand->getLast()==true)
				{
					if (sendCommand->getPrimary())
						sctpAlgorithm->sendCommandInvoked(IPvXAddress("0.0.0.0"));
					else
						sctpAlgorithm->sendCommandInvoked(datMsg->getInitialDestination());
				}
			}
		break;
	}
}

void SCTPAssociation::process_RECEIVE_REQUEST(SCTPEventCode& event, SCTPCommand *sctpCommand)
{
 	SCTPSendCommand *sendCommand = check_and_cast<SCTPSendCommand *>(sctpCommand);
	if ((uint32)sendCommand->getSid() > inboundStreams || sendCommand->getSid() < 0) 
	{
		 
		sctpEV3<<"Application tries to read from invalid stream id....\n";	
		
	}
	 
	state->numMsgsReq[sendCommand->getSid()]+= sendCommand->getNumMsgs();
	pushUlp();
}

void SCTPAssociation::process_PRIMARY(SCTPEventCode& event, SCTPCommand *sctpCommand)
{
	SCTPPathInfo *pinfo = check_and_cast<SCTPPathInfo *>(sctpCommand);
	state->primaryPathIndex = pinfo->getRemoteAddress();
}


void SCTPAssociation::process_QUEUE(SCTPCommand *sctpCommand)
{
	SCTPInfo* qinfo = check_and_cast<SCTPInfo*>(sctpCommand);
	state->queueLimit = qinfo->getText();
}

void SCTPAssociation::process_CLOSE(SCTPEventCode& event)
{
	 
	sctpEV3<<"SCTPAssociationEventProc:process_CLOSE; assoc="<<assocId<<"\n";
	
	switch(fsm->getState())
	{
		case SCTP_S_ESTABLISHED: 
			sendAll(state->primaryPathIndex);
				
			if (remoteAddr!=state->primaryPathIndex)
				sendAll(remoteAddr);
					
			sendShutdown();
			break;
		case SCTP_S_SHUTDOWN_RECEIVED:
			if (getOutstandingBytes()==0)
				sendShutdownAck(remoteAddr);
			break;         
    }
}

void SCTPAssociation::process_ABORT(SCTPEventCode& event)
{
	sctpEV3<<"SCTPAssociationEventProc:process_ABORT\n";
	switch(fsm->getState())
	{
		case SCTP_S_ESTABLISHED: 
			if (state->ackState < sackFrequency)
			{
				state->ackState = sackFrequency;	
				sendAll(state->primaryPathIndex);
				if (remoteAddr!=state->primaryPathIndex)
					sendAll(remoteAddr);	
			}
			sendAbort();
			break;         
    	}

}

void SCTPAssociation::process_STATUS(SCTPEventCode& event, SCTPCommand *sctpCommand, cPacket *msg)
{
	SCTPStatusInfo *statusInfo = new SCTPStatusInfo();
	statusInfo->setState(fsm->getState());
	statusInfo->setStateName(stateName(fsm->getState()));
	
	statusInfo->setPathId(remoteAddr);
	statusInfo->setActive(getPath(remoteAddr)->activePath);
	
	msg->setControlInfo(statusInfo);
	sendToApp(msg);
}


