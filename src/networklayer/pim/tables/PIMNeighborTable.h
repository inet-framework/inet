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

#ifndef __INET_PIMNEIGHBORTABLE_H
#define __INET_PIMNEIGHBORTABLE_H

#include "INETDefs.h"
#include "ModuleAccess.h"
#include "InterfaceEntry.h"
#include "PIMTimer_m.h"

/**
 * Class holding information about a neighboring PIM router.
 * Routers are identified by the link to which they are connected
 * and their address.
 *
 * Currently only the version of the routers are stored.
 * TODO add fields for options received in Hello Messages (RFC 3973 4.7.5, RFC 4601 4.9.2).
 */
class INET_API PIMNeighbor: public cObject
{
	protected:
		InterfaceEntry *ie;
		IPv4Address address;
		int version;
		cMessage *livenessTimer;

	public:
		PIMNeighbor(InterfaceEntry *ie, IPv4Address address, int version);
	    virtual ~PIMNeighbor();
	    virtual std::string info() const;

	    int getInterfaceId() const {return ie->getInterfaceId(); }
	    InterfaceEntry *getInterfacePtr() const {return ie;}
	    IPv4Address getAddress() const {return address;}
	    int getVersion() const {return version;}
	    cMessage *getLivenessTimer() const {return livenessTimer;}
};

/**
 * Class holding informatation about neighboring PIM routers.
 * Routers are identified by the link to which they are connected and their address.
 *
 * Expired entries are automatically deleted.
 */
class INET_API PIMNeighborTable: public cSimpleModule
{
	protected:
        typedef std::vector<PIMNeighbor*> PIMNeighborVector;

        // contains at most one neighbor with a given (ie,address)
		PIMNeighborVector	neighbors;

	public:
		virtual ~PIMNeighborTable();

		/**
		 * Adds the a neighbor to the table. The operation might fail
		 * if there is a neighbor with the same (ie,address) in the table.
		 * Success is indicated by the returned value.
		 */
		virtual bool addNeighbor(PIMNeighbor *neighbor);

		/**
		 * Deletes a neighbor from the table. If the neighbor was
		 * not found in the table then it is untouched, otherwise deleted.
		 * Returns true if the neighbor object was deleted.
		 */
		virtual bool deleteNeighbor(PIMNeighbor *neighbor);

        /**
         * Restarts the Neighbor Liveness timer of the given neighbor.
         * When the timer expires, the neigbor is automatically deleted.
         */
        virtual void restartLivenessTimer(PIMNeighbor *neighbor);

        /**
         * Returns the neighbor that is identified by the given (interfaceId,addr),
         * or NULL if no such neighbor.
         */
        virtual PIMNeighbor *findNeighbor(int interfaceId, IPv4Address addr);

        /**
         * Returns the number of neighbors.
         */
        virtual int getNumNeighbors() const {return neighbors.size();}

        /**
         * Returns the kth neighbor.
         */
        virtual PIMNeighbor *getNeighbor(int k) const {return neighbors[k];}

        /**
         * Returns all neighbors observed on the given interface.
         */
		virtual PIMNeighborVector getNeighborsByIntID(int interfaceId);

		/**
		 * Returns the neighbor that was first observed on the given interface,
		 * or NULL if there is none.
		 * XXX What is the use case of this method?
		 */
		virtual PIMNeighbor *getNeighborByIntID(int interfaceId);

		/**
		 * Returns the number of neighbors on the given interface.
		 */
		virtual int getNumNeighborsOnInt(int interfaceId);

	protected:
        virtual int numInitStages() const  {return NUM_INIT_STAGES;}
		virtual void initialize(int stage);
		virtual void handleMessage(cMessage *);
		virtual void processLivenessTimer(cMessage *timer);
};

/**
 * Use PIMNeighborTableAccess().get() to access PIMNeighborTable from other modules of the node.
 */
// TODO eliminate this; do not hard-wire the name of the module into C++ code.
class INET_API PIMNeighborTableAccess : public ModuleAccess<PIMNeighborTable>
{
	public:
		PIMNeighborTableAccess() : ModuleAccess<PIMNeighborTable>("pimNeighborTable") {}
};

#endif
