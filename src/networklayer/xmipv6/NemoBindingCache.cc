/**
 * Copyright (C) 2007
 * Faqir Zarrar Yousaf
 * Communication Networks Institute, University of Dortmund, Germany.
 * Christian Bauer
 * Institute of Communications and Navigation, German Aerospace Center (DLR)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "NemoBindingCache.h"

Define_Module(NemoBindingCache);

std::ostream& operator<<(std::ostream& os, const NemoBindingCache::NemoBindingCacheEntry& nbce)
{
    os << "CoA of MN:" << nbce.careOfAddress << " BU Lifetime: " << nbce.bindingLifetime
       << " Home Registeration: " << nbce.isHomeRegisteration << " BU_Sequence#: "
       << nbce.sequenceNumber << "\n";

    return os;
}

NemoBindingCache::NemoBindingCache()
{
}

NemoBindingCache::~NemoBindingCache()
{
//     for (unsigned int i = 0; i < bindingUpdateList.size(); i++)
//         delete bindingUpdateList[i];
}

void NemoBindingCache::initialize()
{
    WATCH_MAP(nemoBindingCache); //added by Zarrar Yousaf
}

void NemoBindingCache::handleMessage(cMessage *msg)
{
    opp_error("This module doesn't process messages");
}

void NemoBindingCache::addOrUpdateBC(const IPv6Address& hoa, const IPv6Address& coa,
                                 const uint lifetime, const uint seq, bool homeReg)
{
    EV << "\n++++++++++++++++++++Binding Cache Being Updated in Routing Table6 ++++++++++++++\n";
    nemoBindingCache[hoa].careOfAddress = coa;
    nemoBindingCache[hoa].bindingLifetime = lifetime;
    nemoBindingCache[hoa].sequenceNumber = seq;
    nemoBindingCache[hoa].isHomeRegisteration = homeReg;
}

uint NemoBindingCache::readBCSequenceNumber(const IPv6Address& HoA) const
{
    //Reads the sequence number of the last received BU Message
    /*IPv6Address HoA = bu->getHomeAddressMN();
    uint seqNumber = bindingCache[HoA].sequenceNumber;
    return seqNumber;*/

    // update 10.09.07 - CB
    // the code from above creates a new (empty) entry if
    // the provided HoA does not yet exist.
    NemoBindingCache6::const_iterator pos = nemoBindingCache.find(HoA);

    if (pos == nemoBindingCache.end())
        return 0; // HoA not yet registered
    else
        return pos->second.sequenceNumber;
}

bool NemoBindingCache::isInBindingCache(const IPv6Address& HoA, IPv6Address& CoA) const
{
    NemoBindingCache6::const_iterator pos = nemoBindingCache.find(HoA);

    if (pos == nemoBindingCache.end())
        return false; // if HoA is not registered then there's obviously no valid entry in the BC

    return (pos->second.careOfAddress == CoA); // if CoA corresponds to HoA, everything is fine
}

bool NemoBindingCache::isInBindingCache(const IPv6Address& HoA) const
{
    return nemoBindingCache.find(HoA) != nemoBindingCache.end();
}

void NemoBindingCache::deleteEntry(IPv6Address& HoA)
{
    NemoBindingCache6::iterator pos = nemoBindingCache.find(HoA);

    if (pos != nemoBindingCache.end()) // update 11.9.07 - CB
        nemoBindingCache.erase(pos);
}

bool NemoBindingCache::getHomeRegistration(const IPv6Address& HoA) const
{
    NemoBindingCache6::const_iterator pos = nemoBindingCache.find(HoA);

    if (pos == nemoBindingCache.end())
        return false; // HoA not yet registered; should not occur anyway
    else
        return pos->second.isHomeRegisteration;
}

uint NemoBindingCache::getLifetime(const IPv6Address& HoA) const
{
    NemoBindingCache6::const_iterator pos = nemoBindingCache.find(HoA);

    if (pos == nemoBindingCache.end())
        return 0; // HoA not yet registered; should not occur anyway
    else
        return pos->second.bindingLifetime;
}

bool NemoBindingCache::getMobileRouter(const IPv6Address& HoA) const
{
    NemoBindingCache6::const_iterator pos = nemoBindingCache.find(HoA);

        if (pos == nemoBindingCache.end())
            return 0; // HoA not yet registered; should not occur anyway
        else
            return pos->second.mobileRouter;
}
