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

#include "MACAddressTable.h"

Define_Module(MACAddressTable);

void MACAddressTable::initialize()
{
    agingTime=(simtime_t) par("agingTime");
    verbose=(bool) par ("verbose");
}

void MACAddressTable::handleMessage(cMessage *)
{
        opp_error("This module doesn't process messages");
}

/*
 * RESOLVEMAC
 * For a known arriving port, V-TAG and destination MAC. It generates a vector with the ports where relay component
 * should deliver the message.
 * returns false if not found
 */
bool MACAddressTable::resolveMAC(std::vector <int> * outputPorts, MACAddress MACDest, unsigned int Vid)
{
    Enter_Method("Obtaining output ports");
    cleanAgedEntries();
    bool result=false;
    outputPorts->clear();
    for(unsigned int i=0; i<RelayTable.size(); i++)
    {
        if(Vid != RelayTable[i].Vid)
            continue;
        if(MACDest != RelayTable[i].MAC)
            continue;
        outputPorts->push_back(RelayTable[i].Gate);
        result=true;
    }
    return result;
}

/*
 * REGISTERMAC
 * Register a new MAC at RelayTable.
 * True if refreshed. False if it is new.
 */
bool MACAddressTable::registerMAC(int Gate, MACAddress MAC, unsigned int Vid)
{
    bool exists=false;
    if(!MAC.isBroadcast())
    {
        for(unsigned int i=0; i<RelayTable.size(); i++)
        {//Looking for the entry
            if((Vid == RelayTable[i].Vid)&&(MAC == RelayTable[i].MAC)&&(Gate==RelayTable[i].Gate))
            {
                RelayTable[i].inserted=simulation.getSimTime();
                exists=true;
            }
        }
        if(exists==false)
        {//It is a new entry.
            Enter_Method("Registering MAC");
            RelayEntry Entry;
            Entry.MAC=MAC;
            Entry.Vid=Vid;
            Entry.Gate=Gate;
            Entry.inserted = simulation.getSimTime();
            RelayTable.push_back(Entry);
        }
    }
    return exists;
}

/*
 * Clears gate MAC cache.
 */
void MACAddressTable::flush(int Gate)
{
    char buf[50];
    sprintf(buf,"Clearing gate %d cache.",Gate);
    Enter_Method(buf);
    for(unsigned int i=0;i<RelayTable.size();i++)
    {
        if(RelayTable[i].Gate==Gate)
        {
            RelayTable.erase(RelayTable.begin()+i);
            i--; //Keep position.
        }
    }
}

/*
 * Prints verbose information
 */
void MACAddressTable::printState()
{
    ev<<endl<<"RelayTable"<<endl;
    ev<<"Vid    MAC    Gate    Inserted"<<endl;
    for(unsigned int i=0;i<RelayTable.size();i++)
    {
        ev<<RelayTable[i].Vid<<"   "<<RelayTable[i].MAC<<"    "<<RelayTable[i].Gate<<"    "<<RelayTable[i].inserted<<endl;
    }
}

void MACAddressTable::cpCache(int a, int b)
{ //Copy from a to b
    for(unsigned int i=0;i<RelayTable.size();i++)
    {
        if(RelayTable[i].Gate==a)
        {
            RelayEntry Entry;
            Entry.MAC=RelayTable[i].MAC;
            Entry.Vid=RelayTable[i].Vid;
            Entry.Gate=b;
            Entry.inserted = RelayTable[i].inserted;
            RelayTable.push_back(Entry);
        }
    }
}

void MACAddressTable::cleanAgedEntries()
{
    for(unsigned int i=0;i<RelayTable.size();i++)
    {
        if((simulation.getSimTime()-(SimTime)(RelayTable[i].inserted)) > (SimTime) agingTime)
        {//Delete entry
            if(verbose==true)
                ev<<"Deleting old entry."<<endl;
            RelayTable.erase(RelayTable.begin()+i);
            i--;
        }
    }
}

