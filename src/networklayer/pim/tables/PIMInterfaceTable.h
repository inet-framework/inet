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

#ifndef __INET_PIMINTERFACETABLE_H
#define __INET_PIMINTERFACETABLE_H

#include <omnetpp.h>
#include "IInterfaceTable.h"
#include "INETDefs.h"
#include "ModuleAccess.h"
#include "InterfaceEntry.h"
#include "IPv4Address.h"

/**
 * Class represents one entry of PIMInterfaceTable.
 */
class INET_API PIMInterface: public cObject
{
    public:
        enum PIMMode
        {
            DenseMode = 1,
            SparseMode = 2
        };

	protected:
		InterfaceEntry *intPtr;                    /**< Pointer to interface table entry. */
		PIMMode mode;                              /**< Type of mode. */
		bool stateRefreshFlag;                     /**< Indicator of State Refresh Originator. If it is true, router will send SR messages. */

	public:
		PIMInterface() {intPtr = NULL; stateRefreshFlag = false; }
        PIMInterface(InterfaceEntry *ie, PIMMode mode, bool stateRefreshFlag)
		    : intPtr(ie), mode(mode), stateRefreshFlag(stateRefreshFlag) { ASSERT(ie); }
	    virtual std::string info() const;

	    int getInterfaceId() const { return intPtr ? intPtr->getInterfaceId() : -1;}
		InterfaceEntry *getInterfacePtr() const {return intPtr;}
		PIMMode getMode() const {return mode;}
		bool getSR() const {return stateRefreshFlag;}
};


/**
 * @brief Class represents PIM Interface Table.
 * @brief It is vector of PIMInterface. Class contains methods to work with the table.
 */
class INET_API PIMInterfaceTable: public cSimpleModule, protected cListener
{
	protected:
		std::vector<PIMInterface*>	pimIft;					/**< List of PIM interfaces. */

	public:
		virtual ~PIMInterfaceTable();

        virtual int getNumInterfaces() {return pimIft.size();}                                /**< Returns number of entries in PIMInterfaceTable. */
		virtual PIMInterface *getInterface(int k) {return pimIft[k];}					/**< Get pointer to entry of PIMInterfaceTable from the object. */
        virtual PIMInterface *getInterfaceById(int interfaceId);                                   /**< Returns entry from PIMInterfaceTable with given interface ID. */
		virtual void addInterface(PIMInterface *entry) { pimIft.push_back(entry); }		/**< Add entry to PIMInterfaceTable. */
		//virtual bool deleteInterface(const PIMInterface *entry){};

	protected:
        virtual int numInitStages() const  {return NUM_INIT_STAGES;}
		virtual void initialize(int stage);
		virtual void handleMessage(cMessage *);
		virtual void configureInterfaces(cXMLElement *config);
		virtual void addInterface(InterfaceEntry *ie, cXMLElement *config);
        virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj);
};

/**
 * @brief Class gives access to the PIMInterfaceTable.
 */
class INET_API PIMInterfaceTableAccess : public ModuleAccess<PIMInterfaceTable>
{
	private:
		PIMInterfaceTable *p;

	public:
	PIMInterfaceTableAccess() : ModuleAccess<PIMInterfaceTable>("pimInterfaceTable") {p=NULL;}
};

#endif
