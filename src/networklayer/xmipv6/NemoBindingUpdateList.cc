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


// #include "BindingUpdateList.h"
#include "NemoBindingUpdateList.h"

#include "IPv6InterfaceData.h"

// fayruz 27.01.2015
Define_Module(NemoBindingUpdateList);


std::ostream& operator<<(std::ostream& os, const NemoBindingUpdateList::NemoBindingUpdateListEntry& nbul)
{
    os << "Destination: " << nbul.destAddress << " HoA of MN: " << nbul.homeAddress
       << " CoA of MN: "<< nbul.careOfAddress << "\n"
       << "Binding Lifetime: " << nbul.bindingLifetime << " binding expiry: "
       << SIMTIME_STR(nbul.bindingExpiry) << " BU Sequence#: " << nbul.sequenceNumber
       << " Sent Time: "<< SIMTIME_STR(nbul.sentTime) /* << " Next_Tx_Time: " << bul.nextBUTx << */
       << " BU_Ack: " << nbul.BAck << "\n";

    os << "State: ";
    switch (nbul.state)
    {
        case NemoBindingUpdateList::NONE:
            os << "none";
            break;

        case NemoBindingUpdateList::REGISTER:
            os << "Registering";
            break;

        case NemoBindingUpdateList::REGISTERED:
            os << "Registered";
            break;

        case NemoBindingUpdateList::DEREGISTER:
            os << "Deregistering";
            break;

        default:
            os << "Unknown";
            break;
    }
    os << endl;

    return os;
}

NemoBindingUpdateList::NemoBindingUpdateList()
{
}

NemoBindingUpdateList::~NemoBindingUpdateList()
{
//    for (unsigned int i = 0; i < bindingUpdateList.size(); i++)
//        delete bindingUpdateList[i];
}

void NemoBindingUpdateList::initialize()
{
    WATCH_MAP(nemoBindingUpdateList); //added by Zarrar Yousaf
}

void NemoBindingUpdateList::handleMessage(cMessage *msg)
{
    opp_error("This module doesn't process messages");
}

void NemoBindingUpdateList::addOrUpdateBUL(const IPv6Address& dest, const IPv6Address& hoa, const IPv6Address& coa, const uint lifetime, const uint seq, const simtime_t buSentTime, const bool mR, const IPv6Address& prefix, int intID)//,const simtime_t& nextBUSentTime)
{
    // modified structure - CB

    // search for entry
    NemoBindingUpdateList::NemoBindingUpdateListEntry* entry = lookup(dest);

    // if it is not yet existing, create it
    if (entry == NULL)
    {
        entry = createBULEntry(dest);
    }

    EV << "\n++++++++++++++++++++Binding Update List Being Updated in Routing Table6 ++++++++++++++\n";

    entry->homeAddress = hoa;
    entry->careOfAddress = coa;
    // update lifetime 11.06.08 - CB
    entry->bindingLifetime = lifetime; // for the moment a constant but then it is supposed to decrement with time.
    entry->bindingExpiry = simTime() + lifetime; // binding expires at this point in time
    //TODO bindingUpdateList[dest].remainingLifetime = ;
    entry->sentTime = buSentTime; //the time at which the BU, whose ack is awaited is sent
    //entry->nextBUTx = nextBUSentTime; //the nextScgheduledTime at which the BU will be sent in case of timeout.
    entry->sequenceNumber = seq; //seq number of the last BU sent.
    entry->BAck = false;

    //fayruz 27.02.2015
    entry->mobileRouter = mR; // Mobile Router Flag
    entry->prefixInfo = prefix; // Prefix Information

    entry->interfaceID = intID; // for forwarding purpose - fayruz
}

NemoBindingUpdateList::NemoBindingUpdateListEntry* NemoBindingUpdateList::createBULEntry(const IPv6Address& dest)
{
    nemoBindingUpdateList[dest].destAddress = dest;

    NemoBindingUpdateListEntry& entry = nemoBindingUpdateList[dest];
    initializeBUValues(entry);

    return &entry;
}

void NemoBindingUpdateList::initializeBUValues(NemoBindingUpdateListEntry& entry)
{
    // normal BU values
    entry.bindingLifetime = 0;
    entry.bindingExpiry = 0;
    //TODO bindingUpdateList[dest].remainingLifetime = ;
    entry.sentTime = 0;
    //entry.nextBUTx = 0;
    entry.sequenceNumber = 0;
    entry.BAck = false;

    // 21.07.08 - CB
    entry.state = NONE;

    // fayruz 04.02.2015 new data structure RFC 3963 point 5.1
    //entry.prefixInfo = NULL;
    entry.mobileRouter = 0;

    entry.interfaceID = 0;
}

