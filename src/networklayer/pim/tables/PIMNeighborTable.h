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
 * @brief  Class represents one entry of PIMNeighborTable.
 * @details Structure PIM neighbor with info about interface, IP address of neighbor
 * link to Neighbor Livness Timer and PIM version. The class contains
 * methods to work with items of structure.
 */
class INET_API PIMNeighbor: public cObject
{
	protected:
		int					id;					/**< Unique identifier of entry. */
		InterfaceEntry 		*ie;			/**< Link to interface table entry. */
		IPv4Address         address; 				/**< IP address of neighbor. */
		int					version;				/**< PIM version. */
		PIMnlt 				*nlt;				/**< Pointer to Neighbor Livness Timer. */

	protected:
		friend class PIMNeighborTable;
        void setId(int id)  {this->id = id; nlt->setNtId(id); }                     /**< Set unique identifier of entry. */

	public:
		PIMNeighbor(InterfaceEntry *ie, IPv4Address address, int version);
	    virtual ~PIMNeighbor() {};
	    virtual std::string info() const;

	    int getId() const {return id;}												/**< Get unique identifier of entry. */
	    int getInterfaceID() const {return ie->getInterfaceId(); }									/**< Get interface ID. */
	    InterfaceEntry *getInterfacePtr() const {return ie;}					/**< Get pointer to interface. */
	    IPv4Address getAddress() const {return address;}									/**< Get IP address of neighbor. */
	    int getVersion() const {return version;}										/**< Get PIM version. */
	    PIMnlt *getNlt() const {return nlt;}										/**< Get pointer to NeighborLivenessTimer. */
};

/**
 * @brief Class represents PIM Neighbor Table.
 * @details Table is list of PIMNeighbor and class contains methods to work with them.
 */
class INET_API PIMNeighborTable: public cSimpleModule
{
	protected:
        typedef std::vector<PIMNeighbor*> PIMNeighborVector;

		int					id;				/**< Counter of PIMNeighbor IDs*/
		PIMNeighborVector	nt;				/**< List of PIM neighbors (show ip pim neighbor) */

	public:
		PIMNeighborTable() : id(0) {};
		virtual ~PIMNeighborTable();

        virtual int getNumNeighbors() {return this->nt.size();}                     /**< Get number of entries in the table */
		virtual PIMNeighbor *getNeighbor(int k){return this->nt[k];}				/**< Get k-th entry in the table */
		virtual void addNeighbor(PIMNeighbor *entry);                               /**< Add new entry to the table*/
		virtual bool deleteNeighbor(int id);

		virtual void printPimNeighborTable();
		virtual PIMNeighborVector getNeighborsByIntID(int intID);
		virtual PIMNeighbor *getNeighborByIntID(int intId);
		virtual PIMNeighbor *getNeighborsByID(int id);
		virtual int getIdCounter(){return this->id;}								/**< Get counter of entry IDs */
		virtual PIMNeighbor *findNeighbor(int intId, IPv4Address addr);
		virtual int getNumNeighborsOnInt(int intId);

	protected:
        virtual int numInitStages() const  {return NUM_INIT_STAGES;}
		virtual void initialize(int stage);
		virtual void handleMessage(cMessage *);
};

/**
 * @brief Class gives access to the PIMNeighborTable.
 */
class INET_API PIMNeighborTableAccess : public ModuleAccess<PIMNeighborTable>
{
	public:
		PIMNeighborTableAccess() : ModuleAccess<PIMNeighborTable>("pimNeighborTable") {}
};

#endif
