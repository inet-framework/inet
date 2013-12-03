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

#include "PIMNeighborTable.h"

Define_Module(PIMNeighborTable);

using namespace std;

PIMNeighbor::PIMNeighbor(InterfaceEntry *ie, IPv4Address address, int version)
    : ie(ie), address(address), version(version), nlt(NULL)
{
    ASSERT(ie);

    nlt = new PIMnlt("NeighborLivenessTimer");
    nlt->setTimerKind(NeighborLivenessTimer);
}


/** Printout of structure Neighbor table (PIMNeighbor). */
std::ostream& operator<<(std::ostream& os, const PIMNeighbor* e)
{
    os << e->getId() << ": ID = " << e->getInterfaceID() << "; Addr = " << e->getAddress() << "; Ver = " << e->getVersion();
    return os;
};

/** Printout of structure Neighbor table (PIMNeighbor). */
std::string PIMNeighbor::info() const
{
	std::stringstream out;
	out << this;
	return out.str();
}

PIMNeighborTable::~PIMNeighborTable()
{
    for (PIMNeighborVector::iterator it = nt.begin(); it != nt.end(); ++it)
        delete (*it);
}

/**
 * HANDLE MESSAGE
 *
 * Module does not have any gate, it cannot get messages
 */
void PIMNeighborTable::handleMessage(cMessage *msg)
{
    opp_error("This module doesn't process messages");
}

void PIMNeighborTable::initialize(int stage)
{
    if (stage == INITSTAGE_LOCAL)
    {
        WATCH_VECTOR(nt);
        id = 0;
    }
}

/**
 * PRINT PIM NEIGHBOR TABLE
 *
 * Printout of Table of PIM interfaces
 */
void PIMNeighborTable::printPimNeighborTable()
{
	for(PIMNeighborVector::iterator i = nt.begin(); i < nt.end(); i++)
	{
		EV << (*i)->info() << endl;
	}
}

void PIMNeighborTable::addNeighbor(PIMNeighbor *entry)
{
    entry->setId(id); this->nt.push_back(entry); id++;
}


/**
 * GET NEIGHBORS BY INTERFACE ID
 *
 * The method returns all neigbors which are connected to given router interface.
 *
 * @param intId Identifier of interface.
 * @return Vector of entries from PIM neighbor table.
 */
PIMNeighborTable::PIMNeighborVector PIMNeighborTable::getNeighborsByIntID(int intId)
{
	vector<PIMNeighbor*> nbr;

	for(int i = 0; i < getNumNeighbors(); i++)
	{
		if(intId == getNeighbor(i)->getInterfaceID())
		{
			nbr.push_back(getNeighbor(i));
		}
	}
	return nbr;
}

/**
 * GET NEIGHBOR BY ID
 *
 * The method returns pointer to neigbor which ais registered with given unique identifier.
 *
 * @param id Identifier of entry in the table.
 * @return Pointer to entry from PIM neighbor table.
 */
PIMNeighbor *PIMNeighborTable::getNeighborsByID(int id)
{
	for(int i = 0; i < getNumNeighbors(); i++)
	{
		if(id == getNeighbor(i)->getId())
		{
			return getNeighbor(i);
			break;
		}
	}
	return NULL;
}

PIMNeighbor *PIMNeighborTable::getNeighborByIntID(int intId)
{
    for(int i = 0; i < getNumNeighbors(); i++)
    {
        //int iddd = getNeighbor(i)->getInterfaceID();
        if(intId == getNeighbor(i)->getInterfaceID())
        {
            return getNeighbor(i);
            break;
        }
    }
    return NULL;
}

/**
 * DELETE NEIGHBOR
 *
 * The method removes entry with given unique identifier from the table.
 *
 * @param id Identifier of entry in the table.
 * @return True if entry was found and deleted successfully, otherwise false.
 */
bool PIMNeighborTable::deleteNeighbor(int id)
{
	for(int i = 0; i < getNumNeighbors(); i++)
	{
		if(id == getNeighbor(i)->getId())
		{
			nt.erase(nt.begin() + i);
			return true;
		}
	}
	return false;
}

/**
 * FIND NEIGHBOR
 *
 * The method finds entry in the table according given interface ID and neighbor IP address.
 *
 * @param intId Identifier of interface.
 * @param addr IP address of neighbor.
 * @return Pointer to entry if entry was found in the table, otherwise NULL.
 */
PIMNeighbor *PIMNeighborTable::findNeighbor(int intId, IPv4Address addr)
{
	for(int i = 0; i < getNumNeighbors(); i++)
	{
		if((addr == getNeighbor(i)->getAddress()) && (intId == getNeighbor(i)->getInterfaceID()))
			return getNeighbor(i);
	}
	return NULL;
}

/**
 * GET NUMBER OF NEIGHBORS ON INTERFACE
 *
 * The method returns number of neighbors which are connected to given interface.
 *
 * @param intId Identifier of interface.
 * @return Number of neighbors which are connected to given interface.
 */
int PIMNeighborTable::getNumNeighborsOnInt(int intId)
{
	PIMNeighborVector neighbors = getNeighborsByIntID(intId);
	return neighbors.size();
}
