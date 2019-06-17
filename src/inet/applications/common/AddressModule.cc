//
// Copyright (C) Alfonso Ariza 2011 Universidad de Malaga
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
// 

#include "inet/applications/common/AddressModule.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include <string.h>

namespace inet {

simsignal_t AddressModule::changeAddressSignalInit;
simsignal_t AddressModule::changeAddressSignalDelete;

Register_Class(AddressModule)
AddressModule::AddressModule()
{
    // TODO Auto-generated constructor stub
    emitSignal = false;
    isInitialized = false;
    myAddress.reset();
    changeAddressSignalInit = getSimulation()->getSystemModule()->registerSignal("changeAddressSignalInit");
    changeAddressSignalDelete = getSimulation()->getSystemModule()->registerSignal("changeAddressSignalDelete");
}

AddressModule::~AddressModule()
{
    // TODO Auto-generated destructor stub
    if (emitSignal) {
        cSimpleModule * owner = check_and_cast<cSimpleModule*> (getOwner());
        owner->emit(changeAddressSignalDelete,this);
        getSimulation()->getSystemModule()->unsubscribe(changeAddressSignalDelete, this);
        getSimulation()->getSystemModule()->unsubscribe(changeAddressSignalInit, this);
    }
}

void AddressModule::initModule(bool mode)
{
    cSimpleModule * owner = check_and_cast<cSimpleModule*>(getOwner());
    emitSignal = mode;
    destAddresses.clear();
    destModuleId.clear();

    if (owner->hasPar("destAddresses")) {
        const char *token;
        std::string aux = owner->par("destAddresses").stdstringValue();
        cStringTokenizer tokenizer(aux.c_str());
        if (!L3AddressResolver().tryResolve(owner->getParentModule()->getFullPath().c_str(), myAddress))
            throw cRuntimeError("Node Address no available");

        while ((token = tokenizer.nextToken()) != nullptr) {
            if (strstr(token, "Broadcast") != nullptr) {
                if (myAddress.getType() == L3Address::IPv4)
                    destAddresses.push_back(Ipv4Address::ALLONES_ADDRESS);
                else
                    destAddresses.push_back(Ipv6Address::ALL_NODES_1);
            }
            else {
                L3Address addr = L3AddressResolver().resolve(token);
                if (addr != myAddress)
                    destAddresses.push_back(addr);
            }
        }
    }

    for (unsigned int i = 0; i < destAddresses.size(); i++) {
        destModuleId.push_back(L3AddressResolver().findHostWithAddress(destAddresses[i])->getId());
    }

    if (emitSignal) {
        getSimulation()->getSystemModule()->subscribe(changeAddressSignalInit, this);
        getSimulation()->getSystemModule()->subscribe(changeAddressSignalDelete, this);
        if (simTime() > 0)
            owner->emit(changeAddressSignalInit, this);

    }
    isInitialized = true;
    index = -1;
}

L3Address AddressModule::getAddress(int val)
{
    if (val == -1) {
        if (chosedAddresses.isUnspecified() && !destAddresses.empty())
            chosedAddresses = choseNewAddress(index);
        return chosedAddresses;
    }
    if (val >= 0 && val < (int)destAddresses.size())
        return destAddresses[val];

    throw cRuntimeError(this, "Invalid index: %i", val);
}

L3Address AddressModule::choseNewAddress(int &index)
{
    index = -1;
    L3Address choseAddr;
    if (destAddresses.empty())
        return choseAddr;
    else if (destAddresses.size() == 1) {
        choseAddr =  destAddresses[0];
        index = 0;
    }
    else {
        int k = getEnvir()->getRNG(0)->intRand((long)destAddresses.size());
        choseAddr = destAddresses[k];
        index = k;
    }
    return choseAddr;
}

int AddressModule::choseNewModule()
{
    chosedAddresses = choseNewAddress(index);
    if (index >= 0)
        return destModuleId[index];
    return index;
}

void AddressModule::receiveSignal(cComponent *src, simsignal_t id, cObject *obj, cObject *details)
{
    if (id != changeAddressSignalInit &&  id != changeAddressSignalDelete)
        return;
    if (obj == this)
        return;
    // rebuild address destination table
    cSimpleModule * owner = check_and_cast<cSimpleModule*> (getOwner());
    std::string aux = owner->par("destAddresses").stdstringValue();
    cStringTokenizer tokenizer(aux.c_str());
    const char *token;

    L3Address myAddr = L3AddressResolver().resolve(owner->getParentModule()->getFullPath().c_str());
    while ((token = tokenizer.nextToken()) != nullptr) {
        if (strstr(token, "Broadcast") != nullptr) {
            if (myAddress.getType() == L3Address::IPv4)
                destAddresses.push_back(Ipv4Address::ALLONES_ADDRESS);
            else
                destAddresses.push_back(Ipv6Address::ALL_NODES_1);
        }
        else {
            if (id == changeAddressSignalDelete) {
                if (strstr(obj->getFullPath().c_str(),token) != nullptr)
                    continue;
            }
            L3Address addr = L3AddressResolver().resolve(token);
            if (addr != myAddr)
                destAddresses.push_back(addr);
        }
    }
    if (destAddresses.empty()) {
        chosedAddresses.reset();
        return;
    }
    // search if the address continue
    for (unsigned int i = 0; i < destAddresses.size(); i++) {
        if (chosedAddresses == destAddresses[i])
            return;
    }
    // choose other address
    chosedAddresses = choseNewAddress(index);
    isInitialized = true;
}

void AddressModule::rebuildAddressList()
{
    cSimpleModule * owner = check_and_cast<cSimpleModule*> (getOwner());
    L3Address myAddr;
    L3AddressResolver().tryResolve(owner->getParentModule()->getFullPath().c_str(), myAddr);
    if (myAddr == myAddress)
        return;

    if (isInitialized)
        return;

    const char *destAddrs = owner->par("destAddresses");
    cStringTokenizer tokenizer(destAddrs);
    const char *token;
    while ((token = tokenizer.nextToken()) != nullptr) {
        if (strstr(token, "Broadcast") != nullptr) {
            if (myAddress.getType() == L3Address::IPv4)
                destAddresses.push_back(Ipv4Address::ALLONES_ADDRESS);
            else
                destAddresses.push_back(Ipv6Address::ALL_NODES_1);
        }
        else {
            L3Address addr = L3AddressResolver().resolve(token);
            if (addr != myAddr)
                destAddresses.push_back(addr);
        }
    }
    if (destAddresses.empty()) {
        L3Address aux;
        chosedAddresses = aux;
        return;
    }
    // search if the address continue
    for (unsigned int i = 0; i < destAddresses.size(); i++) {
        if (chosedAddresses == destAddresses[i])
            return;
    }
    // choose other address
    chosedAddresses = choseNewAddress(index);
    if (emitSignal)
        owner->emit(changeAddressSignalInit, this);
}


int AddressModule::getModule(int val)
{
    if (val == -1) {
        if (index == -1)
            chosedAddresses = choseNewAddress(index);
        if (index != -1)
            return destModuleId[index];
        else
            return -1;
    }

    if (val >= 0 && val < (int)destModuleId.size())
        return destModuleId[val];
    throw cRuntimeError(this, "Invalid index: %i", val);
}

} // namespace inet
