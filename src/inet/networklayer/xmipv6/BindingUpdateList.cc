/**
 * Copyright (C) 2007
 * Faqir Zarrar Yousaf
 * Communication Networks Institute, Technical University Dortmund (TU Dortmund), Germany.
 * Christian Bauer
 * Institute of Communications and Navigation, German Aerospace Center (DLR)

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

#include "inet/networklayer/xmipv6/BindingUpdateList.h"

#include "inet/networklayer/ipv6/IPv6InterfaceData.h"

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
    WATCH_MAP(bindingUpdateList);    //added by Zarrar Yousaf
}

void BindingUpdateList::handleMessage(cMessage *msg)
{
    throw cRuntimeError("This module doesn't process messages");
}

void BindingUpdateList::addOrUpdateBUL(const IPv6Address& dest, const IPv6Address& hoa, const IPv6Address& coa, const uint lifetime, const uint seq, const simtime_t buSentTime)    //,const simtime_t& nextBUSentTime)
{
    // modified structure - CB

    // search for entry
    BindingUpdateList::BindingUpdateListEntry *entry = lookup(dest);

    // if it is not yet existing, create it
    if (entry == NULL) {
        /*bindingUpdateList[dest].destAddress = dest;
           entry = & bindingUpdateList[dest];
           initializeBUValues(*entry);*/
        entry = createBULEntry(dest);
    }

    EV_INFO << "\n++++++++++++++++++++Binding Update List Being Updated in Routing Table6 ++++++++++++++\n";

    entry->homeAddress = hoa;
    entry->careOfAddress = coa;
    // update lifetime 11.06.08 - CB
    entry->bindingLifetime = lifetime;    // for the moment a constant but then it is supposed to decrement with time.
    entry->bindingExpiry = simTime() + lifetime;    // binding expires at this point in time
    //TODO bindingUpdateList[dest].remainingLifetime = ;
    entry->sentTime = buSentTime;    //the time at which the BU, whose ack is awaited is sent
    //entry->nextBUTx = nextBUSentTime; //the nextScgheduledTime at which the BU will be sent in case of timeout.
    entry->sequenceNumber = seq;    //seq number of the last BU sent.
    entry->BAck = false;
}

BindingUpdateList::BindingUpdateListEntry *BindingUpdateList::createBULEntry(const IPv6Address& dest)
{
    bindingUpdateList[dest].destAddress = dest;

    BindingUpdateListEntry& entry = bindingUpdateList[dest];
    //BindingUpdateList::BindingUpdateListEntry* entry = & bindingUpdateList[dest];
    initializeBUValues(entry);

    return &entry;
}

void BindingUpdateList::initializeBUValues(BindingUpdateListEntry& entry)
{
    // normal BU values
    entry.bindingLifetime = 0;
    entry.bindingExpiry = 0;
    //TODO bindingUpdateList[dest].remainingLifetime = ;
    entry.sentTime = 0;
    //entry.nextBUTx = 0;
    entry.sequenceNumber = 0;
    entry.BAck = false;

    // RR specific values
    entry.sentHoTI = 0;
    entry.sentCoTI = 0;
    entry.cookieHoTI = UNDEFINED_COOKIE;
    entry.cookieCoTI = UNDEFINED_COOKIE;
    //entry.sendNext = 0;
    entry.tokenH = UNDEFINED_TOKEN;
    entry.tokenC = UNDEFINED_TOKEN;
    // 21.07.08 - CB
    entry.state = NONE;
}

