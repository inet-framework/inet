/* -*- mode:c++ -*- ********************************************************
 * file:        NicEntryDebug.cc
 *
 * author:      Daniel Willkomm
 *
 * copyright:   (C) 2005 Telecommunication Networks Group (TKN) at
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
 * description: Class to store information about a nic for the
 *              ConnectionManager module
 **************************************************************************/

#include "NicEntryDebug.h"

#include <cassert>

#include "DetailedRadioChannelAccess.h"
#include "FindModule.h"

using std::endl;

void NicEntryDebug::connectTo(NicEntry* other)
{
	EV_DEBUG << "connecting nic #"<<nicId<< " and #"<<other->nicId<<endl;

	NicEntryDebug* otherNic = (NicEntryDebug*) other;

	cGate *localoutgate = requestOutGate();
	localoutgate->connectTo(otherNic->requestInGate());
	outConns[other] = localoutgate->getPathStartGate();
}

void NicEntryDebug::disconnectFrom(NicEntry* other)
{
	EV_DEBUG << "disconnecting nic #"<<nicId<< " and #"<<other->nicId<<endl;

	NicEntryDebug* otherNic = (NicEntryDebug*) other;

	//search the connection in the outConns list
	GateList::iterator p = outConns.find(other);
	//no need to check whether entry is valid; is already check by ConnectionManager isConnected
	//get the hostGate
	//order is phyGate->nicGate->hostGate
	cGate* hostGate = p->second->getNextGate()->getNextGate();

	// release local out gate
	freeOutGates.push_back(hostGate);

	// release remote in gate
	otherNic->freeInGates.push_back(hostGate->getNextGate());

	//reset gates
	//hostGate->getNextGate()->connectTo(0);
	hostGate->disconnect();

	//delete the connection
	outConns.erase(p);
}

int NicEntryDebug::collectGates(const char* pattern, GateStack& gates)
{
	cModule* host = FindModule<>::findHost(nicPtr);
	int i = 1;
	char gateName[20];
	//create the unique name for the gate (composed of the nic module id and a counter)
	sprintf(gateName, pattern, nicId, i);
	while(host->hasGate(gateName))
	{
		cGate* hostGate = host->gate(gateName);
		if(hostGate->isConnectedOutside()) {
			opp_error("Gate %s is still connected but not registered with this "
					  "NicEntry. Either the last NicEntry for this NIC did not "
					  "clean up correctly or another gate creation module is "
					  "interfering with this one!", gateName);
		}
		assert(hostGate->isConnectedInside());
		gates.push_back(hostGate);

		++i;
		sprintf(gateName, pattern, nicId, i);
	}

	return i - 1;
}

void NicEntryDebug::collectFreeGates()
{
	if(!checkFreeGates)
		return;

	inCnt = collectGates("in%d-%d", freeInGates);
	EV_DEBUG << "found " << inCnt << " already existing usable in-gates." << endl;

	EV_DEBUG << "found " << inCnt << " already existing usable out-gates." << endl;

	checkFreeGates = false;
}

cGate* NicEntryDebug::requestInGate(void)
{
	collectFreeGates();

	// gate of the host
	cGate *hostGate;

	if (!freeInGates.empty()) {
		hostGate = freeInGates.back();
		freeInGates.pop_back();
	} else {
		char gateName[20];

		// we will have one more in gate
		++inCnt;

		//get a unique name for the gate (composed of the nic module id and a counter)
		sprintf(gateName, "in%d-%d", nicId, inCnt);

		// create a new gate for the host module
		cModule* const pHost = FindModule<>::findHost(nicPtr);
		pHost->addGate(gateName, cGate::INPUT);
		hostGate = pHost->gate(gateName);

		// gate of the nic
		cGate *nicGate;

		// create a new gate for the nic module
		nicPtr->addGate(gateName, cGate::INPUT);
		nicGate = nicPtr->gate(gateName);

		// connect the hist gate with the nic gate
		hostGate->connectTo(nicGate);

		// pointer to the phy module
                DetailedRadioChannelAccess* phyModule;
		// gate of the phy module
		cGate *phyGate;

		// to avoid unnecessary dynamic_casting we check for a "phy"-named submodule first
		if ((phyModule = dynamic_cast<DetailedRadioChannelAccess *> (nicPtr->getSubmodule("phy"))) == NULL)
			phyModule = FindModule<DetailedRadioChannelAccess*>::findSubModule(nicPtr);
		assert(phyModule != 0);

		// create a new gate for the phy module
		phyModule->addGate(gateName, cGate::INPUT);
		phyGate = phyModule->gate(gateName);

		// connect the nic gate (the gate of the compound module) to
		// a "real" gate -- the gate of the phy module
		nicGate->connectTo(phyGate);
	}

	return hostGate;
}

cGate* NicEntryDebug::requestOutGate(void)
{
	collectFreeGates();

	// gate of the host
	cGate *hostGate;

	if (!freeOutGates.empty()) {
		hostGate = freeOutGates.back();
		freeOutGates.pop_back();
	} else {
		char gateName[20];

		// we will have one more out gate
		++outCnt;

		//get a unique name for the gate (composed of the nic module id and a counter)
		sprintf(gateName, "out%d-%d", nicId, outCnt);

		// create a new gate for the host module
		cModule* const pHost = FindModule<>::findHost(nicPtr);
		pHost->addGate(gateName, cGate::OUTPUT);
		hostGate = pHost->gate(gateName);

		// gate of the nic
		cGate *nicGate;
		// create a new gate for the nic module
		nicPtr->addGate(gateName, cGate::OUTPUT);
		nicGate = nicPtr->gate(gateName);

		// connect the hist gate with the nic gate
		nicGate->connectTo(hostGate);

		// pointer to the phy module
		DetailedRadioChannelAccess* phyModule;
		// gate of the phy module
		cGate *phyGate;

		// to avoid unnecessary dynamic_casting we check for a "phy"-named submodule first
		if ((phyModule = dynamic_cast<DetailedRadioChannelAccess *> (nicPtr->getSubmodule("phy"))) == NULL)
			phyModule = FindModule<DetailedRadioChannelAccess*>::findSubModule(nicPtr);
		assert(phyModule != 0);

		// create a new gate for the phy module
		phyModule->addGate(gateName, cGate::OUTPUT);
		phyGate = phyModule->gate(gateName);

		// connect the nic gate (the gate of the compound module) to
		// a "real" gate -- the gate of the phy module
		phyGate->connectTo(nicGate);
	}

	return hostGate;
}
