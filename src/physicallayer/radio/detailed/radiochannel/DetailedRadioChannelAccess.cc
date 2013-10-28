/***************************************************************************
 * file:        ConnectionManagerAccess.cc
 *
 * author:      Marc Loebbers
 *

 * copyright:   (C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 ***************************************************************************
 * part of:     framework implementation developed by tkn
 * description: - Base class for physical layers
 *              - if you create your own physical layer, please subclass
 *                from this class and use the sendToChannel() function!!
 ***************************************************************************
 * changelog:   $Revision: 284 $
 *              last modified:   $Date: 2006-06-07 16:55:24 +0200 (Mi, 07 Jun 2006) $
 *              by:              $Author: willkomm $
 **************************************************************************/

#include "InitStages.h"
#include "DetailedRadioChannelAccess.h"
#include "FindModule.h"
#include "ModuleAccess.h"
#include "BaseConnectionManager.h"

using std::endl;

BaseConnectionManager* DetailedRadioChannelAccess::getConnectionManager(const cModule* nic)
{
	std::string cmName = nic->hasPar("connectionManagerName")
						 ? nic->par("connectionManagerName").stringValue()
						 : "";
	if (cmName != "") {
		cModule* ccModule = simulation.getModuleByPath(cmName.c_str());

		return dynamic_cast<BaseConnectionManager *>(ccModule);
	}
	else {
		return FindModule<BaseConnectionManager *>::findGlobalModule();
	}
}

void DetailedRadioChannelAccess::initialize( int stage )
{
    RadioBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL){
        findContainingNode(this)->subscribe(IMobility::mobilityStateChangedSignal, this);

        cc = getConnectionManager(getNic());
        if( cc == NULL ) error("Could not find ConnectionManager module");
        isRegistered = false;
    }

    usePropagationDelay = par("usePropagationDelay");
}



void DetailedRadioChannelAccess::sendToChannel(cPacket *msg)
{
    const NicEntry::GateList& gateList = cc->getGateList( getNic()->getId());
    NicEntry::GateList::const_iterator i = gateList.begin();

    if(useSendDirect){
        // use Andras stuff
        if( i != gateList.end() ){
        	simtime_t delay = SIMTIME_ZERO;
            for(; i != --gateList.end(); ++i){
            	//calculate delay (Propagation) to this receiving nic
            	delay = calculatePropagationDelay(i->first);

                int radioStart = i->second->getId();
                int radioEnd = radioStart + i->second->size();
                for (int g = radioStart; g != radioEnd; ++g)
                    sendDirect(static_cast<cPacket*>(msg->dup()),
                               delay, msg->getDuration(), i->second->getOwnerModule(), g);
            }
            //calculate delay (Propagation) to this receiving nic
			delay = calculatePropagationDelay(i->first);

            int radioStart = i->second->getId();
            int radioEnd = radioStart + i->second->size();
            for (int g = radioStart; g != --radioEnd; ++g)
                sendDirect(static_cast<cPacket*>(msg->dup()),
                           delay, msg->getDuration(), i->second->getOwnerModule(), g);

            sendDirect(msg, delay, msg->getDuration(), i->second->getOwnerModule(), radioEnd);
        }
        else{
            EV_INFO << "Nic is not connected to any gates!" << endl;
            delete msg;
        }
    }
    else{
        // use our stuff
        EV_INFO << "sendToChannel: sending to gates\n";
        if( i != gateList.end() ){
        	simtime_t delay = SIMTIME_ZERO;
            for(; i != --gateList.end(); ++i){
            	//calculate delay (Propagation) to this receiving nic
				delay = calculatePropagationDelay(i->first);

                sendDelayed( static_cast<cPacket*>(msg->dup()),
                             delay, i->second );
            }
            //calculate delay (Propagation) to this receiving nic
			delay = calculatePropagationDelay(i->first);

            sendDelayed( msg, delay, i->second );
        }
        else{
            EV_INFO << "Nic is not connected to any gates!" << endl;
            delete msg;
        }
    }
}

simtime_t DetailedRadioChannelAccess::calculatePropagationDelay(const NicEntry* nic) {
	if(!usePropagationDelay)
		return 0;

	DetailedRadioChannelAccess *const senderModule   = this;
	DetailedRadioChannelAccess *const receiverModule = nic->chAccess;
	//const simtime_t_cref sStart         = simTime();

	ASSERT(senderModule);
	ASSERT(receiverModule);

	/** claim the Move pattern of the sender from the Signal */
	Coord           sendersPos  = senderModule->getMobility()->getCurrentPosition(/*sStart*/);
	Coord           receiverPos = receiverModule->getMobility()->getCurrentPosition(/*sStart*/);

	// this time-point is used to calculate the distance between sending and receiving host
	return receiverPos.distance(sendersPos) / SPEED_OF_LIGHT;
}

void DetailedRadioChannelAccess::receiveSignal(cComponent */*source*/, simsignal_t signalID, cObject *obj)
{
    if (signalID == IMobility::mobilityStateChangedSignal) {
    	IMobility * const mobility = check_and_cast<IMobility *>(obj);
        Coord                        pos      = mobility->getCurrentPosition();

        if(isRegistered) {
            cc->updateNicPos(getNic()->getId(), &pos);
        }
        else {
            // register the nic with ConnectionManager
            // returns true, if sendDirect is used
            useSendDirect = cc->registerNic(getNic(), this, &pos);
            isRegistered  = true;
        }
    }
}