void BindingUpdateList::addOrUpdateBUL(const IPv6Address& dest, const IPv6Address& addr,
        const simtime_t sentTime, const int cookie, bool HoTI = false)
{
    EV_INFO << "\n++++++++++++++++++++Binding Update List for HoTI/CoTI Being Updated in Routing Table6 ++++++++++++++\n";
    // search for entry
    BindingUpdateList::BindingUpdateListEntry *entry = lookup(dest);

    // if it is not yet existing, create it
    if (entry == NULL) {
        bindingUpdateList[dest].destAddress = dest;
        entry = &bindingUpdateList[dest];
        initializeBUValues(*entry);
    }

    if (HoTI) {    // those values are from the HoTI message
        entry->homeAddress = addr;
        entry->sentHoTI = sentTime;
        entry->cookieHoTI = cookie;
    }
    else {    // and those from the CoTI
        entry->careOfAddress = addr;
        entry->sentCoTI = sentTime;
        entry->cookieCoTI = cookie;
    }
}

BindingUpdateList::BindingUpdateListEntry *BindingUpdateList::lookup(const IPv6Address& dest)
{
    BindingUpdateList6::iterator i = bindingUpdateList.find(dest);

    return (i == bindingUpdateList.end()) ? NULL : &(i->second);
}

BindingUpdateList::BindingUpdateListEntry *BindingUpdateList::fetch(const IPv6Address& dest)
{
    BindingUpdateList::BindingUpdateListEntry *entry = lookup(dest);

    if (entry == NULL)
        return createBULEntry(dest);
    else
        return entry;
}

BindingUpdateList::MobilityState BindingUpdateList::getMobilityState(const IPv6Address& dest) const
{
    BindingUpdateList6::const_iterator i = bindingUpdateList.find(dest);

    if (i == bindingUpdateList.end())
        return NONE;
    else
        return i->second.state;
}

void BindingUpdateList::setMobilityState(const IPv6Address& dest, BindingUpdateList::MobilityState state)
{
    BindingUpdateList6::iterator i = bindingUpdateList.find(dest);

    if (i != bindingUpdateList.end())
        i->second.state = state;
}

int BindingUpdateList::generateBAuthData(const IPv6Address& dest, const IPv6Address& CoA)
{
    BindingUpdateList::BindingUpdateListEntry *entry = lookup(dest);

    if (entry == NULL) {
        EV_WARN << "Impossible to generate Binding Authorization Data as CN is not existing in BUL!\n";
        return 0;
    }

    // generate the key
    return BindingUpdateList::generateKey(entry->tokenH, entry->tokenC, CoA);
}

int BindingUpdateList::generateKey(int homeToken, int careOfToken, const IPv6Address& CoA)
{
    // use a dummy value
    return homeToken + careOfToken;
}

int BindingUpdateList::generateHomeToken(const IPv6Address& HoA, int nonce)
{
    return HO_TOKEN;
}

int BindingUpdateList::generateCareOfToken(const IPv6Address& CoA, int nonce)
{
    return CO_TOKEN;
}

void BindingUpdateList::resetHomeToken(const IPv6Address& dest, const IPv6Address& hoa)
{
    BindingUpdateList::BindingUpdateListEntry *entry = lookup(dest);
    ASSERT(entry != NULL);

    entry->tokenH = UNDEFINED_TOKEN;
    //entry->sentHoTI = 0;
}

void BindingUpdateList::resetCareOfToken(const IPv6Address& dest, const IPv6Address& hoa)
{
    BindingUpdateList::BindingUpdateListEntry *entry = lookup(dest);
    ASSERT(entry != NULL);

    entry->tokenC = UNDEFINED_TOKEN;
    //entry->sentCoTI = 0;
}

bool BindingUpdateList::isHomeTokenAvailable(const IPv6Address& dest, InterfaceEntry *ie)
{
    BindingUpdateList::BindingUpdateListEntry *entry = lookup(dest);
    ASSERT(entry != NULL);

    return entry->tokenH != UNDEFINED_TOKEN &&
           (entry->sentHoTI + ie->ipv6Data()->_getMaxTokenLifeTime()) > simTime();
}

bool BindingUpdateList::isCareOfTokenAvailable(const IPv6Address& dest, InterfaceEntry *ie)
{
    BindingUpdateList::BindingUpdateListEntry *entry = lookup(dest);
    ASSERT(entry != NULL);

    return entry->tokenC != UNDEFINED_TOKEN &&
           (entry->sentCoTI + ie->ipv6Data()->_getMaxTokenLifeTime()) > simTime();
}

