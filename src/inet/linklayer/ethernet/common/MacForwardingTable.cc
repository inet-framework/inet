//
// Copyright (C) 2013 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#include "inet/linklayer/ethernet/common/MacForwardingTable.h"

#include <map>

#include "inet/common/ModuleAccess.h"
#include "inet/common/stlutils.h"
#include "inet/common/StringFormat.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

namespace inet {

#define MAX_LINE    100

Define_Module(MacForwardingTable);

std::ostream& operator<<(std::ostream& os, const std::vector<int>& ids)
{
    os << "[";
    for (int i = 0; i < ids.size(); i++) {
        auto id = ids[i];
        if (i != 0)
            os << ", ";
        os << id;
    }
    return os << "]";
}

std::ostream& operator<<(std::ostream& os, const MacForwardingTable::AddressEntry& entry)
{
    return os << "{interfaceId=" << entry.interfaceId << ", insertionTime=" << entry.insertionTime << "}";
}

std::ostream& operator<<(std::ostream& os, const MacForwardingTable::ForwardingTableKey& key)
{
    return os << "{VID=" << key.first << ", addr=" << key.second << "}";
}

std::ostream& operator<<(std::ostream& os, const MacForwardingTable::MulticastAddressEntry& entry)
{
    return os << "{interfaceIds=" << entry.interfaceIds << "}";
}

void MacForwardingTable::initialize(int stage)
{
    OperationalBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        agingTime = par("agingTime");
        lastPurge = SIMTIME_ZERO;
        ifTable.reference(this, "interfaceTableModule", true);
        WATCH_MAP(forwardingTable);
        WATCH_MAP(multicastForwardingTable);
    }
}

void MacForwardingTable::handleParameterChange(const char *name)
{
    if (!strcmp(name, "forwardingTable")) {
        clearTable();
        parseForwardingTableParameter();
    }
}

/**
 * Function reads from a file stream pointed to by 'fp' and stores characters
 * until the '\n' or EOF character is found, the resultant string is returned.
 * Note that neither '\n' nor EOF character is stored to the resultant string,
 * also note that if on a line containing useful data that EOF occurs, then
 * that line will not be read in, hence must terminate file with unused line.
 */
static char *fgetline(FILE *fp)
{
    // alloc buffer and read a line
    char *line = new char[MAX_LINE];
    if (fgets(line, MAX_LINE, fp) == nullptr) {
        delete[] line;
        return nullptr;
    }

    // chop CR/LF
    line[MAX_LINE - 1] = '\0';
    int len = strlen(line);
    while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
        line[--len] = '\0';

    return line;
}

void MacForwardingTable::handleMessage(cMessage *)
{
    throw cRuntimeError("This module doesn't process messages");
}

void MacForwardingTable::handleMessageWhenUp(cMessage *)
{
    throw cRuntimeError("This module doesn't process messages");
}

void MacForwardingTable::refreshDisplay() const
{
    updateDisplayString();
}

void MacForwardingTable::updateDisplayString() const
{
    if (getEnvir()->isGUI()) {
        auto text = StringFormat::formatString(par("displayStringTextFormat"), this);
        getDisplayString().setTagArg("t", 0, text.c_str());
    }
}

std::string MacForwardingTable::resolveDirective(char directive) const
{
    switch (directive) {
        case 'a':
            return std::to_string(forwardingTable.size());
        case 'v':
            return "";
        default:
            throw cRuntimeError("Unknown directive: %c", directive);
    }
}

int MacForwardingTable::getUnicastAddressForwardingInterface(const MacAddress& address, unsigned int vid) const
{
    Enter_Method("getUnicastAddressForwardingInterface");
    ASSERT(!address.isMulticast());
    ForwardingTableKey key(vid, address);
    auto it = forwardingTable.find(key);
    if (it == forwardingTable.end())
        return -1;
    else if (it->second.insertionTime <= simTime() - agingTime) {
        EV_TRACE << "Ignoring aged entry: " << it->first << " --> " << it->second << "\n";
        return -1;
    }
    else
        return it->second.interfaceId;
}

void MacForwardingTable::setUnicastAddressForwardingInterface(int interfaceId, const MacAddress& address, unsigned int vid)
{
    Enter_Method("setUnicastAddressForwardingInterface");
    ASSERT(!address.isMulticast());
    ForwardingTableKey key(vid, address);
    auto it = forwardingTable.find(key);
    if (it == forwardingTable.end())
        forwardingTable[key] = AddressEntry(vid, interfaceId, -1);
    else {
        it->second.interfaceId = interfaceId;
        it->second.insertionTime = SimTime::getMaxTime();
    }
}

