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
#include "inet/linklayer/ethernet/switch/MACAddressTable.h"

namespace inet {

#define MAX_LINE    100

Define_Module(MACAddressTable);

std::ostream& operator<<(std::ostream& os, const MACAddressTable::AddressEntry& entry)
{
    os << "{VID=" << entry.vid << ", port=" << entry.portno << ", insertionTime=" << entry.insertionTime << "}";
    return os;
}

MACAddressTable::MACAddressTable()
{
    addressTable = new AddressTable();
    // Set addressTable for VLAN ID 0
    vlanAddressTable[0] = addressTable;
}

void MACAddressTable::initialize()
{
    agingTime = par("agingTime");
    lastPurge = SIMTIME_ZERO;

    // Option to pre-read in Address Table. To turn it off, set addressTableFile to empty string
    const char *addressTableFile = par("addressTableFile");
    if (addressTableFile && *addressTableFile)
        readAddressTable(addressTableFile);

    AddressTable& addressTable = *this->addressTable;    // magic to hide the '*' from the name of the watch below
    WATCH_MAP(addressTable);
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
    if (fgets(line, MAX_LINE, fp) == NULL) {
        delete[] line;
        return NULL;
    }

    // chop CR/LF
    line[MAX_LINE - 1] = '\0';
    int len = strlen(line);
    while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
        line[--len] = '\0';

    return line;
}

void MACAddressTable::handleMessage(cMessage *)
{
    throw cRuntimeError("This module doesn't process messages");
}

/*
 * getTableForVid
 * Returns a MAC Address Table for a specified VLAN ID
 * or NULL pointer if it is not found
 */

MACAddressTable::AddressTable *MACAddressTable::getTableForVid(unsigned int vid)
{
    if (vid == 0)
        return addressTable;

    MACAddressTable::VlanAddressTable::iterator iter;
    iter = vlanAddressTable.find(vid);
    if (iter != vlanAddressTable.end())
        return iter->second;
    return NULL;
}

/*
 * For a known arriving port, V-TAG and destination MAC. It generates a vector with the ports where relay component
 * should deliver the message.
 * returns false if not found
 */

int MACAddressTable::getPortForAddress(MACAddress& address, unsigned int vid)
{
    Enter_Method("MACAddressTable::getPortForAddress()");

    AddressTable *table = getTableForVid(vid);
    // VLAN ID vid does not exist
    if (table == NULL)
        return -1;

    AddressTable::iterator iter = table->find(address);

    if (iter == table->end()) {
        // not found
        return -1;
    }
    if (iter->second.insertionTime + agingTime <= simTime()) {
        // don't use (and throw out) aged entries
        EV << "Ignoring and deleting aged entry: " << iter->first << " --> port" << iter->second.portno << "\n";
        table->erase(iter);
        return -1;
    }
    return iter->second.portno;
}

/*
 * Register a new MAC address at addressTable.
 * True if refreshed. False if it is new.
 */

bool MACAddressTable::updateTableWithAddress(int portno, MACAddress& address, unsigned int vid)
{
    Enter_Method("MACAddressTable::updateTableWithAddress()");
    if (address.isBroadcast())
        return false;

    AddressTable::iterator iter;
    AddressTable *table = getTableForVid(vid);

    if (table == NULL) {
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
        EV << "Adding entry to Address Table: " << address << " --> port" << portno << "\n";
        (*table)[address] = AddressEntry(vid, portno, simTime());
        return false;
    }
    else {
        // Update existing entry
        EV << "Updating entry in Address Table: " << address << " --> port" << portno << "\n";
        AddressEntry& entry = iter->second;
        entry.insertionTime = simTime();
        entry.portno = portno;
    }
    return true;
}

/*
 * Clears portno MAC cache.
 */

void MACAddressTable::flush(int portno)
{
    Enter_Method("MACAddressTable::flush():  Clearing gate %d cache", portno);
    for (VlanAddressTable::iterator i = vlanAddressTable.begin(); i != vlanAddressTable.end(); i++) {
        AddressTable *table = i->second;
        for (AddressTable::iterator j = table->begin(); j != table->end(); ) {
            AddressTable::iterator cur = j++;
            if (cur->second.portno == portno)
                table->erase(cur);
        }
    }
}

/*
 * Prints verbose information
 */

void MACAddressTable::printState()
{
    EV << endl << "MAC Address Table" << endl;
    EV << "VLAN ID    MAC    Port    Inserted" << endl;
    for (VlanAddressTable::iterator i = vlanAddressTable.begin(); i != vlanAddressTable.end(); i++) {
        AddressTable *table = i->second;
        for (AddressTable::iterator j = table->begin(); j != table->end(); j++)
            EV << j->second.vid << "   " << j->first << "   " << j->second.portno << "   " << j->second.insertionTime << endl;
    }
}

void MACAddressTable::copyTable(int portA, int portB)
{
    for (VlanAddressTable::iterator i = vlanAddressTable.begin(); i != vlanAddressTable.end(); i++) {
        AddressTable *table = i->second;
        for (AddressTable::iterator j = table->begin(); j != table->end(); j++)
            if (j->second.portno == portA)
                j->second.portno = portB;

    }
}

void MACAddressTable::removeAgedEntriesFromVlan(unsigned int vid)
{
    AddressTable *table = getTableForVid(vid);
    if (table == NULL)
        return;
    // TODO: this part could be factored out
    for (AddressTable::iterator iter = table->begin(); iter != table->end(); ) {
        AddressTable::iterator cur = iter++;    // iter will get invalidated after erase()
        AddressEntry& entry = cur->second;
        if (entry.insertionTime + agingTime <= simTime()) {
            EV << "Removing aged entry from Address Table: "
               << cur->first << " --> port" << cur->second.portno << "\n";
            table->erase(cur);
        }
    }
}

void MACAddressTable::removeAgedEntriesFromAllVlans()
{
    for (VlanAddressTable::iterator i = vlanAddressTable.begin(); i != vlanAddressTable.end(); i++) {
        AddressTable *table = i->second;
        // TODO: this part could be factored out
        for (AddressTable::iterator j = table->begin(); j != table->end(); ) {
            AddressTable::iterator cur = j++;    // iter will get invalidated after erase()
            AddressEntry& entry = cur->second;
            if (entry.insertionTime + agingTime <= simTime()) {
                EV << "Removing aged entry from Address Table: "
                   << cur->first << " --> port" << cur->second.portno << "\n";
                table->erase(cur);
            }
        }
    }
}

void MACAddressTable::removeAgedEntriesIfNeeded()
{
    simtime_t now = simTime();

    if (now >= lastPurge + 1)
        removeAgedEntriesFromAllVlans();

    lastPurge = simTime();
}

void MACAddressTable::readAddressTable(const char *fileName)
{
    FILE *fp = fopen(fileName, "r");
    if (fp == NULL)
        error("cannot open address table file `%s'", fileName);

    //  Syntax of the file goes as:
    //  VLAN ID, address in hexadecimal representation, portno
    //  1    ffffffff    1
    //  1    ffffeed1    2
    //  2    aabcdeff    3
    //
    //  etc...
    //
    //  Each iteration of the loop reads in an entire line i.e. up to '\n' or EOF characters
    //  and uses strtok to extract tokens from the resulting string
    char *line;
    int lineno = 0;
    while ((line = fgetline(fp)) != NULL) {
        lineno++;

        // lines beginning with '#' are treated as comments
        if (line[0] == '#')
            continue;

        // scan in VLAN ID
        char *vlanID = strtok(line, " \t");
        // scan in hexaddress
        char *hexaddress = strtok(NULL, " \t");
        // scan in port number
        char *portno = strtok(NULL, " \t");

        // empty line?
        if (!vlanID)
            continue;

        // broken line?
        if (!portno || !hexaddress)
            error("line %d invalid in address table file `%s'", lineno, fileName);

        // Create an entry with address and portno and insert into table
        AddressEntry entry(atoi(vlanID), atoi(portno), 0);
        AddressTable *table = getTableForVid(entry.vid);

        if (table == NULL) {
            table = new AddressTable();
            vlanAddressTable[entry.vid] = table;
        }

        (*table)[MACAddress(hexaddress)] = entry;

        // Garbage collection before next iteration
        delete[] line;
    }
    fclose(fp);
}

void MACAddressTable::clearTable()
{
    for (VlanAddressTable::iterator iter = vlanAddressTable.begin(); iter != vlanAddressTable.end(); iter++)
        delete iter->second;

    vlanAddressTable.clear();
    addressTable = NULL;
}

MACAddressTable::~MACAddressTable()
{
    for (VlanAddressTable::iterator iter = vlanAddressTable.begin(); iter != vlanAddressTable.end(); iter++)
        delete iter->second;
}

void MACAddressTable::setAgingTime(simtime_t agingTime)
{
    this->agingTime = agingTime;
}

void MACAddressTable::resetDefaultAging()
{
    agingTime = par("agingTime");
}

} // namespace inet

