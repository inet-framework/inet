//
// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//
// Authors: Veronika Rybova, Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)

#include <algorithm>
#include "PIMNeighborTable.h"

Define_Module(PIMNeighborTable);

using namespace std;

PIMNeighbor::PIMNeighbor(InterfaceEntry *ie, IPv4Address address, int version)
    : ie(ie), address(address), version(version)
{
    ASSERT(ie);

    livenessTimer = new cMessage("NeighborLivenessTimer", NeighborLivenessTimer);
    livenessTimer->setContextPointer(this);
}

PIMNeighbor::~PIMNeighbor()
{
    ASSERT(!livenessTimer->isScheduled());
    delete livenessTimer;
}

// for WATCH_VECTOR()
std::ostream& operator<<(std::ostream& os, const PIMNeighbor* e)
{
    os << "Neighbor: If = " << e->getInterfacePtr()->getName() << "; Addr = " << e->getAddress() << "; Ver = " << e->getVersion();
    return os;
};

std::string PIMNeighbor::info() const
{
	std::stringstream out;
	out << this;
	return out.str();
}

PIMNeighborTable::~PIMNeighborTable()
{
    for (PIMNeighborVector::iterator it = neighbors.begin(); it != neighbors.end(); ++it)
    {
        cancelEvent((*it)->getLivenessTimer());
        delete (*it);
    }
}

void PIMNeighborTable::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL)
    {
        WATCH_VECTOR(neighbors);
    }
}

void PIMNeighborTable::handleMessage(cMessage *msg)
{
    // self message (timer)
   if (msg->isSelfMessage())
   {
       if (msg->getKind() == NeighborLivenessTimer)
       {
           processLivenessTimer(msg);
       }
       else
           ASSERT(false);
   }
   else
       throw cRuntimeError("PIMNeighborTable received a message although it does not have gates.");
}

void PIMNeighborTable::processLivenessTimer(cMessage *livenessTimer)
{
    EV << "PIMNeighborTable::processNLTimer\n";
    PIMNeighbor *neighbor = check_and_cast<PIMNeighbor*>((cObject*)livenessTimer->getContextPointer());
    IPv4Address neighborAddress = neighbor->getAddress();
    deleteNeighbor(neighbor);
    EV << "PIM::processNLTimer: Neighbor " << neighborAddress << "was removed from PIM neighbor table.\n";
}

bool PIMNeighborTable::addNeighbor(PIMNeighbor *entry)
{
    Enter_Method_Silent();
    if (!findNeighbor(entry->getInterfaceId(), entry->getAddress()))
    {
        this->neighbors.push_back(entry);
        take(entry->getLivenessTimer());
        restartLivenessTimer(entry);
        return true;
    }
    else
        return false;
}

bool PIMNeighborTable::deleteNeighbor(PIMNeighbor* neighbor)
{
    Enter_Method_Silent();
    PIMNeighborVector::iterator it = find(neighbors.begin(), neighbors.end(), neighbor);
    if (it != neighbors.end())
    {
        cancelEvent((*it)->getLivenessTimer());
        delete (*it);
        neighbors.erase(it);
        return true;
    }
    else
        return false;
}

#define HT 30.0                                     /**< Hello Timer = 30s. */

void PIMNeighborTable::restartLivenessTimer(PIMNeighbor *neighbor)
{
    Enter_Method_Silent();
    cancelEvent(neighbor->getLivenessTimer());
    scheduleAt(simTime() + 3.5*HT, neighbor->getLivenessTimer()); // XXX should use Hold Time from Hello Message
}

PIMNeighbor *PIMNeighborTable::findNeighbor(int interfaceId, IPv4Address addr)
{
    for(PIMNeighborVector::iterator it = neighbors.begin(); it != neighbors.end(); ++it)
        if((*it)->getAddress() == addr && (*it)->getInterfaceId() == interfaceId)
            return *it;
    return NULL;
}

PIMNeighborTable::PIMNeighborVector PIMNeighborTable::getNeighborsOnInterface(int interfaceId)
{
    PIMNeighborVector result;
	for(PIMNeighborVector::iterator it = neighbors.begin(); it != neighbors.end(); ++it)
		if((*it)->getInterfaceId() == interfaceId)
			result.push_back(*it);
	return result;
}

PIMNeighbor *PIMNeighborTable::getFirstNeighborOnInterface(int interfaceId)
{
    for(PIMNeighborVector::iterator it = neighbors.begin(); it != neighbors.end(); ++it)
    {
        if((*it)->getInterfaceId() == interfaceId)
        {
            return *it;
            break;
        }
    }
    return NULL;
}

int PIMNeighborTable::getNumNeighborsOnInterface(int interfaceId)
{
    int result = 0;
    for (PIMNeighborVector::iterator it = neighbors.begin(); it != neighbors.end(); ++it)
        if ((*it)->getInterfaceId() == interfaceId)
            result++;
	return result;
}