void MacForwardingTable::removeUnicastAddressForwardingInterface(int interfaceId, const MacAddress& address, unsigned int vid)
{
    Enter_Method("removeUnicastAddressForwardingInterface");
    ASSERT(!address.isMulticast());
    ForwardingTableKey key(vid, address);
    auto it = forwardingTable.find(key);
    if (it == forwardingTable.end())
        throw cRuntimeError("Cannot find entry");
    forwardingTable.erase(it);
}

void MacForwardingTable::learnUnicastAddressForwardingInterface(int interfaceId, const MacAddress& address, unsigned int vid)
{
    Enter_Method("learnUnicastAddressForwardingInterface");
    ASSERT(!address.isMulticast());
    removeAgedEntriesIfNeeded();
    ForwardingTableKey key(vid, address);
    auto it = forwardingTable.find(key);
    if (it == forwardingTable.end()) {
        EV << "Adding entry" << EV_FIELD(address) << EV_FIELD(interfaceId) << EV_FIELD(vid) << EV_ENDL;
        forwardingTable[key] = AddressEntry(vid, interfaceId, simTime());
    }
    else if (it->second.insertionTime != SimTime::getMaxTime()) {
        EV << "Updating entry" << EV_FIELD(address) << EV_FIELD(interfaceId) << EV_FIELD(vid) << EV_ENDL;
        AddressEntry& entry = it->second;
        entry.interfaceId = interfaceId;
        entry.insertionTime = simTime();
    }
    else
        EV << "Ignoring manually configured entry" << EV_FIELD(address) << EV_FIELD(interfaceId) << EV_FIELD(vid) << EV_ENDL;
}

std::vector<int> MacForwardingTable::getMulticastAddressForwardingInterfaces(const MacAddress& address, unsigned int vid) const
{
    Enter_Method("getMulticastAddressForwardingInterfaces");
    ASSERT(address.isMulticast());
    ForwardingTableKey key(vid, address);
    auto it = multicastForwardingTable.find(key);
    if (it == multicastForwardingTable.end())
        return std::vector<int>();
    else
        return it->second.interfaceIds;
}

void MacForwardingTable::addMulticastAddressForwardingInterface(int interfaceId, const MacAddress& address, unsigned int vid)
{
    Enter_Method("addMulticastAddressForwardingInterface");
    ASSERT(address.isMulticast());
    ForwardingTableKey key(vid, address);
    auto it = multicastForwardingTable.find(key);
    if (it == multicastForwardingTable.end())
        multicastForwardingTable[key] = MulticastAddressEntry(vid, {interfaceId});
    else {
        if (contains(it->second.interfaceIds, interfaceId))
            throw cRuntimeError("Already contains interface");
        it->second.interfaceIds.push_back(interfaceId);
    }
}

void MacForwardingTable::removeMulticastAddressForwardingInterface(int interfaceId, const MacAddress& address, unsigned int vid)
{
    Enter_Method("removeMulticastAddressForwardingInterface");
    ASSERT(address.isMulticast());
    ForwardingTableKey key(vid, address);
    auto it = multicastForwardingTable.find(key);
    if (it == multicastForwardingTable.end())
        throw cRuntimeError("Cannot find entry");
    if (contains(it->second.interfaceIds, interfaceId))
        throw cRuntimeError("Cannot find interface");
    remove(it->second.interfaceIds, interfaceId);
}

void MacForwardingTable::removeForwardingInterface(int interfaceId)
{
    Enter_Method("removeForwardingInterface");
    for (auto cur = forwardingTable.begin(); cur != forwardingTable.end();) {
        if (cur->second.interfaceId == interfaceId)
            cur = forwardingTable.erase(cur);
        else
            ++cur;
    }
}

void MacForwardingTable::printState()
{
    EV << endl << "MAC Address Table" << endl;
    EV << "VLAN ID    MAC    IfId    Inserted" << endl;
    for (auto& elem : forwardingTable)
        EV << elem.first.first << "   " << elem.first.second << "   " << elem.second.interfaceId << "   " << elem.second.insertionTime << endl;
}

void MacForwardingTable::replaceForwardingInterface(int oldInterfaceId, int newInterfaceId)
{
    Enter_Method("replaceForwardingInterface");
    for (auto& elem : forwardingTable) {
        if (elem.second.interfaceId == oldInterfaceId)
            elem.second.interfaceId = newInterfaceId;
    }
}

void MacForwardingTable::removeAgedEntriesFromAllVlans()
{
    for (auto cur = forwardingTable.begin(); cur != forwardingTable.end();) {
        AddressEntry& entry = cur->second;
        if (entry.insertionTime <= simTime() - agingTime) {
            EV << "Removing aged entry from Address Table: "
               << cur->first.first << " " << cur->first.second << " --> interfaceId " << cur->second.interfaceId << "\n";
            cur = forwardingTable.erase(cur);
        }
        else
            ++cur;
    }
}

