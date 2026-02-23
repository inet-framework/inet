//
// Copyright (C) 2007
// Faqir Zarrar Yousaf
// Communication Networks Institute, University of Dortmund, Germany.
// Christian Bauer
// Institute of Communications and Navigation, German Aerospace Center (DLR)
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//

#include "inet/networklayer/xmipv6/BindingCache.h"

#include "inet/common/stlutils.h"

namespace inet {

Define_Module(BindingCache);

std::ostream& operator<<(std::ostream& os, const BindingCache::BindingCacheEntry& bce)
{
    os << "CoA of MN:" << bce.careOfAddress << " BU Lifetime: " << bce.bindingLifetime
       << " Home Registeration: " << bce.isHomeRegisteration << " BU_Sequence#: "
       << bce.sequenceNumber << "\n";

    return os;
}

BindingCache::BindingCache()
{
}

BindingCache::~BindingCache()
{
//     for (unsigned int i = 0; i < bindingUpdateList.size(); i++)
//         delete bindingUpdateList[i];
}

void BindingCache::initialize()
{
    WATCH_MAP(bindingCache); // added by Zarrar Yousaf
}

void BindingCache::handleMessage(cMessage *msg)
{
    throw cRuntimeError("This module doesn't process messages");
}

void BindingCache::addOrUpdateBC(const Ipv6Address& hoa, const Ipv6Address& coa,
        const uint lifetime, const uint seq, bool homeReg)
{
    EV_INFO << "\n++++++++++++++++++++Binding Cache Being Updated in Routing Table6 ++++++++++++++\n";
    bindingCache[hoa].careOfAddress = coa;
    bindingCache[hoa].bindingLifetime = lifetime;
    bindingCache[hoa].sequenceNumber = seq;
    bindingCache[hoa].isHomeRegisteration = homeReg;
}

uint BindingCache::readBCSequenceNumber(const Ipv6Address& HoA) const
{
    // Reads the sequence number of the last received BU Message
    /*Ipv6Address HoA = bu->getHomeAddressMN();
       uint seqNumber = bindingCache[HoA].sequenceNumber;
       return seqNumber;*/

    // update 10.09.07 - CB
    // the code from above creates a new (empty) entry if
    // the provided HoA does not yet exist.
    BindingCache6::const_iterator pos = bindingCache.find(HoA);
    return (pos == bindingCache.end()) ? 0 : pos->second.sequenceNumber;
}

bool BindingCache::isInBindingCache(const Ipv6Address& HoA, const Ipv6Address& CoA) const
{
    BindingCache6::const_iterator pos = bindingCache.find(HoA);
    return (pos == bindingCache.end()) ? false : pos->second.careOfAddress == CoA;
}

bool BindingCache::isInBindingCache(const Ipv6Address& HoA) const
{
    return containsKey(bindingCache, HoA);
}

void BindingCache::deleteEntry(const Ipv6Address& HoA)
{
    auto pos = bindingCache.find(HoA);
    if (pos != bindingCache.end())
        bindingCache.erase(pos);
}

bool BindingCache::getHomeRegistration(const Ipv6Address& HoA) const
{
    BindingCache6::const_iterator pos = bindingCache.find(HoA);
    return (pos == bindingCache.end()) ? false : pos->second.isHomeRegisteration;
}

uint BindingCache::getLifetime(const Ipv6Address& HoA) const
{
    BindingCache6::const_iterator pos = bindingCache.find(HoA);
    return (pos == bindingCache.end()) ? 0 : pos->second.bindingLifetime;
}

int BindingCache::generateHomeToken(const Ipv6Address& HoA, int nonce)
{
    return HO_TOKEN;
}

int BindingCache::generateCareOfToken(const Ipv6Address& CoA, int nonce)
{
    return CO_TOKEN;
}

int BindingCache::generateKey(int homeToken, int careOfToken, const Ipv6Address& CoA)
{
    // use a dummy value
    return homeToken + careOfToken;
}

} // namespace inet

