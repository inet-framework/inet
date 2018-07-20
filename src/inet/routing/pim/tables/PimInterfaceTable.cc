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
// Authors: Veronika Rybova, Vladimir Vesely (ivesely@fit.vutbr.cz),
//          Tamas Borbely (tomi@omnetpp.org)

#include "inet/networklayer/common/InterfaceMatcher.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/common/ModuleAccess.h"
#include "inet/routing/pim/tables/PimInterfaceTable.h"

using namespace std;

namespace inet {
Define_Module(PimInterfaceTable);

// for WATCH_VECTOR
std::ostream& operator<<(std::ostream& os, const PimInterface *e)
{
    os << "name = " << e->getInterfacePtr()->getInterfaceName() << "; mode = ";
    if (e->getMode() == PimInterface::DenseMode)
        os << "Dense";
    else if (e->getMode() == PimInterface::SparseMode)
        os << "Sparse; DR = " << e->getDRAddress();
    return os;
};

std::string PimInterface::str() const
{
    std::stringstream out;
    out << this;
    return out.str();
}

PimInterfaceTable::~PimInterfaceTable()
{
    for (auto & elem : pimInterfaces)
        delete elem;
}

void PimInterfaceTable::handleMessage(cMessage *msg)
{
    throw cRuntimeError("This module doesn't process messages");
}

void PimInterfaceTable::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        WATCH_VECTOR(pimInterfaces);
    }
    else if (stage == INITSTAGE_LINK_LAYER_2) {
        configureInterfaces(par("pimConfig"));

        cModule *host = findContainingNode(this);
        if (!host)
            throw cRuntimeError("PimInterfaceTable: containing node not found.");
        host->subscribe(interfaceCreatedSignal, this);
        host->subscribe(interfaceDeletedSignal, this);
    }
}

void PimInterfaceTable::configureInterfaces(cXMLElement *config)
{
    cXMLElementList interfaceElements = config->getChildrenByTagName("interface");
    InterfaceMatcher matcher(interfaceElements);
    IInterfaceTable *ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);

    for (int k = 0; k < ift->getNumInterfaces(); ++k) {
        InterfaceEntry *ie = ift->getInterface(k);
        if (ie->isMulticast() && !ie->isLoopback()) {
            int i = matcher.findMatchingSelector(ie);
            if (i >= 0) {
                PimInterface *pimInterface = createInterface(ie, interfaceElements[i]);
                if (pimInterface)
                    pimInterfaces.push_back(pimInterface);
            }
        }
    }
}

PimInterface *PimInterfaceTable::createInterface(InterfaceEntry *ie, cXMLElement *config)
{
    const char *modeAttr = config->getAttribute("mode");
    if (!modeAttr)
        return nullptr;

    PimInterface::PimMode mode;
    if (strcmp(modeAttr, "dense") == 0)
        mode = PimInterface::DenseMode;
    else if (strcmp(modeAttr, "sparse") == 0)
        mode = PimInterface::SparseMode;
    else
        throw cRuntimeError("PimInterfaceTable: invalid 'mode' attribute value in the configuration of interface '%s'", ie->getInterfaceName());

    const char *stateRefreshAttr = config->getAttribute("state-refresh");
    bool stateRefreshFlag = stateRefreshAttr && !strcmp(stateRefreshAttr, "true");

    return new PimInterface(ie, mode, stateRefreshFlag);
}

PimInterface *PimInterfaceTable::getInterfaceById(int interfaceId)
{
    for (int i = 0; i < getNumInterfaces(); i++)
        if (interfaceId == getInterface(i)->getInterfaceId())
            return getInterface(i);

    return nullptr;
}

void PimInterfaceTable::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method_Silent();
    printSignalBanner(signalID, obj);

    if (signalID == interfaceCreatedSignal) {
        InterfaceEntry *ie = check_and_cast<InterfaceEntry *>(obj);
        if (ie->isMulticast() && !ie->isLoopback())
            addInterface(ie);
    }
    else if (signalID == interfaceDeletedSignal) {
        InterfaceEntry *ie = check_and_cast<InterfaceEntry *>(obj);
        if (ie->isMulticast() && !ie->isLoopback())
            removeInterface(ie);
    }
}

PimInterfaceTable::PimInterfaceVector::iterator PimInterfaceTable::findInterface(InterfaceEntry *ie)
{
    for (auto it = pimInterfaces.begin(); it != pimInterfaces.end(); ++it)
        if ((*it)->getInterfacePtr() == ie)
            return it;

    return pimInterfaces.end();
}

void PimInterfaceTable::addInterface(InterfaceEntry *ie)
{
    ASSERT(findInterface(ie) == pimInterfaces.end());

    cXMLElement *config = par("pimConfig");
    cXMLElementList interfaceElements = config->getChildrenByTagName("interface");
    InterfaceMatcher matcher(interfaceElements);

    int i = matcher.findMatchingSelector(ie);
    if (i >= 0) {
        PimInterface *pimInterface = createInterface(ie, interfaceElements[i]);
        if (pimInterface)
            pimInterfaces.push_back(pimInterface);
    }
}

void PimInterfaceTable::removeInterface(InterfaceEntry *ie)
{
    auto it = findInterface(ie);
    if (it != pimInterfaces.end())
        pimInterfaces.erase(it);
}
}    //namespace inet

