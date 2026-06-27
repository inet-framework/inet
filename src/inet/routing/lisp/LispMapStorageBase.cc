//
// Copyright (C) 2013 - today Brno University of Technology, Czech Republic
// Copyright (C) OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// LISP (Locator/ID Separation Protocol, RFC 6830) ported from the ANSAINET project.
// Original author: Vladimir Vesely (Brno University of Technology).
//

#include "inet/routing/lisp/LispMapStorageBase.h"

#include <sstream>

namespace inet {
namespace lisp {

void LispMapStorageBase::addMapEntry(LispMapEntry& entry)
{
    MappingStorage.push_front(entry);
    MappingStorage.sort();
}

void LispMapStorageBase::removeMapEntry(const LispMapEntry& entry)
{
    MappingStorage.remove(entry);
}

LispMapEntry *LispMapStorageBase::findMapEntryByEidPrefix(const LispEidPrefix& eidpref)
{
    for (auto& entry : MappingStorage)
        if (entry.getEidPrefix() == eidpref)
            return &entry;
    return nullptr;
}

LispMapEntry *LispMapStorageBase::lookupMapEntry(L3Address address)
{
    LispMapEntry *bestMatch = nullptr;
    int bestLen = 0;
    for (auto& entry : MappingStorage) {
        int len = LispCommon::doPrefixMatch(entry.getEidPrefix().getEidAddr(), address);
        if (len > bestLen && len >= entry.getEidPrefix().getEidLength()) {
            bestMatch = &entry;
            bestLen = len;
        }
    }
    return bestLen > 0 ? bestMatch : nullptr;
}

LispMapEntry *LispMapStorageBase::findMapEntryFromByLocator(const L3Address& rloc, const LispEidPrefix& eidPref)
{
    MapStorageItem startme;
    if (eidPref == LispEidPrefix() || eidPref == MappingStorage.back().getEidPrefix())
        startme = MappingStorage.begin();
    else {
        startme = MappingStorage.begin();
        for (MapStorageItem it = MappingStorage.begin(); it != MappingStorage.end(); ++it)
            if (it->getEidPrefix() == eidPref) {
                startme = it;
                break;
            }
    }
    for (MapStorageItem it = startme; it != MappingStorage.end(); ++it)
        if (it->isLocatorExisting(rloc))
            return &(*it);
    return nullptr;
}

MapStorage LispMapStorageBase::findMapEntriesByLocator(const L3Address& rloc)
{
    MapStorage entrylist;
    for (auto& entry : MappingStorage)
        if (entry.isLocatorExisting(rloc))
            entrylist.push_back(entry);
    return entrylist;
}

bool LispMapStorageBase::updateMapEntry(const LispMapRecord& rec)
{
    LispEidPrefix eidPref(rec.getEidPrefix(), rec.getEidMaskLength());

    bool isnew = false;
    if (!findMapEntryByEidPrefix(eidPref)) {
        LispMapEntry newEntry(eidPref);
        isnew = true;
        addMapEntry(newEntry);
    }
    LispMapEntry *entry = findMapEntryByEidPrefix(eidPref);

    entry->setExpiry(simTime() + rec.getRecordTTL() * 60);
    entry->setTtl(rec.getRecordTTL());
    entry->setAction((LispCommon::EAct)rec.getAct());
    // NOTE: the Authoritative (A) bit is not processed yet

    if (rec.getLocatorsArraySize()) {
        for (size_t i = 0; i < rec.getLocatorsArraySize(); i++) {
            const LispLocatorRecord& lr = rec.getLocators(i);
            LispRlocator rloc;
            rloc.setRlocAddr(lr.rloc);
            rloc.setPriority(lr.priority);
            rloc.setWeight(lr.weight);
            rloc.setMpriority(lr.mpriority);
            rloc.setMweight(lr.mweight);
            rloc.setLocal(lr.localLocBit);
            if (!entry->isLocatorExisting(rloc.getRlocAddr()))
                entry->addLocator(rloc);
            LispRlocator *rl = entry->getLocator(rloc.getRlocAddr());
            rl->updateRlocator(rloc);
            if (lr.reachableBit)
                rl->setState(LispRlocator::UP);
        }
        entry->getRlocs().sort();
    }
    else
        EV_WARN << "Record for EID " << rec.getEidPrefix() << " has zero locators!\n";

    return isnew;
}

bool LispMapStorageBase::syncMapEntry(LispMapEntry& mapentry)
{
    bool isnew = false;
    LispMapEntry *entry = findMapEntryByEidPrefix(mapentry.getEidPrefix());
    if (!entry) {
        isnew = true;
        addMapEntry(mapentry);
    }
    else {
        entry->setExpiry(mapentry.getExpiry());
        entry->setTtl(mapentry.getTtl());
        entry->setAction(mapentry.getAction());
        if (mapentry.getRlocs().size()) {
            for (auto& rloc : mapentry.getRlocs()) {
                if (!entry->isLocatorExisting(rloc.getRlocAddr()))
                    entry->addLocator(rloc);
                else
                    entry->getLocator(rloc.getRlocAddr())->updateRlocator(rloc);
            }
            mapentry.getRlocs().sort();
        }
        else
            EV_WARN << "Record for EID " << mapentry.getEidPrefix() << " has zero locators!\n";
    }
    return isnew;
}

void LispMapStorageBase::parseMapEntry(cXMLElement *config)
{
    if (!config)
        return;
    for (cXMLElement *m : config->getChildrenByTagName(EID_TAG)) {
        if (!m->getAttribute(ADDRESS_ATTR)) {
            EV_WARN << "LISP config: <EID> missing 'address' attribute\n";
            continue;
        }
        std::string addr, leng;
        LispCommon::parseIpAddress(m->getAttribute(ADDRESS_ATTR), addr, leng);
        LispEidPrefix pref(addr.c_str(), leng.c_str());
        LispMapEntry me(pref);

        for (cXMLElement *n : m->getChildrenByTagName(RLOC_TAG)) {
            if (!n->getAttribute(ADDRESS_ATTR)) {
                EV_WARN << "LISP config: <RLOC> missing 'address' attribute\n";
                continue;
            }
            std::string rlocaddr = n->getAttribute(ADDRESS_ATTR);
            std::string pri = n->getAttribute(PRIORITY_ATTR) ? n->getAttribute(PRIORITY_ATTR) : "";
            std::string wei = n->getAttribute(WEIGHT_ATTR) ? n->getAttribute(WEIGHT_ATTR) : "";
            bool local = n->getAttribute(LOCAL_ATTR) && !strcmp(n->getAttribute(LOCAL_ATTR), ENABLED_VAL);

            if (!rlocaddr.empty()) {
                LispRlocator locator = (!pri.empty() && !wei.empty())
                    ? LispRlocator(rlocaddr.c_str(), pri.c_str(), wei.c_str(), local)
                    : LispRlocator(rlocaddr.c_str());
                me.addLocator(locator);
            }
        }
        addMapEntry(me);
    }
}

std::string LispMapStorageBase::str() const
{
    std::stringstream os;
    for (const auto& entry : MappingStorage)
        os << entry.str() << "\n";
    return os.str();
}

std::ostream& operator<<(std::ostream& os, const LispMapStorageBase& msb)
{
    return os << msb.str();
}

std::ostream& operator<<(std::ostream& os, const MapStorage& mapstor)
{
    for (const auto& entry : mapstor)
        os << entry.str() << "\n";
    return os;
}

} // namespace lisp
} // namespace inet
