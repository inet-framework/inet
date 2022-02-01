//
// Copyright (C) 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

// Authors: Veronika Rybova, Vladimir Vesely (ivesely@fit.vutbr.cz),
//          Tamas Borbely (tomi@omnetpp.org)

#include "inet/routing/pim/tables/PimInterfaceTable.h"

#include "inet/common/ModuleAccess.h"
#include "inet/networklayer/common/InterfaceMatcher.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"

namespace inet {

Define_Module(PimInterfaceTable);

PimInterfaceTable::~PimInterfaceTable()
{
    for (auto& elem : pimInterfaces)
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
    // TODO INITSTAGE
    else if (stage == INITSTAGE_LINK_LAYER) {
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
        NetworkInterface *ie = ift->getInterface(k);
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

PimInterface *PimInterfaceTable::createInterface(NetworkInterface *ie, cXMLElement *config)
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
    Enter_Method("%s", cComponent::getSignalName(signalID));

    printSignalBanner(signalID, obj, details);

    if (signalID == interfaceCreatedSignal) {
        NetworkInterface *ie = check_and_cast<NetworkInterface *>(obj);
        if (ie->isMulticast() && !ie->isLoopback())
            addInterface(ie);
    }
    else if (signalID == interfaceDeletedSignal) {
        NetworkInterface *ie = check_and_cast<NetworkInterface *>(obj);
        if (ie->isMulticast() && !ie->isLoopback())
            removeInterface(ie);
    }
}

PimInterfaceTable::PimInterfaceVector::iterator PimInterfaceTable::findInterface(NetworkInterface *ie)
{
    for (auto it = pimInterfaces.begin(); it != pimInterfaces.end(); ++it)
        if ((*it)->getInterfacePtr() == ie)
            return it;

    return pimInterfaces.end();
}

void PimInterfaceTable::addInterface(NetworkInterface *ie)
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

void PimInterfaceTable::removeInterface(NetworkInterface *ie)
{
    auto it = findInterface(ie);
    if (it != pimInterfaces.end())
        pimInterfaces.erase(it);
}

// for WATCH_VECTOR
std::ostream& operator<<(std::ostream& os, const PimInterface *e)
{
    os << "name: " << e->getInterfacePtr()->getInterfaceName() << " ";
    os << "mode: ";
    if (e->getMode() == PimInterface::DenseMode)
        os << "Dense" << " ";
    else if (e->getMode() == PimInterface::SparseMode)
        os << "Sparse DR: " << e->getDRAddress() << " ";
    os << "stateRefreshFlag: " << e->getSR() << " ";
    return os;
};

std::string PimInterface::str() const
{
    std::stringstream out;
    out << this;
    return out.str();
}

} // namespace inet

