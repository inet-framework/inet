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

#ifndef PIMINTERFACES_H_
#define PIMINTERFACES_H_

#include <omnetpp.h>
#include "IInterfaceTable.h"
#include "INETDefs.h"
#include "ModuleAccess.h"
#include "InterfaceEntry.h"
#include "IPv4Address.h"

/**
 * PIM modes configured on the interface.
 */
enum PIMmode
{
    Dense = 1,
    Sparse = 2
};

/**
 * @brief  Class represents one entry of PIMInterfaceTable.
 * @details One entry contains interfaces ID, pointer to the interface, PIM mode and multicast
 * addresses assigned to the interface.
 */
class INET_API PIMInterface: public cObject
{
	protected:
		int 					intID;          			/**< Identification of interface. */
		InterfaceEntry *		intPtr;						/**< Pointer to interface table entry. */
		PIMmode					mode;						/**< Type of mode. */
		std::vector<IPv4Address> 	intMulticastAddresses;		/**< Multicast addresses assigned to interface. */
		bool					SR;							/**< Indicator of State Refresh Originator. */

	public:
		PIMInterface(){intPtr = NULL; SR = false;};
	    virtual ~PIMInterface() {};
	    virtual std::string info() const;

	    // set methods
	    void setInterfaceID(int iftID)  {this->intID = iftID;}							/**< Set identifier of interface. */
	    void setInterfacePtr(InterfaceEntry *intPtr)  {this->intPtr = intPtr;}			/**< Set pointer to interface. */
	    void setMode(PIMmode mode) {this->mode = mode;}									/**< Set PIM mode configured on the interface. */
	    void setSR(bool SR)  {this->SR = SR;}											/**< Set State Refresh indicator. If it is true, router will send SR msgs. */

	    //get methods
	    int getInterfaceID() const {return intID;}													/**< Get identifier of interface. */
		InterfaceEntry *getInterfacePtr() const {return intPtr;}									/**< Get pointer to interface. */
		PIMmode getMode() const {return mode;}														/**< Get PIM mode configured on the interface. */
		std::vector<IPv4Address> getIntMulticastAddresses() const {return intMulticastAddresses;}		/**< Get list of multicast addresses assigned to the interface. */
		bool getSR() const {return SR;}																/**< Get State Refresh indicator. If it is true, router will send SR msgs. */

	    // methods for work with vector "intMulticastAddresses"
	    void setIntMulticastAddresses(std::vector<IPv4Address> intMulticastAddresses)  {this->intMulticastAddresses = intMulticastAddresses;} 	/**< Set multicast addresses to the interface. */
	    void addIntMulticastAddress(IPv4Address addr)  {this->intMulticastAddresses.push_back(addr);}												/**< Add multicast address to the interface. */
	    void removeIntMulticastAddress(IPv4Address addr);
	    bool isLocalIntMulticastAddress (IPv4Address addr);
	    std::vector<IPv4Address> deleteLocalIPs(std::vector<IPv4Address> multicastAddr);
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

		virtual PIMInterface *getInterface(int k){return &this->pimIft[k];}						/**< Get pointer to entry of PIMInterfaceTable from the object. */
		virtual void addInterface(const PIMInterface entry){this->pimIft.push_back(entry);}		/**< Add entry to PIMInterfaceTable. */
		//virtual bool deleteInterface(const PIMInterface *entry){};
		virtual int getNumInterface() {return this->pimIft.size();}								/**< Returns number of entries in PIMInterfaceTable. */
		virtual void printPimInterfaces();
		virtual PIMInterface *getInterfaceByIntID(int intID);									/**< Returns entry from PIMInterfaceTable with given interface ID. */

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

	virtual PIMInterfaceTable *getMyIfExists()
	{
		if (!p)
		{
			cModule *m = findModuleWherever("pimInterfaceTable", simulation.getContextModule());
			p = dynamic_cast<PIMInterfaceTable*>(m);
		}
		return p;
	}
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

		void setAddr(std::vector<IPv4Address> addr)  {this->addr = addr;} /**< Set addresses to the object. */
		void setInt(PIMInterface *pimInt)  {this->pimInt = pimInt;}		/**< Set pointer to interface to the object. */
		std::vector<IPv4Address> getAddr () {return this->addr;}			/**< Get addresses from the object. */
		int getAddrSize () {return this->addr.size();}					/**< Returns size of addresses vector. */
		PIMInterface *getInt () {return this->pimInt;}					/**< Get pointer to interface from the object. */
};

#endif /* PIMINTERFACES_H_ */
