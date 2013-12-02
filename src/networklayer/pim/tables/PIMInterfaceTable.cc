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

#include "InterfaceMatcher.h"
#include "InterfaceTableAccess.h"
#include "IPv4InterfaceData.h"
#include "PIMInterfaceTable.h"

using namespace std;

Define_Module(PIMInterfaceTable);
Register_Class(addRemoveAddr);

/** Printout of structure PIMInterface. */
std::ostream& operator<<(std::ostream& os, const PIMInterface& e)
{
	int i;
	std::vector<IPv4Address> intMulticastAddresses = e.getIntMulticastAddresses();

    os << "ID = " << e.getInterfaceId() << "; mode = ";
    if (e.getMode() == PIMInterface::DenseMode)
    	os << "Dense";
    else if (e.getMode() == PIMInterface::SparseMode)
    	os << "Sparse";
    os << "; Multicast addresses: ";

    int vel = intMulticastAddresses.size();
    if (vel > 0)
    {
		for(i = 0; i < (vel - 1); i++)
			os << intMulticastAddresses[i] << ", ";
		os << intMulticastAddresses[i];
    }
    else
    	os << "Null";
    return os;
};


/** Printout of structure PIMInterfaces Table. */
std::ostream& operator<<(std::ostream& os, const PIMInterfaceTable& e)
{
    for (int i = 0; i < e.size(); i++)
    	os << "";
		//os << "ID = " << e.getInterface(i)->getInterfaceID() << "; mode = " << e.getInterface(i)->getMode();
    return os;
};

/** Actually not in use */
std::string PIMInterface::info() const
{
	std::stringstream out;
	out << "ID = " << getInterfaceId() << "; mode = " << mode;
	return out.str();
}

/**
 * REMOVE INTERFACE MULTICAST ADDRESS
 *
 * The method removes given address from vector of multicast addresses.
 *
 * @param addr IP address which should be deleted.
 */
void PIMInterface::removeIntMulticastAddress(IPv4Address addr)
{
	for(unsigned int i = 0; i < reportedMulticastGroups.size(); i++)
	{
		if (reportedMulticastGroups[i] == addr)
		{
			reportedMulticastGroups.erase(reportedMulticastGroups.begin() + i);
			return;
		}
	}
}

/**
 * IS LOCAL INETRFACE MULTICAST ADDRESS
 *
 * The method finds out if IP address is assigned to interface as local multicast address.
 *
 * @param addr Multicast IP address which we are looking for.
 * @return True if method finds the IP address on the list, return false otherwise.
 */
bool PIMInterface::isLocalIntMulticastAddress (IPv4Address addr)
{
	for(unsigned int i = 0; i < reportedMulticastGroups.size(); i++)
	{
		if (reportedMulticastGroups[i] == addr)
			return true;
	}
	return false;
}



/**
 * HANDLE MESSAGE
 *
 * Module does not have any gate, it cannot get messages
 */
void PIMInterfaceTable::handleMessage(cMessage *msg)
{
    opp_error("This module doesn't process messages");
}

void PIMInterfaceTable::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL)
    {
		WATCH_VECTOR(pimIft);

        cModule *host = findContainingNode(this);
        if (!host)
            throw cRuntimeError("PIMSplitter: containing node not found.");
        host->subscribe(NF_INTERFACE_IPv4CONFIG_CHANGED, this);
    }
    else if (stage == INITSTAGE_LINK_LAYER_2)
    {
        configureInterfaces(par("pimConfig").xmlValue());
    }
}

void PIMInterfaceTable::configureInterfaces(cXMLElement *config)
{
    cXMLElementList interfaceElements = config->getChildrenByTagName("interface");
    InterfaceMatcher matcher(interfaceElements);
    IInterfaceTable *ift = InterfaceTableAccess().get(this);

    for (int k = 0; k < ift->getNumInterfaces(); ++k)
    {
        InterfaceEntry *ie = ift->getInterface(k);
        if (ie->isMulticast() && !ie->isLoopback())
        {
            int i = matcher.findMatchingSelector(ie);
            if (i >= 0)
                addInterface(ie, interfaceElements[i]);
        }
    }
}

void PIMInterfaceTable::addInterface(InterfaceEntry *ie, cXMLElement *config)
{
    const char *modeAttr = config->getAttribute("mode");
    if (!modeAttr)
        return;

    PIMInterface::PIMMode mode;
    if (strcmp(modeAttr, "dense") == 0)
        mode = PIMInterface::DenseMode;
    else if (strcmp(modeAttr, "sparse") == 0)
        mode = PIMInterface::SparseMode;
    else
        throw cRuntimeError("PIMInterfaceTable: invalid 'mode' attribute value in the configuration of interface '%s'", ie->getName());

    const char *stateRefreshAttr = config->getAttribute("state-refresh");
    bool stateRefreshFlag = stateRefreshAttr && strcmp(stateRefreshAttr, "true");

    addInterface(PIMInterface(ie, mode, stateRefreshFlag));
}

