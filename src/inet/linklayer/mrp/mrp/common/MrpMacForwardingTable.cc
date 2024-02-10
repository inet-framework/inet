// Copyright (C) 2024 Daniel Zeitler
// SPDX-License-Identifier: LGPL-3.0-or-later


#include "inet/linklayer/mrp/common/MrpMacForwardingTable.h"

#include <map>

#include "inet/common/ModuleAccess.h"
#include "inet/common/stlutils.h"
#include "inet/networklayer/common/L3AddressResolver.h"

namespace inet {

#define MAX_LINE    100

Define_Module(MrpMacForwardingTable);

void MrpMacForwardingTable::initialize(int stage)
{
    MacForwardingTable::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        agingTime = SimTime(3,SIMTIME_S);
        lastPurge = SIMTIME_ZERO;
        //ifTable.reference(this, "interfaceTableModule", true);
        //WATCH_MAP(mrpForwardingTable);
    }
}

std::vector<int> MrpMacForwardingTable::getMrpForwardingInterfaces(const MacAddress& address, unsigned int vid) const
{
    Enter_Method("getMrpAddressForwardingInterfaces");
    ASSERT(address.isMulticast());
    ForwardingTableKey key(vid, address);
    auto it = mrpForwardingTable.find(key);
    if (it == mrpForwardingTable.end())
        return std::vector<int>();
    else
        return it->second.interfaceIds;
}

void MrpMacForwardingTable::addMrpForwardingInterface(int interfaceId, const MacAddress& address, unsigned int vid)
{
    Enter_Method("addMrpForwardingInterface");
    ASSERT(address.isMulticast());
    ForwardingTableKey key(vid, address);
    auto it = mrpForwardingTable.find(key);
    if (it == mrpForwardingTable.end())
        mrpForwardingTable[key] = MulticastAddressEntry(vid, {interfaceId});
    else {
        if (!contains(it->second.interfaceIds, interfaceId))
            it->second.interfaceIds.push_back(interfaceId);
        else
            //throw cRuntimeError("Already contains interface");
            EV_DEBUG << "Already contains interface" << EV_ENDL;
    }
}

void MrpMacForwardingTable::removeMrpForwardingInterface(int interfaceId, const MacAddress& address, unsigned int vid)
{
    Enter_Method("removeMrpForwardingInterface");
    ASSERT(address.isMulticast());
    ForwardingTableKey key(vid, address);
    auto it = mrpForwardingTable.find(key);
    if (it == mrpForwardingTable.end())
        throw cRuntimeError("Cannot find entry");
    if (!contains(it->second.interfaceIds, interfaceId))
        throw cRuntimeError("Cannot find interface");
    remove(it->second.interfaceIds, interfaceId);
}

void MrpMacForwardingTable::clearTable()
{
    forwardingTable.clear();
    multicastForwardingTable.clear();
}

void MrpMacForwardingTable::clearMrpTable()
{
    mrpForwardingTable.clear();
}

void MrpMacForwardingTable::handleStopOperation(LifecycleOperation *operation)
{
    clearTable();
    clearMrpTable();
}

void MrpMacForwardingTable::handleCrashOperation(LifecycleOperation *operation)
{
    clearTable();
    clearMrpTable();
}

} // namespace inet

