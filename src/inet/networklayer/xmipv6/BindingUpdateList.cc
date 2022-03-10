//
// Copyright (C) 2007
// Faqir Zarrar Yousaf
// Communication Networks Institute, Technical University Dortmund (TU Dortmund), Germany.
// Christian Bauer
// Institute of Communications and Navigation, German Aerospace Center (DLR)
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
//

#include "inet/networklayer/xmipv6/BindingUpdateList.h"

#include "inet/common/stlutils.h"
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"

namespace inet {

// secret key used in RR by CN
#define KCN    1

Define_Module(BindingUpdateList);

std::ostream& operator<<(std::ostream& os, const BindingUpdateList::BindingUpdateListEntry& bul)
{
    os << "Destination: " << bul.destAddress << " HoA of MN: " << bul.homeAddress
       << " CoA of MN: " << bul.careOfAddress << "\n"
       << "Binding Lifetime: " << bul.bindingLifetime << " binding expiry: "
       << SIMTIME_STR(bul.bindingExpiry) << " BU Sequence#: " << bul.sequenceNumber
       << " Sent Time: " << SIMTIME_STR(bul.sentTime) /* << " Next_Tx_Time: " << bul.nextBUTx << */
       << " BU_Ack: " << bul.BAck << "\n";

    // this part will only be displayed if the BUL entry is for CN registration
    if (bul.sentHoTI != 0) {
        os << "Sent Time HoTI: " << SIMTIME_STR(bul.sentHoTI) << " HoTI cookie: " << bul.cookieHoTI
           << " home token: " << bul.tokenH << "\n";
    }

    if (bul.sentCoTI != 0) {
        os << " Sent Time CoTI: " << SIMTIME_STR(bul.sentCoTI) << " CoTI cookie: " << bul.cookieCoTI
           << " care-of token: " << bul.tokenC << "\n";
    }

    os << "State: ";
    switch (bul.state) {
        case BindingUpdateList::NONE:
            os << "none";
            break;

        case BindingUpdateList::RR:
            os << "Return Routability";
            break;

        case BindingUpdateList::RR_COMPLETE:
            os << "Return Routability completed";
            break;

        case BindingUpdateList::REGISTER:
            os << "Registering";
            break;

        case BindingUpdateList::REGISTERED:
            os << "Registered";
            break;

        case BindingUpdateList::DEREGISTER:
            os << "Deregistering";
            break;

        default:
            os << "Unknown";
            break;
    }
    os << endl;

    return os;
}

BindingUpdateList::BindingUpdateList()
{
}

BindingUpdateList::~BindingUpdateList()
{
//    for (unsigned int i = 0; i < bindingUpdateList.size(); i++)
//        delete bindingUpdateList[i];
}

void BindingUpdateList::initialize()
{
    WATCH_MAP(bindingUpdateList); // added by Zarrar Yousaf
}

void BindingUpdateList::handleMessage(cMessage *msg)
{
    throw cRuntimeError("This module doesn't process messages");
}

void BindingUpdateList::addOrUpdateBUL(const Ipv6Address& dest, const Ipv6Address& hoa, const Ipv6Address& coa, const uint lifetime, const uint seq, const simtime_t buSentTime) // ,const simtime_t& nextBUSentTime)
{
    // modified structure - CB

    // search for entry
    BindingUpdateList::BindingUpdateListEntry *entry = lookup(dest);

    // if it is not yet existing, create it
    if (entry == nullptr) {
        /*bindingUpdateList[dest].destAddress = dest;
           entry = & bindingUpdateList[dest];
           initializeBUValues(*entry);*/
        entry = createBULEntry(dest);
    }

    EV_INFO << "\n++++++++++++++++++++Binding Update List Being Updated in Routing Table6 ++++++++++++++\n";

    entry->homeAddress = hoa;
    entry->careOfAddress = coa;
    // update lifetime 11.06.08 - CB
    entry->bindingLifetime = lifetime; // for the moment a constant but then it is supposed to decrement with time.
    entry->bindingExpiry = simTime() + lifetime; // binding expires at this point in time
    // TODO bindingUpdateList[dest].remainingLifetime = ;
    entry->sentTime = buSentTime; // the time at which the BU, whose ack is awaited is sent
//    entry->nextBUTx = nextBUSentTime; //the nextScgheduledTime at which the BU will be sent in case of timeout.
    entry->sequenceNumber = seq; // seq number of the last BU sent.
    entry->BAck = false;
}

BindingUpdateList::BindingUpdateListEntry *BindingUpdateList::createBULEntry(const Ipv6Address& dest)
{
    bindingUpdateList[dest].destAddress = dest;

    BindingUpdateListEntry& entry = bindingUpdateList[dest];
//    BindingUpdateList::BindingUpdateListEntry* entry = & bindingUpdateList[dest];
    initializeBUValues(entry);

    return &entry;
}

void BindingUpdateList::initializeBUValues(BindingUpdateListEntry& entry)
{
    // normal BU values
    entry.bindingLifetime = 0;
    entry.bindingExpiry = 0;
    // TODO bindingUpdateList[dest].remainingLifetime = ;
    entry.sentTime = 0;
//    entry.nextBUTx = 0;
    entry.sequenceNumber = 0;
    entry.BAck = false;

    // RR specific values
    entry.sentHoTI = 0;
    entry.sentCoTI = 0;
    entry.cookieHoTI = UNDEFINED_COOKIE;
    entry.cookieCoTI = UNDEFINED_COOKIE;
//    entry.sendNext = 0;
    entry.tokenH = UNDEFINED_TOKEN;
    entry.tokenC = UNDEFINED_TOKEN;
    // 21.07.08 - CB
    entry.state = NONE;
}

void BindingUpdateList::addOrUpdateBUL(const Ipv6Address& dest, const Ipv6Address& addr,
        const simtime_t sentTime, const int cookie, bool HoTI = false)
{
    EV_INFO << "\n++++++++++++++++++++Binding Update List for HoTI/CoTI Being Updated in Routing Table6 ++++++++++++++\n";
    // search for entry
    BindingUpdateList::BindingUpdateListEntry *entry = lookup(dest);

    // if it is not yet existing, create it
    if (entry == nullptr) {
        bindingUpdateList[dest].destAddress = dest;
        entry = &bindingUpdateList[dest];
        initializeBUValues(*entry);
    }

    if (HoTI) { // those values are from the HoTI message
        entry->homeAddress = addr;
        entry->sentHoTI = sentTime;
        entry->cookieHoTI = cookie;
    }
    else { // and those from the CoTI
        entry->careOfAddress = addr;
        entry->sentCoTI = sentTime;
        entry->cookieCoTI = cookie;
    }
}

BindingUpdateList::BindingUpdateListEntry *BindingUpdateList::lookup(const Ipv6Address& dest)
{
    auto i = bindingUpdateList.find(dest);
    return (i == bindingUpdateList.end()) ? nullptr : &(i->second);
}

BindingUpdateList::BindingUpdateListEntry *BindingUpdateList::fetch(const Ipv6Address& dest)
{
    BindingUpdateList::BindingUpdateListEntry *entry = lookup(dest);
    return (entry == nullptr) ? createBULEntry(dest) : entry;
}

BindingUpdateList::MobilityState BindingUpdateList::getMobilityState(const Ipv6Address& dest) const
{
    BindingUpdateList6::const_iterator i = bindingUpdateList.find(dest);
    return (i == bindingUpdateList.end()) ? NONE : i->second.state;
}

void BindingUpdateList::setMobilityState(const Ipv6Address& dest, BindingUpdateList::MobilityState state)
{
    auto i = bindingUpdateList.find(dest);

    if (i != bindingUpdateList.end())
        i->second.state = state;
}

int BindingUpdateList::generateBAuthData(const Ipv6Address& dest, const Ipv6Address& CoA)
{
    BindingUpdateList::BindingUpdateListEntry *entry = lookup(dest);

    if (entry == nullptr) {
        EV_WARN << "Impossible to generate Binding Authorization Data as CN is not existing in BUL!\n";
        return 0;
    }

    // generate the key
    return BindingUpdateList::generateKey(entry->tokenH, entry->tokenC, CoA);
}

int BindingUpdateList::generateKey(int homeToken, int careOfToken, const Ipv6Address& CoA)
{
    // use a dummy value
    return homeToken + careOfToken;
}

int BindingUpdateList::generateHomeToken(const Ipv6Address& HoA, int nonce)
{
    return HO_TOKEN;
}

int BindingUpdateList::generateCareOfToken(const Ipv6Address& CoA, int nonce)
{
    return CO_TOKEN;
}

void BindingUpdateList::resetHomeToken(const Ipv6Address& dest, const Ipv6Address& hoa)
{
    BindingUpdateList::BindingUpdateListEntry *entry = lookup(dest);
    ASSERT(entry != nullptr);

    entry->tokenH = UNDEFINED_TOKEN;
//    entry->sentHoTI = 0;
}

void BindingUpdateList::resetCareOfToken(const Ipv6Address& dest, const Ipv6Address& hoa)
{
    BindingUpdateList::BindingUpdateListEntry *entry = lookup(dest);
    ASSERT(entry != nullptr);

    entry->tokenC = UNDEFINED_TOKEN;
//    entry->sentCoTI = 0;
}

bool BindingUpdateList::isHomeTokenAvailable(const Ipv6Address& dest, NetworkInterface *ie)
{
    BindingUpdateList::BindingUpdateListEntry *entry = lookup(dest);
    ASSERT(entry != nullptr);

    return entry->tokenH != UNDEFINED_TOKEN &&
           (entry->sentHoTI + ie->getProtocolData<Ipv6InterfaceData>()->_getMaxTokenLifeTime()) > simTime();
}

bool BindingUpdateList::isCareOfTokenAvailable(const Ipv6Address& dest, NetworkInterface *ie)
{
    BindingUpdateList::BindingUpdateListEntry *entry = lookup(dest);
    ASSERT(entry != nullptr);

    return entry->tokenC != UNDEFINED_TOKEN &&
           (entry->sentCoTI + ie->getProtocolData<Ipv6InterfaceData>()->_getMaxTokenLifeTime()) > simTime();
}

bool BindingUpdateList::isInBindingUpdateList(const Ipv6Address& dest) const
{
    return containsKey(bindingUpdateList, dest);
}

uint BindingUpdateList::getSequenceNumber(const Ipv6Address& dest)
{
    // search for entry
    BindingUpdateList::BindingUpdateListEntry *entry = lookup(dest);
    return (entry != nullptr) ? entry->sequenceNumber : 0;
}

const Ipv6Address& BindingUpdateList::getCoA(const Ipv6Address& dest)
{
    // search for entry
    BindingUpdateList::BindingUpdateListEntry *entry = lookup(dest);

    ASSERT(entry != nullptr);

    return entry->careOfAddress;
}

bool BindingUpdateList::isInBindingUpdateList(const Ipv6Address& dest, const Ipv6Address& HoA)
{
    auto pos = bindingUpdateList.find(dest);
    return (pos == bindingUpdateList.end()) ? false : pos->second.homeAddress == HoA;
}

bool BindingUpdateList::isValidBinding(const Ipv6Address& dest)
{
    BindingUpdateList::BindingUpdateListEntry *entry = lookup(dest);
    return (entry == nullptr) ? false : entry->BAck && (entry->bindingLifetime < SIMTIME_DBL(simTime()));
}

bool BindingUpdateList::isBindingAboutToExpire(const Ipv6Address& dest)
{
    BindingUpdateList::BindingUpdateListEntry *entry = lookup(dest);
    return (entry == nullptr) ? true : entry->bindingLifetime < SIMTIME_DBL(simTime()) - PRE_BINDING_EXPIRY;
}

bool BindingUpdateList::sentBindingUpdate(const Ipv6Address& dest)
{
    BindingUpdateList::BindingUpdateListEntry *entry = lookup(dest);

    if (entry == nullptr)
        return false;

    return (entry->BAck || (entry->tokenH != UNDEFINED_TOKEN && entry->tokenC != UNDEFINED_TOKEN))
           && entry->sentTime != 0;
}

void BindingUpdateList::removeBinding(const Ipv6Address& dest)
{
    BindingUpdateList::BindingUpdateListEntry *entry = lookup(dest);

    ASSERT(entry != nullptr);

    if ((entry->tokenH != UNDEFINED_TOKEN) || (entry->tokenC != UNDEFINED_TOKEN))
        // for CNs, we just delete all entries
        resetBindingCacheEntry(*entry);
    else
        // the BUL entry to the HA is completely deleted
        bindingUpdateList.erase(dest);
}

void BindingUpdateList::suspendBinding(const Ipv6Address& dest)
{
    BindingUpdateList::BindingUpdateListEntry *entry = lookup(dest);

    ASSERT(entry != nullptr);

    entry->BAck = false;
}

bool BindingUpdateList::recentlySentCOTI(const Ipv6Address& dest, NetworkInterface *ie)
{
    BindingUpdateList::BindingUpdateListEntry *entry = lookup(dest);

    ASSERT(entry != nullptr);

    return entry->sentCoTI + ie->getProtocolData<Ipv6InterfaceData>()->_getMaxTokenLifeTime() / 3 > simTime();
}

bool BindingUpdateList::recentlySentHOTI(const Ipv6Address& dest, NetworkInterface *ie)
{
    BindingUpdateList::BindingUpdateListEntry *entry = lookup(dest);

    ASSERT(entry != nullptr);

    return entry->sentHoTI + ie->getProtocolData<Ipv6InterfaceData>()->_getMaxTokenLifeTime() / 3 > simTime();
}

void BindingUpdateList::resetBindingCacheEntry(BindingUpdateListEntry& entry)
{
    entry.bindingLifetime = 0;
    entry.bindingExpiry = 0;
//    entry.remainingLifetime = 0;
//    entry.sequenceNumber = 0;
    entry.sentTime = 0;
//    entry.nextBUTx = 0;
    entry.BAck = false;
    entry.state = NONE;

    // if tokens should sustain handovers then comment out the following lines of code
    // (this could eventually allow for parallel CN and HA registration)
    /*entry.sentHoTI = 0;
       entry.sentCoTI = 0;
       entry.tokenH = 0;
       entry.tokenC = 0;*/
}

} // namespace inet