NemoBindingUpdateList::NemoBindingUpdateListEntry* NemoBindingUpdateList::lookup(const IPv6Address& dest)
{
    NemoBindingUpdateList6::iterator i = nemoBindingUpdateList.find(dest);

    return ( i == nemoBindingUpdateList.end() ) ? NULL : &(i->second);
}

NemoBindingUpdateList::NemoBindingUpdateListEntry* NemoBindingUpdateList::fetch(const IPv6Address& dest)
{
    NemoBindingUpdateList::NemoBindingUpdateListEntry* entry = lookup(dest);

    if (entry == NULL)
        return createBULEntry(dest);
    else
        return entry;
}

NemoBindingUpdateList::MobilityState NemoBindingUpdateList::getMobilityState(const IPv6Address& dest) const
{
    NemoBindingUpdateList6::const_iterator i = nemoBindingUpdateList.find(dest);

    if (i == nemoBindingUpdateList.end())
        return NONE;
    else
        return i->second.state;
}

void NemoBindingUpdateList::setMobilityState(const IPv6Address& dest, NemoBindingUpdateList::MobilityState state)
{
    NemoBindingUpdateList6::iterator i = nemoBindingUpdateList.find(dest);

    if (i != nemoBindingUpdateList.end())
        i->second.state = state;
}

bool NemoBindingUpdateList::isInBindingUpdateList(const IPv6Address& dest) const
{
    return nemoBindingUpdateList.find(dest) != nemoBindingUpdateList.end();
}

uint NemoBindingUpdateList::getSequenceNumber(const IPv6Address& dest)
{
    // search for entry
    NemoBindingUpdateList::NemoBindingUpdateListEntry* entry = lookup(dest);

    if (entry == NULL)
        return 0;

    return entry->sequenceNumber;
}

const IPv6Address& NemoBindingUpdateList::getCoA(const IPv6Address& dest)
{
    // search for entry
    NemoBindingUpdateList::NemoBindingUpdateListEntry* entry = lookup(dest);

    ASSERT(entry != NULL);

    return entry->careOfAddress;
}

bool NemoBindingUpdateList::isInBindingUpdateList(const IPv6Address& dest, const IPv6Address& HoA)
{
    NemoBindingUpdateList6::iterator pos = nemoBindingUpdateList.find(dest);

    if (pos == nemoBindingUpdateList.end())
        return false;

    return pos->second.homeAddress == HoA;
}

bool NemoBindingUpdateList::isValidBinding(const IPv6Address& dest)
{
    NemoBindingUpdateList::NemoBindingUpdateListEntry* entry = lookup(dest);

    if (entry == NULL)
        return false;

    return entry->BAck && ( entry->bindingLifetime < SIMTIME_DBL(simTime()) );
}

bool NemoBindingUpdateList::isBindingAboutToExpire(const IPv6Address& dest)
{
    NemoBindingUpdateList::NemoBindingUpdateListEntry* entry = lookup(dest);

    if (entry == NULL)
        return true;

    return entry->bindingLifetime < SIMTIME_DBL(simTime()) - PRE_BINDING_EXPIRY;
}

bool NemoBindingUpdateList::sentBindingUpdate(const IPv6Address& dest)
{
    NemoBindingUpdateList::NemoBindingUpdateListEntry* entry = lookup(dest);

    if (entry == NULL)
        return false;

    return (entry->BAck)
            && entry->sentTime != 0;
}

void NemoBindingUpdateList::removeBinding(const IPv6Address& dest)
{
    NemoBindingUpdateList::NemoBindingUpdateListEntry* entry = lookup(dest);

    ASSERT(entry != NULL);

        // the BUL entry to the HA is completely deleted
        nemoBindingUpdateList.erase(dest);
}

void NemoBindingUpdateList::suspendBinding(const IPv6Address& dest)
{
    NemoBindingUpdateList::NemoBindingUpdateListEntry* entry = lookup(dest);

    ASSERT(entry != NULL);

    entry->BAck = false;
}

//fayruz 27.02.2015
bool NemoBindingUpdateList::getMobileRouterFlag(const IPv6Address& dest)
{
    // search for entry
        NemoBindingUpdateList::NemoBindingUpdateListEntry* entry = lookup(dest);

        ASSERT(entry != NULL);

        return entry->mobileRouter;
}

void NemoBindingUpdateList::resetBindingCacheEntry(NemoBindingUpdateListEntry& entry)
{
    entry.bindingLifetime = 0;
    entry.bindingExpiry = 0;
    //entry.remainingLifetime = 0;
    //entry.sequenceNumber = 0;
    entry.sentTime = 0;
    //entry.nextBUTx = 0;
    entry.BAck = false;
    entry.state = NONE;
    entry.interfaceID = 0;
}