PIMInterface *PIMInterfaceTable::getInterfaceById(int interfaceId)
{
	for(int i = 0; i < getNumInterfaces(); i++)
		if(interfaceId == getInterface(i)->getInterfaceId())
			return getInterface(i);
	return NULL;
}

void PIMInterfaceTable::receiveSignal(cComponent *source, simsignal_t signalID, cObject *details)
{
    // ignore notifications during initialize
    if (simulation.getContextType()==CTX_INITIALIZE)
        return;

    Enter_Method_Silent();
    printNotificationBanner(signalID, details);

    // configuration of interface changed, it means some change from IGMP
    if (signalID ==  NF_INTERFACE_IPv4CONFIG_CHANGED)
    {
        EV << "PIMInterfaceTable::receiveChangeNotification - IGMP change" << endl;
        InterfaceEntryChangeDetails *changeDetails = check_and_cast<InterfaceEntryChangeDetails*>(details);
        if (changeDetails->getFieldId() == IPv4InterfaceData::F_MULTICAST_LISTENERS)
            igmpChange(changeDetails->getInterfaceEntry());
    }
}

/**
 * IGMP CHANGE
 *
 * The method is used to process notification about IGMP change. Splitter will find out which IP address
 * were added or removed from interface and will send them to appropriate PIM mode.
 *
 * @param interface Pointer to interface where IP address changed.
 * @see addRemoveAddr
 */
void PIMInterfaceTable::igmpChange(InterfaceEntry *interface)
{
    int intId = interface->getInterfaceId();
    PIMInterface * pimInt = getInterfaceById(intId);

    // save old and new set of multicast IP address assigned to interface
    if(pimInt)
    {
    vector<IPv4Address> multicastAddrsOld = pimInt->getIntMulticastAddresses();
    vector<IPv4Address> reportedMulticastGroups;
    for (int i = 0; i < interface->ipv4Data()->getNumOfReportedMulticastGroups(); i++)
    {
        IPv4Address multicastAddr = interface->ipv4Data()->getReportedMulticastGroup(i);
        if (!multicastAddr.isLinkLocalMulticast())
            reportedMulticastGroups.push_back(multicastAddr);
    }

    // vectors of new and removed multicast addresses
    vector<IPv4Address> add;
    vector<IPv4Address> remove;

    // which address was removed from interface
    for (unsigned int i = 0; i < multicastAddrsOld.size(); i++)
    {
        unsigned int j;
        for (j = 0; j < reportedMulticastGroups.size(); j++)
        {
            if (multicastAddrsOld[i] == reportedMulticastGroups[j])
                break;
        }
        if (j == reportedMulticastGroups.size())
        {
            EV << "Multicast address " << multicastAddrsOld[i] << " was removed from the interface " << intId << endl;
            remove.push_back(multicastAddrsOld[i]);
        }
    }

    // which address was added to interface
    for (unsigned int i = 0; i < reportedMulticastGroups.size(); i++)
    {
        unsigned int j;
        for (j = 0; j < multicastAddrsOld.size(); j++)
        {
            if (reportedMulticastGroups[i] == multicastAddrsOld[j])
                break;
        }
        if (j == multicastAddrsOld.size())
        {
            EV << "Multicast address " << reportedMulticastGroups[i] << " was added to the interface " << intId <<endl;
            add.push_back(reportedMulticastGroups[i]);
        }
    }

    // notification about removed multicast address to PIM modules
    addRemoveAddr *addr = new addRemoveAddr();
    if (remove.size() > 0)
    {
        // remove new address
        for(unsigned int i = 0; i < remove.size(); i++)
            pimInt->removeIntMulticastAddress(remove[i]);

        // send notification
        addr->setAddr(remove);
        addr->setInt(pimInt);
        if (pimInt->getMode() == PIMInterface::DenseMode)
            emit(NF_IPv4_NEW_IGMP_REMOVED, addr);
        if (pimInt->getMode() == PIMInterface::SparseMode)
            emit(NF_IPv4_NEW_IGMP_REMOVED_PIMSM, addr);
    }

    // notification about new multicast address to PIM modules
    if (add.size() > 0)
    {
        // add new address
        for(unsigned int i = 0; i < add.size(); i++)
            pimInt->addIntMulticastAddress(add[i]);

        // send notification
        addr->setAddr(add);
        addr->setInt(pimInt);
        if (pimInt->getMode() == PIMInterface::DenseMode)
            emit(NF_IPv4_NEW_IGMP_ADDED, addr);
        if (pimInt->getMode() == PIMInterface::SparseMode)
            emit(NF_IPv4_NEW_IGMP_ADDED_PISM, addr);
    }
    }
}


