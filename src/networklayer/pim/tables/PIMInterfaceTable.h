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
 * @brief  Class represents one entry of PIMInterfaceTable.
 * @details One entry contains interfaces ID, pointer to the interface, PIM mode and multicast
 * addresses assigned to the interface.
 */
class INET_API PIMInterface: public cObject
{
    public:
        typedef std::vector<IPv4Address> IPv4AddressVector;

        enum PIMMode
        {
            DenseMode = 1,
            SparseMode = 2
        };

	protected:
		InterfaceEntry *intPtr;                    /**< Pointer to interface table entry. */
		PIMMode mode;                              /**< Type of mode. */
		IPv4AddressVector reportedMulticastGroups; /**< Addresses of multicast groups that has members on this interface. */
		bool stateRefreshFlag;                     /**< Indicator of State Refresh Originator. If it is true, router will send SR messages. */

	public:
		PIMInterface() {intPtr = NULL; stateRefreshFlag = false; }
        PIMInterface(InterfaceEntry *ie, PIMMode mode, bool stateRefreshFlag)
		    : intPtr(ie), mode(mode), stateRefreshFlag(stateRefreshFlag) { ASSERT(ie); }
	    virtual std::string info() const;

	    int getInterfaceId() const { return intPtr ? intPtr->getInterfaceId() : -1;}
		InterfaceEntry *getInterfacePtr() const {return intPtr;}
		PIMMode getMode() const {return mode;}
		IPv4AddressVector getIntMulticastAddresses() const {return reportedMulticastGroups;}
		bool getSR() const {return stateRefreshFlag;}

	    void setIntMulticastAddresses(const IPv4AddressVector &intMulticastAddresses)  {this->reportedMulticastGroups = intMulticastAddresses;}
	    void addIntMulticastAddress(IPv4Address addr)  {this->reportedMulticastGroups.push_back(addr);}
	    void removeIntMulticastAddress(IPv4Address addr);
	    bool isLocalIntMulticastAddress (IPv4Address addr);
};


/**
 * @brief Class represents PIM Interface Table.
 * @brief It is vector of PIMInterface. Class contains methods to work with the table.
 */
class INET_API PIMInterfaceTable: public cSimpleModule
{
	protected:
		std::vector<PIMInterface>	pimIft;					/**< List of PIM interfaces. */

	public:
		PIMInterfaceTable(){};
		virtual ~PIMInterfaceTable(){};

        virtual int getNumInterfaces() {return this->pimIft.size();}                                /**< Returns number of entries in PIMInterfaceTable. */
		virtual PIMInterface *getInterface(int k) {return &this->pimIft[k];}						/**< Get pointer to entry of PIMInterfaceTable from the object. */
        virtual PIMInterface *getInterfaceById(int interfaceId);                                   /**< Returns entry from PIMInterfaceTable with given interface ID. */
		virtual void addInterface(const PIMInterface &entry){this->pimIft.push_back(entry);}		/**< Add entry to PIMInterfaceTable. */
		//virtual bool deleteInterface(const PIMInterface *entry){};

	protected:
        virtual int numInitStages() const  {return NUM_INIT_STAGES;}
		virtual void initialize(int stage);
		virtual void handleMessage(cMessage *);
		virtual void configureInterfaces(cXMLElement *config);
		virtual void addInterface(InterfaceEntry *ie, cXMLElement *config);
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

/**
 * @brief Class is needed by notification about new multicast addresses on interface.
 * @details If you do not use notification board, you probably do not need this class.
 * The problem is that method fireChangeNotification
 * needs object as the second parameter.
 */
class addRemoveAddr : public cObject
{
	protected:
		std::vector<IPv4Address> addr;						/**< Vector of added or removed addresses. */
		PIMInterface *pimInt;								/**< Pointer to interface. */

	public:
		addRemoveAddr(){};
		virtual ~addRemoveAddr() {};
		virtual std::string info() const
		{
			std::stringstream out;
			for (unsigned int i = 0; i < addr.size(); i++)
				out << addr[i] << endl;
			return out.str();
		}

		void setAddr(const std::vector<IPv4Address> &addr)  {this->addr = addr;} /**< Set addresses to the object. */
		void setInt(PIMInterface *pimInt)  {this->pimInt = pimInt;}		/**< Set pointer to interface to the object. */
		std::vector<IPv4Address> getAddr () {return this->addr;}			/**< Get addresses from the object. */
		int getAddrSize () {return this->addr.size();}					/**< Returns size of addresses vector. */
		PIMInterface *getInt () {return this->pimInt;}					/**< Get pointer to interface from the object. */
};

#endif
