// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
/**
 * @file PimNeighborTable.cc
 * @date 19.3.2012
 * @author: Veronika Rybova, Vladimir Vesely (mailto:ivesely@fit.vutbr.cz)
 * @brief File implements table of PIM neighbors.
 * @details Table of neighbors contain information about all PIM neghbor routers which has
 *  also configured PIM protocol. Information about neighbors are obtained from
 *  Hello messages.
 */

#include "PimNeighborTable.h"

Define_Module(PimNeighborTable);

using namespace std;


/** Printout of structure Neighbor table (PimNeighbor). */
std::ostream& operator<<(std::ostream& os, const PimNeighbor& e)
{
    os << e.getId() << ": ID = " << e.getInterfaceID() << "; Addr = " << e.getAddr() << "; Ver = " << e.getVersion();
    return os;
};

/** Printout of structure Neighbor table (PimNeighbor). */
std::string PimNeighbor::info() const
{
	std::stringstream out;
	out << id << ": ID = " << intID << "; Addr = " << addr << "; Ver = " << ver;
	return out.str();
}

/**
 * HANDLE MESSAGE
 *
 * Module does not have any gate, it cannot get messages
 */
void PimNeighborTable::handleMessage(cMessage *msg)
{
    opp_error("This module doesn't process messages");
}

void PimNeighborTable::initialize(int stage)
{
	WATCH_VECTOR(nt);
	id = 0;
}

/**
 * PRINT PIM NEIGHBOR TABLE
 *
 * Printout of Table of PIM interfaces
 */
void PimNeighborTable::printPimNeighborTable()
{
	for(std::vector<PimNeighbor>::iterator i = nt.begin(); i < nt.end(); i++)
	{
		EV << (*i).info() << endl;
	}
}

/**
 * GET NEIGHBORS BY INTERFACE ID
 *
 * The method returns all neigbors which are connected to given router interface.
 *
 * @param intId Identifier of interface.
 * @return Vector of entries from PIM neighbor table.
 */
std::vector<PimNeighbor> PimNeighborTable::getNeighborsByIntID(int intId)
{
	vector<PimNeighbor> nbr;

	for(int i = 0; i < getNumNeighbors(); i++)
	{
		if(intId == getNeighbor(i)->getInterfaceID())
		{
			nbr.push_back(*getNeighbor(i));
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
PimNeighbor *PimNeighborTable::getNeighborsByID(int id)
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

PimNeighbor *PimNeighborTable::getNeighborByIntID(int intId)
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
bool PimNeighborTable::deleteNeighbor(int id)
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
 * IS IN TABLE
 *
 * The method finds out if given entry is present in the table.
 *
 * @param entry PIM neighbor entry.
 * @return True if entry was found in the table, otherwise false.
 */
bool PimNeighborTable::isInTable(PimNeighbor entry)
{
	for(int i = 0; i < getNumNeighbors(); i++)
	{
		if((entry.getAddr() == getNeighbor(i)->getAddr()) && (entry.getInterfaceID() == getNeighbor(i)->getInterfaceID()))
			return true;
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
PimNeighbor *PimNeighborTable::findNeighbor(int intId, IPv4Address addr)
{
	for(int i = 0; i < getNumNeighbors(); i++)
	{
		if((addr == getNeighbor(i)->getAddr()) && (intId == getNeighbor(i)->getInterfaceID()))
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
int PimNeighborTable::getNumNeighborsOnInt(int intId)
{
	std::vector<PimNeighbor> neighbors = getNeighborsByIntID(intId);
	return neighbors.size();
}