void MacForwardingTable::removeAgedEntriesIfNeeded()
{
    simtime_t now = simTime();

    if (now >= lastPurge + SimTime(1, SIMTIME_S))
        removeAgedEntriesFromAllVlans();

    lastPurge = simTime();
}

void MacForwardingTable::readForwardingTable(const char *fileName)
{
    FILE *fp = fopen(fileName, "r");
    if (fp == nullptr)
        throw cRuntimeError("cannot open address table file `%s'", fileName);

    // parse address table file:
    char *line;
    for (int lineno = 0; (line = fgetline(fp)) != nullptr; delete[] line) {
        lineno++;

        // lines beginning with '#' are treated as comments
        if (line[0] == '#')
            continue;

        // scan in VLAN ID
        char *vlanIdStr = strtok(line, " \t");
        // scan in MAC address
        char *macAddressStr = strtok(nullptr, " \t");
        // scan in interface name
        char *interfaceName = strtok(nullptr, " \t");

        char *endptr = nullptr;

        // empty line or comment?
        if (!vlanIdStr || *vlanIdStr == '#')
            continue;

        // broken line?
        if (!vlanIdStr || !macAddressStr || !interfaceName)
            throw cRuntimeError("line %d invalid in address table file `%s'", lineno, fileName);

        // parse columns:

        // parse VLAN ID:
        unsigned int vlanId = strtol(vlanIdStr, &endptr, 10);
        if (!endptr || *endptr)
            throw cRuntimeError("error in line %d in address table file `%s': VLAN ID '%s' unresolved", lineno, fileName, vlanIdStr);

        // parse MAC address:
        L3Address addr;
        if (!L3AddressResolver().tryResolve(macAddressStr, addr, L3AddressResolver::ADDR_MAC))
            throw cRuntimeError("error in line %d in address table file `%s': MAC address '%s' unresolved", lineno, fileName, macAddressStr);
        MacAddress macAddress = addr.toMac();

        // parse interface:
        int interfaceId = -1;
        auto ie = ifTable->findInterfaceByName(interfaceName);
        if (ie == nullptr) {
            long int num = strtol(interfaceName, &endptr, 10);
            if (endptr && *endptr == '\0')
                ie = ifTable->findInterfaceById(num);
        }
        if (ie == nullptr)
            throw cRuntimeError("error in line %d in address table file `%s': interface '%s' not found", lineno, fileName, interfaceName);
        interfaceId = ie->getInterfaceId();

        // Create an entry with address and interfaceId and insert into table
        AddressEntry entry(vlanId, interfaceId, 0);
        ForwardingTableKey key(vlanId, macAddress);
        forwardingTable[key] = entry;
    }
    fclose(fp);
}

void MacForwardingTable::parseForwardingTableParameter()
{
    auto forwardingTableObject = check_and_cast<cValueArray *>(par("forwardingTable").objectValue());
    for (int i = 0; i < forwardingTableObject->size(); i++) {
        cValueMap *entry = check_and_cast<cValueMap *>(forwardingTableObject->get(i).objectValue());
        unsigned int vlan = 0;
        if (entry->containsKey("vlan"))
            vlan = entry->get("vlan");
        auto macAddressString = entry->get("address").stringValue();
        L3Address l3Address;
        if (!L3AddressResolver().tryResolve(macAddressString, l3Address, L3AddressResolver::ADDR_MAC))
            throw cRuntimeError("Cannot resolve MAC address of '%s'", macAddressString);
        MacAddress macAddress = l3Address.toMac();
        auto interfaceName = entry->get("interface").stringValue();
        auto networkInterface = ifTable->findInterfaceByName(interfaceName);
        if (networkInterface == nullptr)
            throw cRuntimeError("Cannot find network interface '%s'", interfaceName);
        if (macAddress.isMulticast())
            addMulticastAddressForwardingInterface(networkInterface->getInterfaceId(), macAddress, vlan);
        else {
            ForwardingTableKey key(vlan, macAddress);
            if (containsKey(forwardingTable, key))
                throw cRuntimeError("Table already contains %s unicast MAC address for vlan %u.", macAddress.str().c_str(), vlan);
            setUnicastAddressForwardingInterface(networkInterface->getInterfaceId(), macAddress, vlan);
        }
    }
}

void MacForwardingTable::initializeTable()
{
    clearTable();
    parseForwardingTableParameter();

    // Option to pre-read in Address Table. To turn it off, set forwardingTableFile to empty string
    const char *forwardingTableFile = par("forwardingTableFile");
    if (forwardingTableFile && *forwardingTableFile)
        readForwardingTable(forwardingTableFile);
}

void MacForwardingTable::clearTable()
{
    forwardingTable.clear();
    multicastForwardingTable.clear();
}

void MacForwardingTable::setAgingTime(simtime_t agingTime)
{
    this->agingTime = agingTime == -1 ? par("agingTime") : agingTime;
}

} // namespace inet