bool BindingUpdateList::isInBindingUpdateList(const IPv6Address& dest) const
{
    return bindingUpdateList.find(dest) != bindingUpdateList.end();
}

uint BindingUpdateList::getSequenceNumber(const IPv6Address& dest)
{
    // search for entry
    BindingUpdateList::BindingUpdateListEntry *entry = lookup(dest);

    if (entry == NULL)
        return 0;

    return entry->sequenceNumber;
}

const IPv6Address& BindingUpdateList::getCoA(const IPv6Address& dest)
{
    // search for entry
    BindingUpdateList::BindingUpdateListEntry *entry = lookup(dest);

    ASSERT(entry != NULL);

    return entry->careOfAddress;
}

bool BindingUpdateList::isInBindingUpdateList(const IPv6Address& dest, const IPv6Address& HoA)
{
    BindingUpdateList6::iterator pos = bindingUpdateList.find(dest);

    if (pos == bindingUpdateList.end())
        return false;

    return pos->second.homeAddress == HoA;
}

bool BindingUpdateList::isValidBinding(const IPv6Address& dest)
{
    BindingUpdateList::BindingUpdateListEntry *entry = lookup(dest);

    if (entry == NULL)
        return false;

    return entry->BAck && (entry->bindingLifetime < SIMTIME_DBL(simTime()));
}

bool BindingUpdateList::isBindingAboutToExpire(const IPv6Address& dest)
{
    BindingUpdateList::BindingUpdateListEntry *entry = lookup(dest);

    if (entry == NULL)
        return true;

    return entry->bindingLifetime < SIMTIME_DBL(simTime()) - PRE_BINDING_EXPIRY;
}

bool BindingUpdateList::sentBindingUpdate(const IPv6Address& dest)
{
    BindingUpdateList::BindingUpdateListEntry *entry = lookup(dest);

    if (entry == NULL)
        return false;

    return (entry->BAck || (entry->tokenH != UNDEFINED_TOKEN && entry->tokenC != UNDEFINED_TOKEN))
           && entry->sentTime != 0;
}

void BindingUpdateList::removeBinding(const IPv6Address& dest)
{
    BindingUpdateList::BindingUpdateListEntry *entry = lookup(dest);

    ASSERT(entry != NULL);

    if ((entry->tokenH != UNDEFINED_TOKEN) || (entry->tokenC != UNDEFINED_TOKEN))
        // for CNs, we just delete all entries
        resetBindingCacheEntry(*entry);
    else
        // the BUL entry to the HA is completely deleted
        bindingUpdateList.erase(dest);
}

void BindingUpdateList::suspendBinding(const IPv6Address& dest)
{
    BindingUpdateList::BindingUpdateListEntry *entry = lookup(dest);

    ASSERT(entry != NULL);

    entry->BAck = false;
}

bool BindingUpdateList::recentlySentCOTI(const IPv6Address& dest, InterfaceEntry *ie)
{
    BindingUpdateList::BindingUpdateListEntry *entry = lookup(dest);

    ASSERT(entry != NULL);

    return entry->sentCoTI + ie->ipv6Data()->_getMaxTokenLifeTime() / 3 > simTime();
}

bool BindingUpdateList::recentlySentHOTI(const IPv6Address& dest, InterfaceEntry *ie)
{
    BindingUpdateList::BindingUpdateListEntry *entry = lookup(dest);

    ASSERT(entry != NULL);

    return entry->sentHoTI + ie->ipv6Data()->_getMaxTokenLifeTime() / 3 > simTime();
}

void BindingUpdateList::resetBindingCacheEntry(BindingUpdateListEntry& entry)
{
    entry.bindingLifetime = 0;
    entry.bindingExpiry = 0;
    //entry.remainingLifetime = 0;
    //entry.sequenceNumber = 0;
    entry.sentTime = 0;
    //entry.nextBUTx = 0;
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

