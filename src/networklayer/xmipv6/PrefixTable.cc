/*
 * PrefixTable.cc
 *
 *  Created on: Mar 26, 2015
 *      Author: d8
 */


#include "PrefixTable.h"

Define_Module(PrefixTable);

std::ostream& operator<<(std::ostream& os, const PrefixTable::PrefixTableEntry& pte)
{
    os << "MobileNetworkPrefix:" << pte.mobileNetworkPrefix << "\n";

    return os;
}

PrefixTable::PrefixTable()
{
}

PrefixTable::~PrefixTable()
{
}

void PrefixTable::initialize()
{
    WATCH_MAP(prefixTable);
}

void PrefixTable::handleMessage(cMessage *msg)
{
    opp_error("This module doesn't process messages");
}

void PrefixTable::addOrUpdatePT(const IPv6Address& HoA, const IPv6Address& prefix)
{
    EV << "\n+++++++++++++++++ Prefix Table Being Updated in Routing Table6 +++++++++++++++++\n";
    if (prefixTable.find(HoA) != prefixTable.end())
    {
        EV << "A record in Prefix Table is being updated...\n";
        prefixTable[HoA].mobileNetworkPrefix = prefix;
    }
    else
    {
        EV << "A new data is being recorded in Prefix Table...\n";
        PrefixTableEntry &entry = prefixTable[HoA];
        entry.mobileNetworkPrefix = prefix;
    }
    EV << "key (HoA) = " << HoA << ", with prefix = " << prefix << endl;

}

bool PrefixTable::isInPrefixTable(const IPv6Address& HoA) const
{
    return prefixTable.find(HoA) != prefixTable.end();
}

void PrefixTable::deleteEntry(IPv6Address& HoA)
{
    PrefixTable6::iterator pos = prefixTable.find(HoA);

    if (pos != prefixTable.end())
        prefixTable.erase(pos);
}

IPv6Address PrefixTable::getPrefix(const IPv6Address& HoA) const
{
    PrefixTable6::const_iterator pos = prefixTable.find(HoA);

    if (pos == prefixTable.end())
        return IPv6Address::UNSPECIFIED_ADDRESS;
    else
        return pos->second.mobileNetworkPrefix;
}

IPv6Address PrefixTable::getLastPrefix() const
{

    if (prefixTable.empty())
            return IPv6Address::UNSPECIFIED_ADDRESS;

    PrefixTable6::const_iterator pos = prefixTable.end();
    --pos;
    return pos->second.mobileNetworkPrefix;
}

IPv6Address PrefixTable::getLastSubPrefix() const
{
    IPv6Address lastSubPrefix(0,0,0,0);
    IPv6Address tempPrefix;
    uint32 oldSegment1, newSegment1;

    PrefixTable6::const_iterator pos;
    for (pos = prefixTable.begin(); pos!=prefixTable.end(); pos++)
    {
        tempPrefix = pos->second.mobileNetworkPrefix;

        oldSegment1 = lastSubPrefix.getSegment1();
        newSegment1 = tempPrefix.getSegment1();
        if( newSegment1 > oldSegment1 )
            lastSubPrefix = pos->second.mobileNetworkPrefix;
    }

    return lastSubPrefix;
}
