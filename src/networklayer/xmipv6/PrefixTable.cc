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
    prefixTable[HoA].mobileNetworkPrefix = prefix;
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

//kayaknya fungsi ini bakal muspro
IPv6Address PrefixTable::getLastPrefix() const
{
    PrefixTable6::const_iterator pos = prefixTable.end();
    --pos;
    return pos->second.mobileNetworkPrefix;
}

//todo:: new function:: getNextSubPrefix :: dari prefix yg ada --> dicari satu per satu for i=0 until end (isInPrefixTable)
