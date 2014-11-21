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

#include "inet/networklayer/pim/tables/PIMNeighborTable.h"

namespace inet {
Register_Abstract_Class(PIMNeighbor);
Define_Module(PIMNeighborTable);

using namespace std;

PIMNeighbor::PIMNeighbor(InterfaceEntry *ie, IPv4Address address, int version)
    : nt(NULL), ie(ie), address(address), version(version), generationId(0), drPriority(-1L)
{
    ASSERT(ie);

    livenessTimer = new cMessage("NeighborLivenessTimer", PIMNeighborTable::NeighborLivenessTimer);
    livenessTimer->setContextPointer(this);
}

PIMNeighbor::~PIMNeighbor()
{
    ASSERT(!livenessTimer->isScheduled());
    delete livenessTimer;
}

// for WATCH_MAP()
std::ostream& operator<<(std::ostream& os, const PIMNeighborTable::PIMNeighborVector& v)
{
    for (unsigned int i = 0; i < v.size(); i++) {
        PIMNeighbor *e = v[i];
        os << "[" << i << "]: "
           << "{ If = " << e->getInterfacePtr()->getName() << "; Addr = " << e->getAddress() << "; Ver = " << e->getVersion()
           << "; GenID = " << e->getGenerationId() << "; Pr = " << e->getDRPriority() << "}; ";
    }
    return os;
};

std::string PIMNeighbor::info() const
{
    std::stringstream out;
    out << this;
    return out.str();
}

void PIMNeighbor::changed()
{
    if (nt)
        nt->emit(NF_PIM_NEIGHBOR_CHANGED, this);
}

PIMNeighborTable::~PIMNeighborTable()
{
    for (InterfaceToNeighborsMap::iterator it = neighbors.begin(); it != neighbors.end(); ++it) {
        for (PIMNeighborVector::iterator it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
            cancelEvent((*it2)->getLivenessTimer());
            delete (*it2);
        }
    }
}

void PIMNeighborTable::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        WATCH_MAP(neighbors);
    }
}

void PIMNeighborTable::handleMessage(cMessage *msg)
{
    // self message (timer)
    if (msg->isSelfMessage()) {
        if (msg->getKind() == NeighborLivenessTimer) {
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
    PIMNeighbor *neighbor = check_and_cast<PIMNeighbor *>((cObject *)livenessTimer->getContextPointer());
    IPv4Address neighborAddress = neighbor->getAddress();
    deleteNeighbor(neighbor);
    EV << "PIM::processNLTimer: Neighbor " << neighborAddress << "was removed from PIM neighbor table.\n";
}

bool PIMNeighborTable::addNeighbor(PIMNeighbor *entry, double holdTime)
{
    Enter_Method_Silent();

    PIMNeighborVector& neighborsOnInterface = neighbors[entry->getInterfaceId()];

    for (PIMNeighborVector::iterator it = neighborsOnInterface.begin(); it != neighborsOnInterface.end(); it++)
        if ((*it)->getAddress() == entry->getAddress())
            return false;


    EV_DETAIL << "Added new neighbor to table: " << entry->info() << "\n";
    entry->nt = this;
    neighborsOnInterface.push_back(entry);
    take(entry->getLivenessTimer());
    restartLivenessTimer(entry, holdTime);

    emit(NF_PIM_NEIGHBOR_ADDED, entry);

    return true;
}

bool PIMNeighborTable::deleteNeighbor(PIMNeighbor *neighbor)
{
    Enter_Method_Silent();

    InterfaceToNeighborsMap::iterator it = neighbors.find(neighbor->getInterfaceId());
    if (it != neighbors.end()) {
        PIMNeighborVector& neighborsOnInterface = it->second;
        PIMNeighborVector::iterator it2 = find(neighborsOnInterface.begin(), neighborsOnInterface.end(), neighbor);
        if (it2 != neighborsOnInterface.end()) {
            neighborsOnInterface.erase(it2);

            emit(NF_PIM_NEIGHBOR_DELETED, neighbor);

            neighbor->nt = NULL;
            cancelEvent(neighbor->getLivenessTimer());
            delete neighbor;

            return true;
        }
    }
    return false;
}

void PIMNeighborTable::restartLivenessTimer(PIMNeighbor *neighbor, double holdTime)
{
    Enter_Method_Silent();
    cancelEvent(neighbor->getLivenessTimer());
    scheduleAt(simTime() + holdTime, neighbor->getLivenessTimer());
}

PIMNeighbor *PIMNeighborTable::findNeighbor(int interfaceId, IPv4Address addr)
{
    InterfaceToNeighborsMap::iterator neighborsOnInterface = neighbors.find(interfaceId);
    if (neighborsOnInterface != neighbors.end()) {
        for (PIMNeighborVector::iterator it = neighborsOnInterface->second.begin(); it != neighborsOnInterface->second.end(); ++it)
            if ((*it)->getAddress() == addr && (*it)->getInterfaceId() == interfaceId)
                return *it;

    }
    return NULL;
}

int PIMNeighborTable::getNumNeighbors(int interfaceId)
{
    InterfaceToNeighborsMap::iterator it = neighbors.find(interfaceId);
    return it != neighbors.end() ? it->second.size() : 0;
}

PIMNeighbor *PIMNeighborTable::getNeighbor(int interfaceId, int index)
{
    InterfaceToNeighborsMap::iterator it = neighbors.find(interfaceId);
    return it != neighbors.end() ? it->second.at(index) : NULL;
}
}    // namespace inet

