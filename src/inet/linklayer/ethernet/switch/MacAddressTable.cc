// Copyright (C) 2013 OpenSim Ltd.
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

#include <map>

#include "inet/common/ModuleAccess.h"
#include "inet/common/StringFormat.h"
#include "inet/linklayer/ethernet/switch/MacAddressTable.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/contract/IInterfaceTable.h"

namespace inet {

#define MAX_LINE    100

Define_Module(MacAddressTable);

std::ostream& operator<<(std::ostream& os, const MacAddressTable::AddressEntry& entry)
{
    os << "{VID=" << entry.vid << ", interfaceId=" << entry.interfaceId << ", insertionTime=" << entry.insertionTime << "}";
    return os;
}

MacAddressTable::MacAddressTable()
{
}

void MacAddressTable::initialize(int stage)
{
    OperationalBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        agingTime = par("agingTime");
        lastPurge = SIMTIME_ZERO;
        ifTable = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
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

void MacAddressTable::handleMessage(cMessage *)
{
    throw cRuntimeError("This module doesn't process messages");
}

void MacAddressTable::handleMessageWhenUp(cMessage *)
{
    throw cRuntimeError("This module doesn't process messages");
}

void MacAddressTable::refreshDisplay() const
{
    updateDisplayString();
}

void MacAddressTable::updateDisplayString() const
{
    auto text = StringFormat::formatString(par("displayStringTextFormat"), [&] (char directive) {
        static std::string result;
        switch (directive) {
            case 'a':
                result = addressTable ? std::to_string(addressTable->size()) : "0";
                break;
            case 'v':
                result = std::to_string(vlanAddressTable.size());
                break;
            default:
                throw cRuntimeError("Unknown directive: %c", directive);
        }
        return result.c_str();
    });
    getDisplayString().setTagArg("t", 0, text);
}

/*
 * getTableForVid
 * Returns a MAC Address Table for a specified VLAN ID
 * or nullptr pointer if it is not found
 */

MacAddressTable::AddressTable *MacAddressTable::getTableForVid(unsigned int vid)
{
    if (vid == 0)
        return addressTable;

    auto iter = vlanAddressTable.find(vid);
    if (iter != vlanAddressTable.end())
        return iter->second;
    return nullptr;
}

int MacAddressTable::getInterfaceIdForAddress(const MacAddress& address, unsigned int vid)
{
    Enter_Method("MacAddressTable::getPortForAddress()");

    AddressTable *table = getTableForVid(vid);
    // VLAN ID vid does not exist
    if (table == nullptr)
        return -1;

    auto iter = table->find(address);

    if (iter == table->end()) {
        // not found
        return -1;
    }
    if (iter->second.insertionTime + agingTime <= simTime()) {
        // don't use (and throw out) aged entries
        EV << "Ignoring and deleting aged entry: " << iter->first << " --> interfaceId " << iter->second.interfaceId << "\n";
        table->erase(iter);
        return -1;
    }
    return iter->second.interfaceId;
}

/*
 * Register a new MAC address at addressTable.
 * True if refreshed. False if it is new.
 */

bool MacAddressTable::updateTableWithAddress(int interfaceId, const MacAddress& address, unsigned int vid)
{
    Enter_Method("MacAddressTable::updateTableWithAddress()");
    if (address.isMulticast())      // broadcast or multicast
        return false;

    AddressTable::iterator iter;
    AddressTable *table = getTableForVid(vid);

    if (table == nullptr) {
        // MAC Address Table does not exist for VLAN ID vid, so we create it
        table = new AddressTable();

        // set 'the addressTable' to VLAN ID 0
        if (vid == 0)
            addressTable = table;

        vlanAddressTable[vid] = table;
        iter = table->end();
    }
    else
        iter = table->find(address);

    if (iter == table->end()) {
        removeAgedEntriesIfNeeded();

        // Add entry to table
        EV << "Adding entry to Address Table: " << address << " --> interfaceId " << interfaceId << "\n";
        (*table)[address] = AddressEntry(vid, interfaceId, simTime());
        return false;
    }
    else {
        // Update existing entry
        EV << "Updating entry in Address Table: " << address << " --> interfaceId " << interfaceId << "\n";
        AddressEntry& entry = iter->second;
        entry.insertionTime = simTime();
        entry.interfaceId = interfaceId;
    }
    return true;
}

/*
 * Clears interfaceId MAC cache.
 */

void MacAddressTable::flush(int interfaceId)
{
    Enter_Method("MacAddressTable::flush():  Clearing interfaceId %d cache", interfaceId);
    for (auto & elem : vlanAddressTable) {
        AddressTable *table = elem.second;
        for (auto j = table->begin(); j != table->end(); ) {
            auto cur = j++;
            if (cur->second.interfaceId == interfaceId)
                table->erase(cur);
        }
    }
}

/*
 * Prints verbose information
 */

void MacAddressTable::printState()
{
    EV << endl << "MAC Address Table" << endl;
    EV << "VLAN ID    MAC    IfId    Inserted" << endl;
    for (auto & elem : vlanAddressTable) {
        AddressTable *table = elem.second;
        for (auto & table_j : *table)
            EV << table_j.second.vid << "   " << table_j.first << "   " << table_j.second.interfaceId << "   " << table_j.second.insertionTime << endl;
    }
}

void MacAddressTable::copyTable(int interfaceIdA, int interfaceIdB)
{
    for (auto & elem : vlanAddressTable) {
        AddressTable *table = elem.second;
        for (auto & table_j : *table)
            if (table_j.second.interfaceId == interfaceIdA)
                table_j.second.interfaceId = interfaceIdB;

    }
}

void MacAddressTable::removeAgedEntriesFromVlan(unsigned int vid)
{
    AddressTable *table = getTableForVid(vid);
    if (table == nullptr)
        return;
    // TODO: this part could be factored out
    for (auto iter = table->begin(); iter != table->end(); ) {
        auto cur = iter++;    // iter will get invalidated after erase()
        AddressEntry& entry = cur->second;
        if (entry.insertionTime + agingTime <= simTime()) {
            EV << "Removing aged entry from Address Table: "
               << cur->first << " --> interfaceId " << cur->second.interfaceId << "\n";
            table->erase(cur);
        }
    }
}

void MacAddressTable::removeAgedEntriesFromAllVlans()
{
    for (auto & elem : vlanAddressTable) {
        AddressTable *table = elem.second;
        // TODO: this part could be factored out
        for (auto j = table->begin(); j != table->end(); ) {
            auto cur = j++;    // iter will get invalidated after erase()
            AddressEntry& entry = cur->second;
            if (entry.insertionTime + agingTime <= simTime()) {
                EV << "Removing aged entry from Address Table: "
                   << cur->first << " --> interfaceId " << cur->second.interfaceId << "\n";
                table->erase(cur);
            }
        }
    }
}

void MacAddressTable::removeAgedEntriesIfNeeded()
{
    simtime_t now = simTime();

    if (now >= lastPurge + 1)
        removeAgedEntriesFromAllVlans();

    lastPurge = simTime();
}

void MacAddressTable::readAddressTable(const char *fileName)
{
    FILE *fp = fopen(fileName, "r");
    if (fp == nullptr)
        throw cRuntimeError("cannot open address table file `%s'", fileName);

    // parse address table file:
    char *line;
    for (int lineno = 0; (line = fgetline(fp)) != nullptr; delete [] line) {
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

        //   parse VLAN ID:
        unsigned int vlanId = strtol(vlanIdStr, &endptr, 10);
        if (!endptr || *endptr)
            throw cRuntimeError("error in line %d in address table file `%s': VLAN ID '%s' unresolved", lineno, fileName, vlanIdStr);

        //   parse MAC address:
        L3Address addr;
        if (! L3AddressResolver().tryResolve(macAddressStr, addr, L3AddressResolver::ADDR_MAC))
            throw cRuntimeError("error in line %d in address table file `%s': MAC address '%s' unresolved", lineno, fileName, macAddressStr);
        MacAddress macAddress = addr.toMac();

        //   parse interface:
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
        AddressTable *table = getTableForVid(entry.vid);

        if (table == nullptr) {
            // MAC Address Table does not exist for VLAN ID vid, so we create it
            table = new AddressTable();

            // set 'the addressTable' to VLAN ID 0
            if (entry.vid == 0)
                addressTable = table;

            vlanAddressTable[entry.vid] = table;
        }

        (*table)[macAddress] = entry;
    }
    fclose(fp);
}

void MacAddressTable::initializeTable()
{
    clearTable();
    // Option to pre-read in Address Table. To turn it off, set addressTableFile to empty string
    const char *addressTableFile = par("addressTableFile");
    if (addressTableFile && *addressTableFile)
        readAddressTable(addressTableFile);

    if (this->addressTable != nullptr) {  // setup a WATCH on VLANID 0 if present
        AddressTable& addressTable = *this->addressTable;    // magic to hide the '*' from the name of the watch below
        WATCH_MAP(addressTable);
    }
}

void MacAddressTable::clearTable()
{
    for (auto & elem : vlanAddressTable)
        delete elem.second;

    vlanAddressTable.clear();
    addressTable = nullptr;
}

MacAddressTable::~MacAddressTable()
{
    for (auto & elem : vlanAddressTable)
        delete elem.second;
}

void MacAddressTable::setAgingTime(simtime_t agingTime)
{
    this->agingTime = agingTime;
}

void MacAddressTable::resetDefaultAging()
{
    agingTime = par("agingTime");
}

} // namespace inet

